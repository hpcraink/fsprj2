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
import java.text.SimpleDateFormat;
import java.util.*;

public class WriteSQLPostTask extends PostTask {
    private static final Logger logger = LogManager.getLogger(WriteSQLPostTask.class);

    public WriteSQLPostTask(Properties props) {
        super(props);
    }


    @Override
    public void performTask(Evaluation evaluation) {
        File file = new File(super.getOutputFolder() + super.getInputFile() + ".sql");

        BasicTrace<FileTrace> fileTraces = evaluation.getFileTraces();
        BasicTrace<BasicTrace<BasicTrace<ThreadTrace>>> cluster = evaluation.getCluster();


        // - ...
        SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd_HH:mm:ss_z_");
        Date date = new Date(System.currentTimeMillis());
        String collectionName = formatter.format(date) + super.getInputFile();

        UUID uuid = UUID.randomUUID();

        try (FileWriter fileWriter = new FileWriter(file)) {
            BufferedWriter bufferedWriter = new BufferedWriter(fileWriter);

            String insert = "insert into grafana_iotrace.collections (id, name) values(" + uuidToBin(uuid) + ", "
                    + getSqlString(collectionName) + ");";
            bufferedWriter.write(insert + System.lineSeparator());

            for (Map.Entry<String, FileTrace> e : fileTraces.getTraces().entrySet()) {
                String fileKind = getSqlString(e.getValue().getFileType().toString());
                String fileName = getSqlString(e.getValue().getFileId().toString());

                insert = "insert into grafana_iotrace.files (collection_id, file, kind) values(" + uuidToBin(uuid)
                        + ", " + fileName + ", " + fileKind + ");";
                bufferedWriter.write(insert + System.lineSeparator());

                for (String s : e.getValue().getFileNames()) {
                    insert = "insert into grafana_iotrace.file_names (collection_id, file, name) values("
                            + uuidToBin(uuid) + ", " + fileName + ", " + getSqlString(s) + ");";
                    bufferedWriter.write(insert + System.lineSeparator());
                }
            }

            for (Map.Entry<String, BasicTrace<BasicTrace<ThreadTrace>>> e : cluster.getTraces().entrySet()) {
                String hostName = getSqlString(e.getKey());

                insert = "insert into grafana_iotrace.hosts (collection_id, name) values(" + uuidToBin(uuid) + ", "
                        + hostName + ");";
                bufferedWriter.write(insert + System.lineSeparator());

                for (Map.Entry<String, BasicTrace<ThreadTrace>> e2 : e.getValue().getTraces().entrySet()) {
                    String processId = e2.getKey();

                    insert = "insert into grafana_iotrace.processes (collection_id, id, host_name) values("
                            + uuidToBin(uuid) + ", " + processId + ", " + hostName + ");";
                    bufferedWriter.write(insert + System.lineSeparator());

                    for (Map.Entry<String, ThreadTrace> e3 : e2.getValue().getTraces().entrySet()) {
                        String threadId = e3.getKey();

                        insert = "insert into grafana_iotrace.threads (collection_id, id, process_id) values("
                                + uuidToBin(uuid) + ", " + threadId + ", " + processId + ");";
                        bufferedWriter.write(insert + System.lineSeparator());

                        FunctionEvent fe = e3.getValue().getFirstFunctionEvent();
                        while (fe != null) {
                            String type;
                            if (fe.getFileRange() == null) {
                                type = "";
                            } else {
                                type = fe.getFileRange().getRangeType().toString();
                            }

                            insert = "insert into grafana_iotrace.functions "
                                    + "(collection_id, id, name, type, error, time, wrapper_time, bytes, start_time, end_time, thread_id) "
                                    + "values(" + uuidToBin(uuid) + ", " + fe.getId() + ", "
                                    + getSqlString(fe.getFunctionName()) + ", " + getSqlString(type) + ", "
                                    + fe.hasError() + ", " + fe.getFunctionTimePeriod() + ", "
                                    + fe.getWrapperTimePeriod() + ", " + fe.getFunctionByteCount() + ", "
                                    + fe.getStartTime() + ", " + fe.getEndTime() + ", " + threadId + ");";
                            bufferedWriter.write(insert + System.lineSeparator());

                            List<FunctionEvent> fes = new LinkedList<>();
                            fes.add(fe);
                            if (fe.hasSame()) {
                                // for function calls which manipulate more than one file
                                fes.addAll(fe.getSame());
                            }

                            for (FunctionEvent event : fes) {
                                FileTrace fileTrace = event.getFileTrace();

                                if (fileTrace != null) {
                                    insert = "insert into grafana_iotrace.function_manipulates_file (collection_id, function_id, file) "
                                            + "values(" + uuidToBin(uuid) + ", " + fe.getId() + ", "
                                            + getSqlString(fileTrace.getFileId().toString()) + ");";
                                    bufferedWriter.write(insert + System.lineSeparator());
                                }
                            }

                            fe = fe.getNextCallInThreadTrace();
                        }
                    }
                }
            }

//			for (Entry<String, BasicTrace<BasicTrace<ThreadTrace>>> e : cluster.getTraces().entrySet()) {
//
//				for (Entry<String, BasicTrace<ThreadTrace>> e2 : e.getValue().getTraces().entrySet()) {
//
//					for (Entry<String, ThreadTrace> e3 : e2.getValue().getTraces().entrySet()) {
//
//						FunctionEvent fe = e3.getValue().getFirstFunctionEvent();
//						while (fe != null) {
//							for (FunctionEvent e4 : fe.getOverlappingFunctionEvents(false)) {
//								insert = "insert into grafana_iotrace.overlapping_function_calls (collection_id, id1, id2) "
//										+ "values(" + uuidToBin(uuid) + ", " + fe.getId() + ", " + e4.getId() + ");";
//								bufferedWriter.write(insert + System.lineSeparator());
//							}
//							for (FunctionEvent e4 : fe.getOverlappingFileRange()) {
//								insert = "insert into grafana_iotrace.overlapping_file_range (collection_id, id1, id2) "
//										+ "values(" + uuidToBin(uuid) + ", " + fe.getId() + ", " + e4.getId() + ");";
//								bufferedWriter.write(insert + System.lineSeparator());
//							}
//
//							fe = fe.getNextCallInThreadTrace();
//						}
//					}
//				}
//			}

            bufferedWriter.close();
        } catch (IOException e) {
            logger.error("Exception during write of csv file", e);
        }
    }


    // - 'Helper'-methods
    private static String getSqlString(String value) {
        if (value != null && !value.isEmpty()) {
            return "\"" + value + "\"";
        } else {
            return "\"\"";
        }
    }

    private String uuidToBin(UUID uuid) {
        return "UNHEX(REPLACE(\"" + uuid.toString() + "\", \"-\", \"\"))";
    }

}