package iotraceanalyze.analyze.tasks;

import iotraceanalyze.analyze.Evaluation;
import iotraceanalyze.model.analysis.FunctionEvent;
import iotraceanalyze.model.analysis.trace.traceables.BasicTrace;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;
import iotraceanalyze.model.analysis.trace.traceables.ThreadTrace;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;

public class WriteCsvPostTask extends PostTask {

    private static final Logger logger = LogManager.getLogger(WriteCsvPostTask.class);

    public WriteCsvPostTask(Properties props) {
        super(props);
    }


    @Override
    public void performTask(Evaluation evaluation) {
        File file = new File(super.getOutputFolder() + super.getInputFile() + ".csv");

        ResourceBundle legends = evaluation.getLegends();
        BasicTrace<FileTrace> fileTraces = evaluation.getFileTraces();


        // - ...
        try (FileWriter fileWriter = new FileWriter(file)) {
            BufferedWriter bufferedWriter = new BufferedWriter(fileWriter);

            String result[] = legends.getString("csvHeader").trim().split("\\s*,\\s*");
            String header = "";
            for (String s : result) {
                header += "\"" + s + "\",";
            }
            header = header.substring(0, header.length() - 1);
            bufferedWriter.write(header + System.lineSeparator());

            for (Map.Entry<String, FileTrace> e : fileTraces.getTraces().entrySet()) {

                FunctionEvent fe = e.getValue().getFirstFunctionEvent();
                while (fe != null) {

                    List<FunctionEvent> events = new LinkedList<>();
                    events.add(fe);
                    if (fe.hasSame()) {
                        events.addAll(fe.getSame());
                    }

                    for (FunctionEvent event : events) {
                        ThreadTrace threadTrace = event.getThreadTrace();
                        String threadId = legends.getString("csvHostName") + ":" + threadTrace.getHostName() + ":"
                                + legends.getString("csvProcessName") + ":" + threadTrace.getProcessId() + ":"
                                + legends.getString("csvThreadName") + ":" + threadTrace.getThreadId();

                        String fileId;
                        String fileName;
                        if (event.getFileTrace() == null) {
                            fileId = null;
                            fileName = null;
                        } else {
                            fileId = event.getFileTrace().getFileId().toString();
                            fileName = getCsvArrayFromStringSet(event.getFileTrace().getFileNames());
                        }

                        String type;
                        if (event.getFileRange() == null) {
                            type = null;
                        } else {
                            type = event.getFileRange().getRangeType().toString();
                        }

                        String function = getCsvString(event.getId()) + "," + getCsvString(event.getFunctionName())
                                + "," + getCsvString(type) + "," + event.hasError() + ","
                                + event.getFunctionTimePeriod() + "," + event.getWrapperTimePeriod() + ","
                                + event.getFunctionByteCount() + "," + event.getStartTime() + "," + event.getEndTime()
                                + "," + getCsvString(threadId) + "," + getCsvString(fileId) + "," + fileName + ","
                                + getCsvArrayFromEvents(event.getSame()) + ","
                                + getCsvArrayFromEvents(event.getOverlappingFunctionEvents()) + ","
                                + getCsvArrayFromEvents(event.getOverlappingFileRange());

                        bufferedWriter.write(function + System.lineSeparator());
                    }

                    fe = fe.getNextCallInFileTrace();
                }
            }

            bufferedWriter.close();
        } catch (IOException e) {
            logger.error("Exception during write of csv file", e);
        }

    }


    // - Helper-methods
    private static String getCsvArrayFromStringSet(Set<String> strings) {
        StringBuilder builder = new StringBuilder();
        builder.append("\"[");

        if (strings != null && !strings.isEmpty()) {
            for (String s : strings) {
                builder.append(s);
                builder.append(",");
            }
            if (builder.charAt(builder.length() - 1) == ',') {
                builder.deleteCharAt(builder.length() - 1);
            }
        }

        builder.append("]\"");

        return builder.toString();
    }

    private static String getCsvArrayFromEvents(Collection<FunctionEvent> events) {
        StringBuilder builder = new StringBuilder();
        builder.append("\"[");

        if (events != null) {
            for (FunctionEvent event : events) {
                builder.append(event.getId());
                builder.append(",");
            }
            if (builder.charAt(builder.length() - 1) == ',') {
                builder.deleteCharAt(builder.length() - 1);
            }
        }

        builder.append("]\"");

        return builder.toString();
    }

    private static String getCsvString(String value) {
        if (value != null && !value.isEmpty()) {
            return "\"" + value + "\"";
        } else {
            return "";
        }
    }

}