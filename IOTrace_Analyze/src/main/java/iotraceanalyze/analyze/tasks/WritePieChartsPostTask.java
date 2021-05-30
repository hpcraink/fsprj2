package iotraceanalyze.analyze.tasks;

import iotraceanalyze.analyze.Evaluation;
import iotraceanalyze.model.analysis.trace.KeyValueTreeNode;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;
import iotraceanalyze.model.analysis.trace.traceables.Traceable;
import iotraceanalyze.analyze.misc.visualization.icebergChart.PieChart;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.*;

public class WritePieChartsPostTask extends PostTask {

    private static final Logger logger = LogManager.getLogger(WritePieChartsPostTask.class);


    // $$$ TODO: REFACTOR (pass differently to methods) $$$
    private Evaluation eval;


    public WritePieChartsPostTask(Properties props) {
        super(props);
    }


    public void performTask(Evaluation evaluation) {
        Properties props = super.getProps();
        String filePrefix = super.getOutputFolder() + super.getInputFile();

        this.eval = evaluation;


        double percent = Double.parseDouble(props.getProperty("pieChartMinPercent", "3.0"));
        int initialWidth = Integer.parseInt(props.getProperty("pieChartInitialWidth", "200"));
        int incrementWidth = Integer.parseInt(props.getProperty("pieChartIncrementWidth", "200"));
        int border = Integer.parseInt(props.getProperty("pieChartBorder", "100"));
        int fontSize = Integer.parseInt(props.getProperty("pieChartFontSize", "40"));
        int titleSize = Integer.parseInt(props.getProperty("pieChartTitleSize", "60"));


        // - ...
        printMultiPieCharts(filePrefix, percent, initialWidth, incrementWidth, border, fontSize, titleSize);
        printOverlappingAsPie(filePrefix, percent, initialWidth, incrementWidth, border, fontSize, titleSize);
        printForkAsPie(filePrefix, percent, initialWidth, incrementWidth, border, fontSize, titleSize);
    }


    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    private void printMultiPieCharts(String filePrefix,
                                     double percent, int initialWidth, int incrementWidth, int border, int fontSize, int titleSize) {
        ArrayList<FileTrace.FileType> kinds = new ArrayList<>();
        kinds.add(FileTrace.FileType.FILE);
        kinds.add(FileTrace.FileType.TMPFILE);

        KeyValueTreeNode node = eval.getCluster().getKeyValueTree(kinds, Traceable.ValueType.FUNCTION_TIME, 2, 6, percent, eval.getLegends());
        new PieChart(node,
                eval.getLegends().getString("pieChartFunctionTimeTitleBeforePercent") + percent
                        + eval.getLegends().getString("pieChartFunctionTimeTitleAfterPercent"),
                filePrefix, "_time_pie", true, false, initialWidth, incrementWidth, border, fontSize, titleSize);

        node = eval.getCluster().getKeyValueTree(kinds, Traceable.ValueType.BYTES, 2, 6, percent, eval.getLegends());
        new PieChart(node,
                eval.getLegends().getString("pieChartBytesTitleBeforePercent") + percent
                        + eval.getLegends().getString("pieChartBytesTitleAfterPercent"),
                filePrefix, "_bytes_pie", true, false, initialWidth, incrementWidth, border, fontSize, titleSize);

        if (eval.isHasWrapperInfo()) {
            node = eval.getCluster().getKeyValueTree(kinds, Traceable.ValueType.WRAPPER_TIME, 2, 6, percent, eval.getLegends());
            new PieChart(node,
                    eval.getLegends().getString("pieChartWrapperTimeTitleBeforePercent") + percent
                            + eval.getLegends().getString("pieChartWrapperTimeTitleAfterPercent"),
                    filePrefix, "_wrapper_time_pie", true, false, initialWidth, incrementWidth, border, fontSize,
                    titleSize);
        }
    }
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    public void printOverlappingAsPie(String filePrefix,
                                      double percent, int initialWidth, int incrementWidth, int border, int fontSize, int titleSize) {
        ArrayList<FileTrace.FileType> kinds = new ArrayList<>();
        kinds.add(FileTrace.FileType.FILE);
        kinds.add(FileTrace.FileType.TMPFILE);

        KeyValueTreeNode node = eval.getFileTraces().getKeyValueTree(kinds, Traceable.ValueType.OVERLAPPING_FILE_RANGE, 2, 6, percent,
                eval.getLegends());
        new PieChart(node,
                eval.getLegends().getString("pieChartOverlappingRangeTitleBeforePercent") + percent
                        + eval.getLegends().getString("pieChartOverlappingRangeTitleAfterPercent"),
                filePrefix, "_overlapping_range_pie", true, true, initialWidth, incrementWidth, border, fontSize,
                titleSize);

        node = eval.getFileTraces().getKeyValueTree(kinds, Traceable.ValueType.OVERLAPPING_FUNCTIONS, 2, 6, percent, eval.getLegends());
        new PieChart(node,
                eval.getLegends().getString("pieChartOverlappingFunctionsTitleBeforePercent") + percent
                        + eval.getLegends().getString("pieChartOverlappingFunctionsTitleAfterPercent"),
                filePrefix, "_overlapping_function_pie", true, true, initialWidth, incrementWidth, border, fontSize,
                titleSize);
    }
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    public void printForkAsPie(String filePrefix, double percent, int initialWidth, int incrementWidth, int border,
                               int fontSize, int titleSize) {
        prepareForkTree(1, eval.getFork());

        new PieChart(eval.getFork(), eval.getLegends().getString("pieChartForkTitle"), filePrefix, "_fork_hierarchy_pie", false, true,
                initialWidth, incrementWidth, border, fontSize, titleSize);
    }

    private void prepareForkTree(int countNeighbours, KeyValueTreeNode node) {
        node.changeValue(100.0 / countNeighbours);

        int count = node.countChildren();
        for (KeyValueTreeNode n : node.getChildren()) {
            prepareForkTree(count, n);
        }
    }
    // ------------------------------------------------------------------------------------------------------------------------------------------------------------------

}
