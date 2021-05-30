package iotrace.model.evaluation.function;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;

public class AnalyzeFunction {

	private String className;
	private Class<?> classForFunctionCall;
	private String functionName;
	private Method method = null;
	private Class<?> returnType;

	private ArrayList<Object> parameters = new ArrayList<>();
	private ArrayList<Class<?>> parameterClasses = new ArrayList<>();


	public AnalyzeFunction(
				String function, HashMap<String, String> classNames,
				HashMap<String, AnalyzeFunction> functions)
			throws ReflectiveOperationException,
			SecurityException, IllegalArgumentException {
		super();
		int positionOfPoint = function.indexOf(".");
		int positionOfOpenBracket = function.indexOf("(");
		int positionOfCloseBracket = function.lastIndexOf(")");

		if (positionOfPoint != -1 && positionOfPoint < positionOfOpenBracket) {
			className = function.substring(0, positionOfPoint);
			className = classNames.get(className);
			functionName = function.substring(positionOfPoint + 1, positionOfOpenBracket);
		} else {
			className = null;
			functionName = function.substring(0, positionOfOpenBracket);
		}

		parseParameters(function.substring(positionOfOpenBracket + 1, positionOfCloseBracket), classNames, functions);

		if (className != null) {
			classForFunctionCall = Class.forName(className);
			Class<?>[] params = new Class<?>[parameterClasses.size()];
			parameterClasses.toArray(params);
			method = classForFunctionCall.getDeclaredMethod(functionName, params);

			returnType = method.getReturnType();

		} else {

			if (functions.containsKey(functionName)) {
				AnalyzeFunction f = functions.get(functionName);
				returnType = f.returnType;
			} else {
				throw new NoSuchMethodException();
			}
		}
	}

	private void parseParameters(
				String stringParameters, HashMap<String, String> classNames,
				HashMap<String, AnalyzeFunction> functions) throws ReflectiveOperationException,
			SecurityException, IllegalArgumentException {
		stringParameters = stringParameters.trim();
		int endPos;

		for (int i = 0; i < stringParameters.length(); i++) {
			switch (stringParameters.charAt(i)) {
			case '"':
				endPos = endPosString(stringParameters, i);
				parameters.add(stringParameters.substring(i + 1, endPos));
				parameterClasses.add(String.class);
				i = endPos;
				break;
			case ',':
			case ' ':
			case '\t':
			case '\n':
			case '\r':
			case '\b':
				// ignore
				break;
			default:
				int commaPos = stringParameters.indexOf(",", i);
				int openBracketPos = stringParameters.indexOf("(", i);

				if (commaPos == -1 && openBracketPos == -1) {
					parseEnum(stringParameters.substring(i), classNames);
					i = stringParameters.length();
				} else if ((commaPos != -1 && openBracketPos == -1)
						|| (commaPos != -1 && openBracketPos != -1 && commaPos < openBracketPos)) {
					parseEnum(stringParameters.substring(i, commaPos), classNames);
					i = commaPos;
				} else {
					int startPos = stringParameters.indexOf("(", i);
					endPos = endPosFunction(stringParameters, startPos);
					AnalyzeFunction ftf = new AnalyzeFunction(stringParameters.substring(i, endPos + 1), classNames,
							functions);
					parameters.add(ftf);
					parameterClasses.add(ftf.returnType);
					i = endPos;
				}
			}
		}
	}

	private void parseEnum(String stringEnum, HashMap<String, String> classNames) throws ReflectiveOperationException,
			SecurityException, IllegalArgumentException {
		String[] tokens = stringEnum.split("\\.");
		Class<?> baseClass = Class.forName(classNames.get(tokens[0]));
		Method m = baseClass.getDeclaredMethod("valueOf", String.class);
		Object o = m.invoke(null, tokens[1]);
		parameters.add(o);
		parameterClasses.add(baseClass);
	}

	private int endPosFunction(String stringParameters, int startPos) {
		int brackets = 1;
		for (int i = startPos + 1; i < stringParameters.length(); i++) {
			switch (stringParameters.charAt(i)) {
			case '"':
				i = endPosString(stringParameters, i);
				break;
			case '(':
				brackets++;
				break;
			case ')':
				brackets--;
				break;
			default:
				// ignore
			}

			if (brackets == 0) {
				return i;
			}
		}

		return stringParameters.length();
	}

	private int endPosString(String stringParameters, int startPos) {
		for (int i = startPos + 1; i < stringParameters.length(); i++) {
			if (stringParameters.charAt(i) == '"' && stringParameters.charAt(i - 1) != '\\') {
				return i;
			}
		}
		return stringParameters.length();
	}

	public Object invoke(HashMap<String, Object> objects, HashMap<String, AnalyzeFunction> functions)
			throws ReflectiveOperationException,
			IllegalArgumentException {
		if (className != null) {
			Object[] params = new Object[parameters.size()];

			for (int i = 0; i < parameters.size(); i++) {
				Object param = parameters.get(i);
				params[i] = (param instanceof AnalyzeFunction) ? (((AnalyzeFunction)param).invoke(objects, functions)) : (param);
			}

			return method.invoke(objects.get(className), params);
		} else {
			if (functions.containsKey(functionName)) {
				AnalyzeFunction f = functions.get(functionName);
				return f.invoke(objects, functions);
			} else {
				throw new NoSuchMethodException();
			}
		}
	}
}
