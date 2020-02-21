package iotrace.analyze;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;

public class AnalyzeFunctionPool {
	private HashMap<String, String> classNames = new HashMap<>();
	private HashMap<String, AnalyzeFunction> functions = new HashMap<>();
	private HashMap<String, Object> objects = new HashMap<>();

	public AnalyzeFunctionPool(String properties) throws IOException, ClassNotFoundException, NoSuchMethodException,
			SecurityException, IllegalAccessException, IllegalArgumentException, InvocationTargetException {
		InputStream is = this.getClass().getResourceAsStream(properties);
		byte[] buffer = new byte[is.available()];
		is.read(buffer);
		is.close();
		String tmpProperties = new String(buffer);

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

	public void cmd(String functionName, String cmd)
			throws IllegalAccessException, IllegalArgumentException, InvocationTargetException, NoSuchMethodException {
		String tmpFunctionName = functionName + "_" + cmd;

		if (containsFunctionName(tmpFunctionName)) {
			invoke(tmpFunctionName);
		} else {
			// TODO: default/Warning
			throw new NoSuchMethodException();
		}
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

	public Object invoke(String functionName)
			throws IllegalAccessException, IllegalArgumentException, InvocationTargetException, NoSuchMethodException {
		return functions.get(functionName).invoke(objects, functions);
	}
}
