package iotraceanalyze.model.analysis.function;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

public class AnalyzeFunctionPool {

    private Map<String, String> classNames = new HashMap<>();
    private Map<String, AnalyzeFunction> functions = new HashMap<>();
    private Map<String, Object> objects = new HashMap<>();


    public AnalyzeFunctionPool(String propsPath) throws IOException, ReflectiveOperationException,
            IllegalArgumentException, SecurityException {
        String tmpProperties = getProperties(propsPath);
        parseProps(tmpProperties);
    }


    public boolean containsFunctionName(String functionName) {
        return functions.containsKey(functionName);
    }

    public void addObject(Object object) {
        this.objects.put(object.getClass().getName(), object);
    }

    public void removeObject(Object object) {
        this.objects.remove(object.getClass().getName());
    }


    public Object invoke(String functionName) throws ReflectiveOperationException,
            IllegalArgumentException {
        return functions.get(functionName).invoke(objects, functions);
    }


    // --------------------------------------- ?? Evaluation properties file stuff ?? ----------------------------------------
    public void cmd(String functionName, String cmd) throws ReflectiveOperationException,
            IllegalArgumentException {
        String tmpFunctionName = functionName + "_" + cmd;

        if (containsFunctionName(tmpFunctionName)) {
            invoke(tmpFunctionName);
        } else {
            // TODO: default/Warning
            throw new NoSuchMethodException();
        }
    }
    // --------------------------------------- ?? Evaluation properties file stuff ?? ----------------------------------------


    // - 'Helper'-methods
    private static String getProperties(String propsFilePath) throws IOException {
        InputStream is = AnalyzeFunctionPool.class.getResourceAsStream(propsFilePath);
        byte[] buffer = new byte[is.available()];
        is.read(buffer);
        is.close();

        return new String(buffer);
    }

    private void parseProps(String propsStr) throws ReflectiveOperationException,
            SecurityException, IllegalArgumentException {
        for (String line : propsStr.split("\n")) {
            line = line.trim();

            if (line.length() > 0 && !line.startsWith("#")) {

                int splitPos = line.indexOf("=");
                String key = line.substring(0, splitPos).trim();
                String value = line.substring(splitPos + 1).trim();

                if (key.equals("ClassNames")) {
                    for (String s : value.split(";")) {
                        int classNameSplitPos = s.indexOf("=");
                        classNames.put(s.substring(0, classNameSplitPos), s.substring(classNameSplitPos + 1));
                    }
                } else {
                    AnalyzeFunction callFunction = new AnalyzeFunction(value, classNames, functions);
                    functions.put(key, callFunction);
                }
            }
        }
    }
}
