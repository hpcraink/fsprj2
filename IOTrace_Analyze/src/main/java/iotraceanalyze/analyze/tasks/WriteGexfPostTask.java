package iotraceanalyze.analyze.tasks;

import iotraceanalyze.analyze.Evaluation;
import iotraceanalyze.model.analysis.FunctionEvent;
import iotraceanalyze.model.analysis.file.FileRange;
import iotraceanalyze.model.analysis.trace.traceables.BasicTrace;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;
import iotraceanalyze.model.analysis.trace.traceables.ThreadTrace;
import iotraceanalyze.analyze.misc.visualization.graphFormat.*;

import java.io.File;
import java.util.*;

public class WriteGexfPostTask extends PostTask {

    public WriteGexfPostTask(Properties props) {
        super(props);
    }


    public void performTask(Evaluation evaluation) {
        File file = new File(super.getOutputFolder() + super.getInputFile() + ".gexf");

        ResourceBundle legends = evaluation.getLegends();
        BasicTrace<FileTrace> fileTraces = evaluation.getFileTraces();
        String directoryDelimiter = evaluation.getDirectoryDelimiter();
        BasicTrace<BasicTrace<BasicTrace<ThreadTrace>>> cluster = evaluation.getCluster();


        // - ...
        ArrayList<FileTrace.FileType> kinds = new ArrayList<>();
        kinds.add(FileTrace.FileType.FILE);
        kinds.add(FileTrace.FileType.TMPFILE);

        Calendar lastModified = Calendar.getInstance();
        GexfGraph gexfGraph = new GexfGraph(lastModified.getTime(), "IOTrace_Analyze",
                legends.getString("gexfDescription"), Gexf.ModeType.DYNAMIC, Gexf.DefaultEdgeType.DIRECTED, Gexf.TimeFormat.DOUBLE);

        // set edge weight as dynamic attribute for easier layout in gephi
        GexfAttributes weightAttributes = gexfGraph.createAttributes(Gexf.ClassType.EDGE, Gexf.ModeType.DYNAMIC);
        GexfAttribute weightAttribute = weightAttributes.createAttribute("weight",
                legends.getString("gexfWeightAttributeTitle"), Gexf.AttrType.FLOAT);

        GexfAttributes allNodeAttributes = gexfGraph.createAttributes(Gexf.ClassType.NODE, Gexf.ModeType.STATIC);
        GexfAttribute typeAttribute = allNodeAttributes.createAttribute("type",
                legends.getString("gexfTypeAttributeTitle"), Gexf.AttrType.STRING);

        GexfAttributes fileNodeAttributes = gexfGraph.createAttributes(Gexf.ClassType.NODE, Gexf.ModeType.STATIC);
        GexfAttribute fullPathAttribute = fileNodeAttributes.createAttribute("fullPath",
                legends.getString("gexfFullPathAttributeTitle"), Gexf.AttrType.STRING);

        GexfAttributes threadNodeAttributes = gexfGraph.createAttributes(Gexf.ClassType.NODE, Gexf.ModeType.DYNAMIC);
//		GexfAttribute startTimeNodeAttribute = threadNodeAttributes.createAttribute("startTime", "startTime",
//				AttrType.LONG);
//		GexfAttribute endTimeNodeAttribute = threadNodeAttributes.createAttribute("endTime", "endTime", AttrType.LONG);
        GexfAttribute sumTimeAttribute = threadNodeAttributes.createAttribute("sumTime",
                legends.getString("gexfSumTimeAttributeTitle"), Gexf.AttrType.LONG);
        GexfAttribute sumBytesAttribute = threadNodeAttributes.createAttribute("sumBytes",
                legends.getString("gexfSumBytesAttributeTitle"), Gexf.AttrType.LONG);

        GexfAttributes edgeAttributes = gexfGraph.createAttributes(Gexf.ClassType.EDGE, Gexf.ModeType.DYNAMIC);
//		GexfAttribute startTimeEdgeAttribute = edgeAttributes.createAttribute("startTime", "startTime", AttrType.LONG);
//		GexfAttribute endTimeEdgeAttribute = edgeAttributes.createAttribute("endTime", "endTime", AttrType.LONG);
        GexfAttribute functionAttribute = edgeAttributes.createAttribute("function",
                legends.getString("gexfFunctionAttributeTitle"), Gexf.AttrType.STRING);
        GexfAttribute bytesAttribute = edgeAttributes.createAttribute("bytes",
                legends.getString("gexfBytesAttributeTitle"), Gexf.AttrType.LONG);
        GexfAttribute timeAttribute = edgeAttributes.createAttribute("time",
                legends.getString("gexfTimeAttributeTitle"), Gexf.AttrType.LONG);
        GexfAttribute errorAttribute = edgeAttributes.createAttribute("error",
                legends.getString("gexfErrorAttributeTitle"), Gexf.AttrType.BOOLEAN);
        GexfAttribute ioTypeAttribute = edgeAttributes.createAttribute("ioType",
                legends.getString("gexfIoTypeAttributeTitle"), Gexf.AttrType.STRING);

        Map<String, GexfNode> fileNodes = new HashMap<>();

        for (Map.Entry<String, FileTrace> e : fileTraces.getTraces().entrySet()) {
            if (!kinds.contains(e.getValue().getFileType())) {
                continue;
            }

            String oldStart = null;
            long oldStartTime = 0;
            long sumUsedTime = 0;
            long sumBytes = 0;
            boolean isRead = false;
            boolean isWritten = false;

            String label;
            if (e.getValue().getFileType() != FileTrace.FileType.FILE && e.getValue().getFileType() != FileTrace.FileType.TMPFILE) {
                label = legends.getString("gexfNoFileNoTmpFileLabel");
            } else {
                label = (String) e.getValue().getFileNames().toArray()[0];
                int slash = label.lastIndexOf(directoryDelimiter);
                if (slash > 0) {
                    label = label.substring(slash);
                }
            }

            // add files as nodes
            GexfNode fileNode = gexfGraph.createNode(e.getKey(), label);
            fileNode.createValue(fullPathAttribute, e.getValue().getFileNames().toString());

            fileNodes.put(e.getKey(), fileNode);

            FunctionEvent fe = e.getValue().getFirstFunctionEvent();
            while (fe != null) {

                long start = fe.getStartTime();
                long end = fe.getEndTime();
                String startString = null;
                long bytes = fe.getFunctionByteCount();
                long usedTime = end - start;

                // add summaries as attributes to file
                // (set end-time to start of next interval => omitting gaps)
                if (oldStart != null) {
                    start = getNextTime(start, oldStartTime);
                    startString = getNormTime(start);

//					fileNode.createValue(startTimeNodeAttribute, String.valueOf(oldStartTime), oldStart, startString);
//					fileNode.createValue(endTimeNodeAttribute, String.valueOf(start), oldStart, startString);
                    fileNode.createValue(sumTimeAttribute, Long.toString(sumUsedTime), oldStart, startString);
                    fileNode.createValue(sumBytesAttribute, Long.toString(sumBytes), oldStart, startString);
                }
                if (startString == null) {
                    oldStart = getNormTime(start);
                } else {
                    oldStart = startString;
                }
                oldStartTime = start;
                sumUsedTime += usedTime;
                sumBytes += bytes;

                FileRange fileRange = fe.getFileRange();
                if (fileRange != null) {
                    switch (fileRange.getRangeType()) {
                        case READ:
                            isRead = true;
                            break;
                        case WRITE:
                            isWritten = true;
                            break;
                        default:
                            break;
                    }
                }

                fe = fe.getNextCallInFileTrace();
            }

            // add summaries as attributes to thread
            // (set end-time to start of next interval => omitting gaps)
            if (oldStart != null) {
//				fileNode.createValue(startTimeNodeAttribute, String.valueOf(oldStartTime), oldStart);
                fileNode.createValue(sumTimeAttribute, Long.toString(sumUsedTime), oldStart);
                fileNode.createValue(sumBytesAttribute, Long.toString(sumBytes), oldStart);
            }

            // add fileType read_only, write_only or read_and_write to file
            String fileTypeValue;
            if (e.getValue().getFileType() != FileTrace.FileType.FILE && e.getValue().getFileType() != FileTrace.FileType.TMPFILE) {
                fileTypeValue = legends.getString("gexfFileTypeNoFileNoTmpFile");
            } else if (isRead && isWritten) {
                fileTypeValue = legends.getString("gexfFileTypeReadWrite");
            } else if (isRead) {
                fileTypeValue = legends.getString("gexfFileTypeRead");
            } else if (isWritten) {
                fileTypeValue = legends.getString("gexfFileTypeWrite");
            } else {
                fileTypeValue = legends.getString("gexfFileTypeNoReadNoWrite");
            }
            fileNode.createValue(typeAttribute, fileTypeValue);
        }

        for (Map.Entry<String, BasicTrace<BasicTrace<ThreadTrace>>> e : cluster.getTraces().entrySet()) {
            String hostName = legends.getString("gexfHostName") + ":" + e.getKey();

            // add hosts as nodes
            GexfNode hostNode = gexfGraph.createNode(hostName, e.getKey());
            hostNode.createValue(typeAttribute, legends.getString("gexfHostType"));

            for (Map.Entry<String, BasicTrace<ThreadTrace>> e2 : e.getValue().getTraces().entrySet()) {
                String processId = hostName + ":" + legends.getString("gexfProcessName") + ":" + e2.getKey();

                // add processes as nodes
                GexfNode processNode = gexfGraph.createNode(processId, e2.getKey());
                processNode.createValue(typeAttribute, legends.getString("gexfProcessType"));

                // add edge from host to process
                GexfEdge edge = gexfGraph.createEdge(
                        hostName + "_" + legends.getString("gexfHostToProcessConnector") + "_" + processId, hostNode,
                        processNode);
                edge.createValue(weightAttribute, "100.0");
                edge.createValue(ioTypeAttribute, legends.getString("gexfIoTypeProcessToHost"));

                for (Map.Entry<String, ThreadTrace> e3 : e2.getValue().getTraces().entrySet()) {
                    String threadId = processId + ":" + legends.getString("gexfThreadName") + ":" + e3.getKey();
                    String oldStart = null;
                    long oldStartTime = 0;
                    long sumUsedTime = 0;
                    long sumBytes = 0;

                    // add threads as nodes
                    GexfNode threadNode = gexfGraph.createNode(threadId, e3.getKey());
                    threadNode.createValue(typeAttribute, legends.getString("gexfThreadType"));

                    // add edge from process to thread
                    edge = gexfGraph.createEdge(
                            processId + "_" + legends.getString("gexfProcessToThreadConnector") + "_" + threadId,
                            processNode, threadNode);
                    edge.createValue(weightAttribute, "100.0");
                    edge.createValue(ioTypeAttribute, legends.getString("gexfIoTypeThreadToProcess"));

                    Map<String, GexfEdge> edges = new HashMap<>();
                    FunctionEvent fe = e3.getValue().getFirstFunctionEvent();
                    while (fe != null) {
                        boolean writeSummary = false;

                        List<FunctionEvent> fes = new LinkedList<>();
                        fes.add(fe);
                        if (fe.hasSame()) {
                            // for function calls which manipulate more than one file
                            fes.addAll(fe.getSame());
                        }

                        long start = fe.getStartTime();
                        long end = fe.getEndTime();
                        String startString = getNormTime(start);
                        end = getNextTime(end, start);
                        String endString = getNormTime(end);
                        long bytes = fe.getFunctionByteCount();
                        long usedTime = end - start;

                        for (FunctionEvent event : fes) {
                            if (event.getFileTrace() != null && kinds.contains(event.getFileTrace().getFileType())) {
                                writeSummary = true;
                                String fileId = event.getFileTrace().getFileId().toString();
                                String edgeId = threadId + "_" + legends.getString("gexfThreadToFileConnector") + "_"
                                        + fileId;

                                // add edge from thread to file
                                if (edges.containsKey(edgeId)) {
                                    edge = edges.get(edgeId);
                                } else {
                                    edge = gexfGraph.createEdge(edgeId, threadNode, fileNodes.get(fileId));
                                    edges.put(edgeId, edge);
                                }

                                edge.createSpell(startString, endString);

                                edge.createValue(weightAttribute, "1.0");
//								edge.createValue(startTimeEdgeAttribute, String.valueOf(start), startString, endString);
//								edge.createValue(endTimeEdgeAttribute, String.valueOf(end), startString, endString);
                                edge.createValue(functionAttribute, event.getFunctionName(), startString, endString);
                                edge.createValue(bytesAttribute, String.valueOf(bytes), startString, endString);
                                edge.createValue(timeAttribute, String.valueOf(usedTime), startString, endString);
                                edge.createValue(errorAttribute, String.valueOf(event.hasError()), startString,
                                        endString);

                                FileRange fileRange = event.getFileRange();
                                String ioType;
                                if (fileRange != null) {
                                    switch (fileRange.getRangeType()) {
                                        case READ:
                                            ioType = legends.getString("gexfIoTypeRead");
                                            break;
                                        case WRITE:
                                            ioType = legends.getString("gexfIoTypeWrite");
                                            break;
                                        default:
                                            ioType = legends.getString("gexfIoTypeOther");
                                            break;
                                    }
                                } else {
                                    ioType = legends.getString("gexfIoTypeOther");
                                }
                                edge.createValue(ioTypeAttribute, ioType, startString, endString);
                            }
                        }

                        if (writeSummary) {
                            // add summaries as attributes to thread
                            // (set end-time to start of next interval => omitting gaps)
                            if (oldStart != null) {
                                start = getNextTime(start, oldStartTime);
                                startString = getNormTime(start);

//								threadNode.createValue(startTimeNodeAttribute, Long.toString(oldStartTime), oldStart,
//										startString);
//								threadNode.createValue(endTimeNodeAttribute, Long.toString(start), oldStart,
//										startString);
                                threadNode.createValue(sumTimeAttribute, Long.toString(sumUsedTime), oldStart,
                                        startString);
                                threadNode.createValue(sumBytesAttribute, Long.toString(sumBytes), oldStart,
                                        startString);
                            }
                            oldStart = startString;
                            oldStartTime = start;
                            sumUsedTime += usedTime;
                            sumBytes += bytes;
                        }

                        fe = fe.getNextCallInThreadTrace();
                    }

                    // add summaries as attributes to thread
                    // (set end-time to start of next interval => omitting gaps)
                    if (oldStart != null) {
//						threadNode.createValue(startTimeNodeAttribute, Long.toString(oldStartTime), oldStart);
                        threadNode.createValue(sumTimeAttribute, Long.toString(sumUsedTime), oldStart);
                        threadNode.createValue(sumBytesAttribute, Long.toString(sumBytes), oldStart);
                    }
                }
            }
        }

        gexfGraph.write(file);
    }


    // - Helper-methods
    private long getNextTime(long time, long unequalTo) {
        double doubleTime = Double.parseDouble(getNormTime(time));
        double doubleUnequalTo = Double.parseDouble(getNormTime(unequalTo));

        while (doubleTime == doubleUnequalTo) {
            time++;
            doubleTime = Double.parseDouble(getNormTime(time));
        }

        return time;
    }

    private String getNormTime(long time) {
        return String.valueOf(time);
        // return String.valueOf(time - minTime);
        // return String.format("0.%0" + timeLength + "d", time - minTime);
    }

}