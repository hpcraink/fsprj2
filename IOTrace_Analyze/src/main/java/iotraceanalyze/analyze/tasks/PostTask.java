package iotraceanalyze.analyze.tasks;

import iotraceanalyze.analyze.Evaluation;

import java.io.File;
import java.util.Properties;

/**
 * May be any arbitrary post-task (i.e., after {@link Evaluation#processJsons()} has been called)
 * to be performed on different {@link Evaluation}s
 */
public abstract class PostTask {

    private final Properties props;


    public PostTask(Properties props) {
        this.props = props;

        // Create dir
        new File(props.getProperty("workingDir")).mkdirs();
    }


    public abstract void performTask(Evaluation eval);


    // - Getters
    public Properties getProps() {
        return props;
    }

    public String getWorkingDir() {
        return props.getProperty("workingDir");
    }

    public String getInputFile() {
        return props.getProperty("inputFile");
    }

    public String getOutputFolder() {
        return props.getProperty("outputFolder");
    }
}
