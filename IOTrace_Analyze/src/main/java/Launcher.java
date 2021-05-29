import iotrace.analyze.Evaluation;
import iotrace.analyze.tasks.*;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.core.config.ConfigurationSource;
import org.apache.logging.log4j.core.config.Configurator;

import java.io.*;
import java.net.URISyntaxException;
import java.util.*;

public class Launcher {

    private static final Logger logger = LogManager.getLogger(Launcher.class);

    private static final String FILE_TRACE_PROPERTIES = "/FileTrace.properties";
    private static final String FILE_RANGE_PROPERTIES = "/FileRange.properties";
    private static final String FILE_LOCK_PROPERTIES = "/FileLock.properties";
    private static final String WORKING_DIR_PROPERTIES = "/WorkingDir.properties";
    private static final String IOTRACE_ANALYZE_PROPERTIES = "IOTrace_Analyze.properties";

    private static final String LEGEND_BUNDLE = "LegendBundle";



    public static void main(String[] args) {
        logger.info("Starting ...");

        Properties props = configureAndGetConfigProps(args);
        performAnalysis(props);


        logger.info("Finished!");
    }


    // - Helper-methods
    // $$$ TODO: REFACTOR $$$
    private static Properties configureAndGetConfigProps(String[] args) {
        // - Parse CLI args
        final String PROPS_CLI_FLAG = "analyzeprop";
        final String LOG4J_PROPS_CLI_FLAG = "log4jprop";

        Map<String, String> miscCliArgProps = new HashMap<>();

        String propsPath = "";
        for (String argStr : args) {
            argStr = argStr.trim();
            int indexOfEqualSign = argStr.indexOf("=");

            if (argStr.startsWith("-" +LOG4J_PROPS_CLI_FLAG +"=") && indexOfEqualSign >= 0) {
                String log4jPropsPath = argStr.substring(indexOfEqualSign + 1).trim();
                loadAndApplyLogProps(log4jPropsPath);

            } else if (argStr.startsWith("-" +PROPS_CLI_FLAG +"=") && indexOfEqualSign >= 0) {
                propsPath = argStr.substring(indexOfEqualSign + 1).trim();


            } else if (argStr.startsWith("-") && indexOfEqualSign >= 0)
                miscCliArgProps.put(argStr.substring(1, indexOfEqualSign).trim(), argStr.substring(indexOfEqualSign + 1).trim());
        }

        // - Build path of config file
        if (propsPath.isEmpty()) {      // Default (no path for config file passed)
            try {
                propsPath = Launcher.class.getProtectionDomain().getCodeSource().getLocation().toURI().getPath();
            } catch (URISyntaxException e1) {
                e1.printStackTrace();
                throw new RuntimeException(e1);
            }
            propsPath = (new File(propsPath)).getParentFile().getPath() + System.getProperty("file.separator")
                    + IOTRACE_ANALYZE_PROPERTIES;
        }

        // - Load properties from file
        Properties properties = new Properties();
        try {
            properties.load(new FileInputStream(propsPath));
        } catch (IOException e) {
            logger.error("Exception during load of " + propsPath, e);
            logger.error("Current working dir = " + System.getProperty("user.dir"));
            throw new RuntimeException(e);
        }

        // Merge flags passed from CLI w/ config-file properties
        for (Map.Entry<String, String> e : miscCliArgProps.entrySet())
            properties.setProperty(e.getKey(), e.getValue());

        return properties;
    }

    private static void loadAndApplyLogProps(String log4jPropsPath) {
        try {
            InputStream is = new FileInputStream(log4jPropsPath);
            File log4jConfigFile = new File(log4jPropsPath);

            ConfigurationSource source = new ConfigurationSource(is, log4jConfigFile);
            Configurator.initialize(null, source);
        } catch (FileNotFoundException e) {
            logger.error("Exception during load of " + log4jPropsPath, e);
        }
    }

    private static Evaluation newEvaluation(Properties props) {
        String language = props.getProperty("localeLanguage", "en");
        String country = props.getProperty("localeCountry", "US");
        ResourceBundle legends = ResourceBundle.getBundle(LEGEND_BUNDLE, new Locale(language, country));

        try {
            return new Evaluation(FILE_TRACE_PROPERTIES, FILE_RANGE_PROPERTIES, FILE_LOCK_PROPERTIES, WORKING_DIR_PROPERTIES,
                    legends);
        } catch (IOException | ReflectiveOperationException | SecurityException | IllegalArgumentException e) {
            logger.error("Exception during initialization", e);
            throw new RuntimeException(e);
        }
    }

    private static void performAnalysis(Properties props) {
        // - Setup
        Evaluation evaluation = newEvaluation(props);

        // - Perform analysis  ...
        String pathPrefix = props.getProperty("workingDir");
        String inputFile = props.getProperty("inputFile");
        evaluation.addWorkingDirs(new File(pathPrefix + inputFile + "_working_dir.log"));
        evaluation.addJsons(new File(pathPrefix + inputFile + "_iotrace.log"));
        evaluation.processJsons();


        // Perform post-tasks
        if (props.getProperty("writeFileTraces", "false").equalsIgnoreCase("true")) {
            logger.debug("writing workingDirLog traces ...");
            PostTask printFileTraces = new WriteFileTracesPostTask(props);
            printFileTraces.performTask(evaluation);
        }

        if (props.getProperty("writeCsv", "false").equalsIgnoreCase("true")) {
            logger.debug("writing csv ...");
            PostTask writeCsv = new WriteCsvPostTask(props);
            writeCsv.performTask(evaluation);
        }

        if (props.getProperty("writeSql", "false").equalsIgnoreCase("true")) {
            logger.debug("writing sql ...");
            PostTask writeSQL = new WriteSQLPostTask(props);
            writeSQL.performTask(evaluation);
        }

        if (props.getProperty("writePieCharts", "false").equalsIgnoreCase("true")) {
            logger.debug("generating pie charts ...");
            PostTask printPieCharts = new WritePieChartsPostTask(props);
            printPieCharts.performTask(evaluation);
        }

        if (props.getProperty("writeBarCharts", "false").equalsIgnoreCase("true")) {
            logger.debug("generating bar charts ...");
            PostTask printBarCharts = new WriteBarChartsPostTask(props);
            printBarCharts.performTask(evaluation);
        }

        if (props.getProperty("writeGexf", "false").equalsIgnoreCase("true")) {
            logger.debug("writing gexf ...");
            PostTask writeGexf = new WriteGexfPostTask(props);
            writeGexf.performTask(evaluation);
        }

        // $$$ TODO: ?? depends on gexf ?? $$$
        if (props.getProperty("writeAnimations", "false").equalsIgnoreCase("true")) {
            logger.debug("generating animations ...");
            PostTask writeAnimations = new WriteGephiVideoPostTask(props);
            writeAnimations.performTask(evaluation);
        }

        evaluation.printStats();
    }

}