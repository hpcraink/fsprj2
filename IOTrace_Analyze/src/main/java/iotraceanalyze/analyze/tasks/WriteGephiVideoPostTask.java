package iotraceanalyze.analyze.tasks;

import iotraceanalyze.analyze.Evaluation;
import iotraceanalyze.analyze.misc.visualization.gephi.GephiVideo;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.*;

public class WriteGephiVideoPostTask extends PostTask {
    private static final Logger logger = LogManager.getLogger(WriteGephiVideoPostTask.class);

    public WriteGephiVideoPostTask(Properties props) {
        super(props);
    }


    @Override
    public void performTask(Evaluation evaluation) {
        // - ...
        GephiVideo.generate(super.getOutputFolder(), super.getInputFile(), super.getProps(), evaluation.getLegends());
    }

}
