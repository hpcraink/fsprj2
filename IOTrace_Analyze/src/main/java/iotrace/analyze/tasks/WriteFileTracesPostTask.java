package iotrace.analyze.tasks;

import iotrace.analyze.Evaluation;
import iotrace.model.analysis.FunctionEvent;
import iotrace.model.analysis.file.FileId;
import iotrace.model.analysis.trace.ProcessTrace;
import iotrace.model.analysis.trace.traceables.BasicTrace;
import iotrace.model.analysis.trace.traceables.FileTrace;
import iotrace.model.analysis.trace.traceables.ThreadTrace;
import iotrace.analyze.visualization.graphFormat.*;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;

public class WriteFileTracesPostTask extends PostTask {

    private static final Logger logger = LogManager.getLogger(WriteFileTracesPostTask.class);


    // $$$ TODO: REFACTOR (pass differently to methods) $$$
    private Evaluation eval;


    public WriteFileTracesPostTask(Properties props) {
        super(props);
    }


    public void performTask(Evaluation evaluation) {
        ResourceBundle legends = evaluation.getLegends();
        BasicTrace<FileTrace> fileTraces = evaluation.getFileTraces();
        String directoryDelimiter = evaluation.getDirectoryDelimiter();
        BasicTrace<BasicTrace<BasicTrace<ThreadTrace>>> cluster = evaluation.getCluster();

        String filePrefix = super.getOutputFolder() + super.getInputFile();

        this.eval = evaluation;


        // - ...
        ArrayList<FileTrace.FileType> kinds = new ArrayList<>();
        kinds.add(FileTrace.FileType.FILE);
        kinds.add(FileTrace.FileType.TMPFILE);

        for (Map.Entry<String, FileTrace> e : fileTraces.getTraces().entrySet()) {
            if (kinds.contains(e.getValue().getFileType())) {
                FileId id = e.getValue().getFileId();
                String fileName = id.toFileName();
                printFileTrace(new File(filePrefix + "_fileTrace_" + fileName + ".txt"), e.getValue());
            }
        }
    }


    // - Helper-methods
    private void printFileTrace(File file, FileTrace fileTrace) {
        FunctionEvent event = fileTrace.getFirstFunctionEvent();
        HashMap<String, Integer> threadSpaces = new HashMap<>();
        int timeSpace = 20;
        int functionSpace = 70;
        int threadSpace = 5;
        int threadCount = -1;

        try (FileWriter fileWriter = new FileWriter(file)) {
            BufferedWriter bufferedWriter = new BufferedWriter(fileWriter);

            bufferedWriter.write(fileTrace + ":" + System.lineSeparator());
            while (event != null) {
                String tmp = String.valueOf(event.getTime());
                tmp = fillSpace(tmp, "-", timeSpace);
                tmp += event.getFunctionName() + "(" + event.printFileTraceId() + "):" + event.getFunctionType();
                tmp = fillSpace(tmp, "-", timeSpace + functionSpace);

                ThreadTrace threadTrace = event.getThreadTrace();
                String thread = threadTrace.getHostName() + ":" + threadTrace.getProcessId() + ":"
                        + threadTrace.getThreadId();
                int threadSpaceCount;
                if (threadSpaces.containsKey(thread)) {
                    threadSpaceCount = threadSpaces.get(thread);
                    thread = "|";
                } else {
                    threadCount++;
                    threadSpaceCount = threadCount;
                    threadSpaces.put(thread, threadCount);
                    thread = "+" + thread;
                }
                tmp = fillSpace(tmp, "-", timeSpace + functionSpace + threadSpace * (threadSpaceCount));
                tmp += thread;
                for (int i = threadSpaceCount; i < threadCount; i++) {
                    tmp = fillSpace(tmp, " ", timeSpace + functionSpace + threadSpace * (i + 1));
                    tmp += "|";
                }

                Set<FunctionEvent> overlapping = event.getOverlappingFileRange();
                if (!overlapping.isEmpty()) {
                    tmp += " " + getOverlapping(overlapping);
                }

                bufferedWriter.write(tmp + System.lineSeparator());

                event = event.getNextInFileTrace();
            }

            bufferedWriter.close();
        } catch (IOException e) {
            logger.error("Exception during write of file trace to file", e);
        }
    }


    private String getOverlapping(Set<FunctionEvent> overlapping) {
        String tmp = null;

        for (FunctionEvent e : overlapping) {
            if (tmp == null) {
                tmp = eval.getLegends().getString("fileTraceOverlapping") + ": ";
            } else {
                tmp += ", ";
            }
            tmp += e.getTime() + "[" + e.getFileRange().getStartPos() + "-" + e.getFileRange().getEndPos() + "]";
        }

        return tmp;
    }

    private String fillSpace(String tmp, String fill, int space) {
        for (int i = tmp.length(); i < space; i++) {
            tmp += fill;
        }
        return tmp;
    }

    private void writeDot(File file) {
        FileWriter fileWriter;
        try {
            fileWriter = new FileWriter(file);
        } catch (IOException e) {
            logger.error("Exception during write of dot file", e);
            return;
        }

        BufferedWriter bufferedWriter = new BufferedWriter(fileWriter);
        try {

            // bufferedWriter.write("digraph process_to_file {rankdir=LR;");
            bufferedWriter.write("digraph fileIO {");

            // Files as subgraph on same level
            // bufferedWriter.write(getDotSubgraphStart("file", "File"));
            for (Map.Entry<String, FileTrace> e : eval.getFileTraces().getTraces().entrySet()) {
                Map<String, String> attributes = new HashMap<>();
                attributes.put("fullPath", e.getValue().getFileNames().toString());

                String label = (String) e.getValue().getFileNames().toArray()[0];
                int slash = label.lastIndexOf(eval.getDirectoryDelimiter());
                if (slash > 0) {
                    label = label.substring(slash + 1);
                }

                bufferedWriter.write(Dot.getDotNode(e.getKey(), label, "file", attributes));
            }
            // bufferedWriter.write(getDotSubgraphEnd());

            // Hosts as subgraph on same level
            // bufferedWriter.write(getDotSubgraphStart("host", "Host"));
            for (Map.Entry<String, HashMap<String, ProcessTrace>> e : eval.getProcessTraces().entrySet()) {
                String hostName = "Host:" + e.getKey();

                bufferedWriter.write(Dot.getDotNode(hostName, e.getKey(), "host"));
            }
            // bufferedWriter.write(getDotSubgraphEnd());

            for (Map.Entry<String, BasicTrace<BasicTrace<ThreadTrace>>> e : eval.getCluster().getTraces().entrySet()) {
                String hostName = "Host:" + e.getKey();

                // Processes as subgraph on same level
                // bufferedWriter.write(getDotSubgraphStart("process", "Process"));
                for (Map.Entry<String, BasicTrace<ThreadTrace>> e2 : e.getValue().getTraces().entrySet()) {
                    String processId = hostName + ":Process:" + e2.getKey();

                    bufferedWriter.write(Dot.getDotNode(processId, e2.getKey(), "process"));
                }
                // bufferedWriter.write(getDotSubgraphEnd());

                for (Map.Entry<String, BasicTrace<ThreadTrace>> e2 : e.getValue().getTraces().entrySet()) {
                    String processId = hostName + ":Process:" + e2.getKey();

                    // Edge between Host and Process
                    bufferedWriter.write(Dot.getDotEdge(hostName, processId));

                    // Threads as subgraph on same level
                    // bufferedWriter.write(getDotSubgraphStart("thread", "Thread"));
                    for (Map.Entry<String, ThreadTrace> e3 : e2.getValue().getTraces().entrySet()) {
                        String threadId = processId + ":Thread:" + e3.getKey();

                        bufferedWriter.write(Dot.getDotNode(threadId, e3.getKey(), "thread"));
                    }
                    // bufferedWriter.write(getDotSubgraphEnd());

                    for (Map.Entry<String, ThreadTrace> e3 : e2.getValue().getTraces().entrySet()) {
                        String threadId = processId + ":Thread:" + e3.getKey();

                        // Edge between Process and Thread
                        bufferedWriter.write(Dot.getDotEdge(processId, threadId));

                        // all Function Events
                        FunctionEvent fe = e3.getValue().getFirstFunctionEvent();
                        if (fe != null) {
                            bufferedWriter.write(Dot.getDotEdge(fe));

                            // fe.getKind();
//							String functionEventId = fe.getId();
//							bufferedWriter.write(getDotNode(functionEventId, fe.getFunctionName(), threadId, fe.getTime()));
//							bufferedWriter.write(getDotEdge(threadId, functionEventId));
//							String functionEventPrevId = functionEventId;
                            for (fe = fe.getNextCallInThreadTrace(); fe != null; fe = fe.getNextCallInThreadTrace()) {
                                bufferedWriter.write(Dot.getDotEdge(fe));

//								functionEventId = fe.getId();
//
//								bufferedWriter.write(getDotNode(functionEventId, fe.getFunctionName(), threadId, fe.getTime()));
//								bufferedWriter.write(getDotEdge(functionEventPrevId, functionEventId));
//
//								functionEventPrevId = functionEventId;
                            }
                        }
                    }
                }
            }

            bufferedWriter.write("}");
            bufferedWriter.close();

        } catch (IOException e) {
            logger.error("Exception during write of dot file", e);
        }
    }

}