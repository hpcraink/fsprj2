package iotraceanalyze.analyze.tasks;

import iotraceanalyze.analyze.Evaluation;
import iotraceanalyze.model.analysis.FunctionEvent;
import iotraceanalyze.model.analysis.trace.traceables.BasicTrace;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;
import iotraceanalyze.model.analysis.trace.traceables.ThreadTrace;
import iotraceanalyze.analyze.misc.visualization.jfreechart.BarChart;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.*;

public class WriteBarChartsPostTask extends PostTask {
    private static final Logger logger = LogManager.getLogger(WriteBarChartsPostTask.class);

    public WriteBarChartsPostTask(Properties props) {
        super(props);
    }


    @Override
    public void performTask(Evaluation evaluation) {
        ResourceBundle legends = evaluation.getLegends();
        BasicTrace<BasicTrace<BasicTrace<ThreadTrace>>> cluster = evaluation.getCluster();
        boolean hasWrapperInfo = evaluation.isHasWrapperInfo();
        Properties props = super.getProps();
        String filePrefix = super.getOutputFolder() + super.getInputFile();

        int tickLabelFontSize = Integer.parseInt(props.getProperty("barChartTickLabelFontSize", "20"));
        int labelFontSize = Integer.parseInt(props.getProperty("barChartLabelFontSize", "25"));
        int legendFontSize = Integer.parseInt(props.getProperty("barChartLegendFontSize", "25"));
        int titleFontSize = Integer.parseInt(props.getProperty("barChartTitleFontSize", "30"));
        int width = Integer.parseInt(props.getProperty("barChartWidth", "2048"));
        int height = Integer.parseInt(props.getProperty("barChartHight", "1024"));


        // - ...
        ArrayList<FileTrace.FileType> kinds = new ArrayList<>();
        kinds.add(FileTrace.FileType.FILE);
        kinds.add(FileTrace.FileType.TMPFILE);

        List<String> legend = new LinkedList<>();
        legend.add(legends.getString("barChartLegendSumBytes"));
        legend.add(legends.getString("barChartLegendSumTime"));
        if (hasWrapperInfo) {
            legend.add(legends.getString("barChartLegendSumWrapperTime"));
        }
        Map<String, List<Long>> functions = new TreeMap<>();

        for (Map.Entry<String, BasicTrace<BasicTrace<ThreadTrace>>> e : cluster.getTraces().entrySet()) {
            for (Map.Entry<String, BasicTrace<ThreadTrace>> e2 : e.getValue().getTraces().entrySet()) {
                for (Map.Entry<String, ThreadTrace> e3 : e2.getValue().getTraces().entrySet()) {
                    FunctionEvent fe = e3.getValue().getFirstFunctionEvent();
                    while (fe != null) {

                        if (fe.getFileTrace() != null && kinds.contains(fe.getFileTrace().getFileType())) {

                            /**
                             * index 0: bytes, index 1: function time, index 2: wrapper time
                             */
                            List<Long> values;
                            if (functions.containsKey(fe.getFunctionName())) {
                                values = functions.get(fe.getFunctionName());
                            } else {
                                values = new LinkedList<>();
                                values.add(0l);
                                values.add(0l);
                                if (hasWrapperInfo) {
                                    values.add(0l);
                                }
                                functions.put(fe.getFunctionName(), values);
                            }

                            values.set(0, values.get(0) + fe.getFunctionByteCount());
                            values.set(1, values.get(1) + fe.getFunctionTimePeriod());
                            if (hasWrapperInfo) {
                                values.set(2, values.get(2) + fe.getWrapperTimePeriod());
                            }
                        }

                        fe = fe.getNextCallInThreadTrace();
                    }
                }
            }
        }

        new BarChart(functions, legend, legends.getString("barChartTitle"), filePrefix, "_function_summary",
                tickLabelFontSize, labelFontSize, legendFontSize, titleFontSize, width, height, legends);
    }

}