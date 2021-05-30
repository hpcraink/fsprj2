package iotrace.model.evaluation.function;

import iotrace.analyze.Evaluation;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;

public class AnalyzeFunctionPool {

    private HashMap<String, String> classNames = new HashMap<>();
    private HashMap<String, AnalyzeFunction> functions = new HashMap<>();
    private HashMap<String, Object> objects = new HashMap<>();


    public AnalyzeFunctionPool(String propsPath) throws IOException, ReflectiveOperationException,
            IllegalArgumentException, SecurityException {
        String tmpProperties = getProperties(propsPath);
        parseProperties(tmpProperties);
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


    // - Helper-methods
    private static String getProperties(String propertiesFilePath) throws IOException {
        InputStream is = Evaluation.class.getResourceAsStream(propertiesFilePath);
        byte[] buffer = new byte[is.available()];
        is.read(buffer);
        is.close();

        return new String(buffer);
    }

    private void parseProperties(String tmpProperties) throws ReflectiveOperationException,
            SecurityException, IllegalArgumentException {
        for (String e : tmpProperties.split("\n")) {
            e = e.trim();

            if (e.length() > 0 && !e.startsWith("#")) {
                int splitPos = e.indexOf("=");
                String key = e.substring(0, splitPos).trim();
                String value = e.substring(splitPos + 1).trim();

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
