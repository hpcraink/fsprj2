package iotrace.analyze;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.TreeMap;
import java.util.UUID;

public class Json {
	private static final Logger logger = LogManager.getLogger(Json.class);

	private UUID uuid = UUID.randomUUID();
	// TODO: differ between strings and other value-types
	private TreeMap<String, String> elements = new TreeMap<>();
	private TreeMap<String, Json> objects = new TreeMap<>();
	// TODO: arrays and objects as array elements (e.g. see hasArrayValue)
	private TreeMap<String, LinkedList<String>> arrays = new TreeMap<>();

	public Json(String json) {
		super();
		parseJson(json);
	}

	private void parseJson(String json) {
		json = json.trim();
		if (json.charAt(0) != '{') {
			throw new IllegalArgumentException("String doesn't represent a valid json-object. Missing '{'.");
		}
		if (json.charAt(json.length() - 1) != '}') {
			throw new IllegalArgumentException("String doesn't represent a valid json-object. Missing '}'.");
		}
		json = json.substring(1, json.length() - 1);
		if (json.charAt(0) != '"') {
			throw new IllegalArgumentException(
					"String doesn't represent a valid json-object. Missing name for first element.");
		}

		boolean nameFound = false;
		boolean stringValueFound = false;
		boolean arrayValueFound = false;
		boolean objectValueFound = false;
		int endPos;
		String name = new String();
		String value = new String();
		for (int i = 0; i < json.length(); i++) {

			// identifies next token
			switch (json.charAt(i)) {
			case '"':
				endPos = endPosString(json, i);
				if (!nameFound) {
					name = json.substring(i + 1, endPos);
					nameFound = true;
				} else {
					value = json.substring(i + 1, endPos);
					stringValueFound = true;
				}
				i = endPos;
				break;
			case '[':
				if (!nameFound) {
					throw new IllegalArgumentException(
							"String doesn't represent a valid json-object. No name for array-value.");
				} else {
					endPos = endPosArray(json, i);
					value = json.substring(i + 1, endPos);
					arrayValueFound = true;
					i = endPos;
				}
				break;
			case '{':
				if (!nameFound) {
					throw new IllegalArgumentException(
							"String doesn't represent a valid json-object. No name for object-value.");
				} else {
					endPos = endPosObject(json, i);
					value = json.substring(i, endPos + 1);
					objectValueFound = true;
					i = endPos;
				}
				break;
			case ':':
			case ',':
			case ' ':
			case '\t':
			case '\n':
			case '\r':
			case '\b':
				// ignore whitespace and delimiter between tokens
				break;
			default:
				if (!nameFound) {
					throw new IllegalArgumentException(
							"String doesn't represent a valid json-object. No name for value.");
				} else {
					endPos = endPosValue(json, i);
					value = json.substring(i, endPos);
					stringValueFound = true;
					i = endPos;
				}
				break;
			}

			// set element
			if (nameFound) {

				name = name.intern(); // save a lot of memory (keys of
										// JSON-elements are often equal)

				if (stringValueFound) {

					if (!name.equals("time_start") && !name.equals("time_end")) {
						value = value.intern(); // save a bit more of memory
					}

					elements.put(name, value);
					nameFound = false;
					stringValueFound = false;
				} else if (arrayValueFound) {
					arrays.put(name, parseArray(value));
					nameFound = false;
					arrayValueFound = false;
				} else if (objectValueFound) {
					objects.put(name, new Json(value));
					nameFound = false;
					objectValueFound = false;
				}
			}
		}
	}

	private LinkedList<String> parseArray(String jsonArray) {
		LinkedList<String> array = new LinkedList<>();

		int endPos;
		String value;
		for (int i = 0; i < jsonArray.length(); i++) {
			switch (jsonArray.charAt(i)) {
			case '"':
				endPos = endPosString(jsonArray, i);
				value = jsonArray.substring(i + 1, endPos);
				array.add(value);
				i = endPos;
				break;
			case ',':
			case ' ':
			case '\t':
			case '\n':
			case '\r':
			case '\b':
				// ignore whitespace and delimiter between tokens
				break;
			default:
				endPos = endPosValue(jsonArray, i);
				value = jsonArray.substring(i, endPos).trim();
				array.add(value);
				i = endPos;
				break;
			}
		}

		return array;
	}

	private int endPosValue(String json, int startPos) {
		for (int i = startPos + 1; i < json.length(); i++) {
			switch (json.charAt(i)) {
			case '"':
				i = endPosString(json, i);
				break;
			case '{':
				i = endPosObject(json, i);
				break;
			case '[':
				i = endPosArray(json, i);
				break;
			case ',':
				return i;
			default:
				// check next char
			}
		}
		return json.length();
	}

	private int endPosString(String json, int startPos) {
		for (int i = startPos + 1; i < json.length(); i++) {
			if (json.charAt(i) == '"' && json.charAt(i - 1) != '\\') {
				return i;
			}
		}
		return json.length();
	}

	private int endPosArray(String json, int startPos) {
		for (int i = startPos + 1; i < json.length(); i++) {
			switch (json.charAt(i)) {
			case '"':
				i = endPosString(json, i);
				break;
			case '{':
				i = endPosObject(json, i);
				break;
			case '[':
				i = endPosArray(json, i);
				break;
			case ']':
				return i;
			default:
				// check next char
			}
		}
		return json.length();
	}

	private int endPosObject(String json, int startPos) {
		for (int i = startPos + 1; i < json.length(); i++) {
			switch (json.charAt(i)) {
			case '"':
				i = endPosString(json, i);
				break;
			case '{':
				i = endPosObject(json, i);
				break;
			case '[':
				i = endPosArray(json, i);
				break;
			case '}':
				return i;
			default:
				// check next char
			}
		}
		return json.length();
	}

	public boolean containsElement(String name) {
		return elements.containsKey(name);
	}

	public boolean containsObject(String name) {
		return objects.containsKey(name);
	}

	public boolean containsArray(String name) {
		return arrays.containsKey(name);
	}

	public String getElement(String name) {
		return elements.get(name);
	}

	public Json getObject(String name) {
		return objects.get(name);
	}

	public LinkedList<String> getArray(String name) {
		return arrays.get(name);
	}

	public String getValue(String path) {
		Json json;
		String[] tokens = path.split("/");

		if (tokens.length < 1) {
			return null;
		}

		String element = tokens[tokens.length - 1];
		if (element.length() <= 0) {
			return null;
		}

		json = getObjectFromPath(tokens, tokens.length - 1);
		if (json == null) {
			return null;
		}

		if (json.containsElement(element)) {
			return json.getElement(element);
		}

		return null;
	}

	public boolean hasValue(String path, String value) {
		String tmpValue = getValue(path);

		if (tmpValue != null) {
			return tmpValue.equals(value);
		}

		return false;
	}

	public static boolean getTrue() {
		return true;
	}

	public static boolean getFalse() {
		return false;
	}

	public static String concat(String s1, String s2) {
		return s1 + s2;
	}

	public LinkedList<String> getArrayFromPath(String path) {
		Json json;
		String[] tokens = path.split("/");

		if (tokens.length < 1) {
			return null;
		}

		String array = tokens[tokens.length - 1];
		if (array.length() <= 0) {
			return null;
		}

		json = getObjectFromPath(tokens, tokens.length - 1);
		if (json == null) {
			return null;
		}

		if (json.containsArray(array)) {
			return json.getArray(array);
		}

		return null;
	}

	public boolean hasArrayValue(String path, String value) {
		LinkedList<String> tmpArray = getArrayFromPath(path);

		if (tmpArray != null) {
			return tmpArray.contains(value);
		}

		return false;
	}

	public static LinkedList<String> mergeArrays(LinkedList<String> a1, LinkedList<String> a2) {
		if (a1 == null) {
			return a2;
		}
		if (a2 == null) {
			return a1;
		}

		LinkedList<String> tmpArray = new LinkedList<>(a1);
		for (String s : a2) {
			tmpArray.remove(s);
			tmpArray.add(s);
		}

		return tmpArray;
	}

	private Json getObjectFromPath(String[] tokens, int length) {
		Json json = this;

		for (int i = 0; i < length; i++) {
			String s = tokens[i];
			if (s.length() <= 0) {
				return null;
			}

			if (json.containsObject(s)) {
				json = json.getObject(s);
			} else {
				return null;
			}
		}

		return json;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((uuid == null) ? 0 : uuid.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		Json other = (Json) obj;
		if (uuid == null) {
			if (other.uuid != null)
				return false;
		} else if (!uuid.equals(other.uuid))
			return false;
		return true;
	}

	@Override
	public String toString() {
		String tmp = "{";

		for (Entry<String, String> e : elements.entrySet()) {
			if (!tmp.endsWith("{")) {
				tmp += ",";
			}
			tmp += "\"" + e.getKey() + "\":\"" + e.getValue() + "\"";
		}

		for (Entry<String, LinkedList<String>> e : arrays.entrySet()) {
			if (!tmp.endsWith("{")) {
				tmp += ",";
			}
			tmp += "\"" + e.getKey() + "\":[";
			for (String a : e.getValue()) {
				if (!tmp.endsWith("[")) {
					tmp += ",";
				}
				tmp += "\"" + a + "\"";
			}
			tmp += "]";
		}

		for (Entry<String, Json> e : objects.entrySet()) {
			if (!tmp.endsWith("{")) {
				tmp += ",";
			}
			tmp += "\"" + e.getKey() + "\":" + e.getValue();
		}

		tmp += "}";

		return tmp;
	}

	public static Map<UniqueStartTime, Json> logFileToJson(File file, AnalyzeFunctionPool fileTraceFunctions) {
		Map<UniqueStartTime, Json> data = new TreeMap<>();
		final int step = 50000;

		try (BufferedReader br = new BufferedReader(new FileReader(file))) {
			int lineCount = 0;
			int tmpLineCount = 0;

			logger.debug("Start reading Json-Data:");

			String line = br.readLine();
			while (line != null) {
				Json tmpJson;
				String time_start;

				tmpJson = new Json(line);
				fileTraceFunctions.addObject(tmpJson);
				try {
					time_start = (String) fileTraceFunctions.invoke(Data.JSON_GET_START_TIME);
				} catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException
						| NoSuchMethodException e) {
					logger.error("Exception getting start time from json", e);
					return data;
				}
				data.put(new UniqueStartTime(Long.parseLong(time_start)), tmpJson);

				tmpLineCount++;
				if (tmpLineCount == step) {
					lineCount += tmpLineCount;
					tmpLineCount = 0;
					logger.debug("    {} lines read", lineCount);
				}

				line = br.readLine();
			}

			logger.debug("    {} lines read", lineCount + tmpLineCount);

		} catch (IOException e) {
			logger.error("Exception during read of json data", e);
		}

		return data;
	}

	public static Map<String, Map<String, TreeMap<UniqueStartTime, Json>>> workingDirFileToJson(File file,
			AnalyzeFunctionPool workingDirFunctions) {
		Map<String, Map<String, TreeMap<UniqueStartTime, Json>>> data = new HashMap<>();

		try (BufferedReader br = new BufferedReader(new FileReader(file))) {
			String line = br.readLine();
			while (line != null) {
				Json tmpJson;
				String host;
				String process;
				String time;

				tmpJson = new Json(line);
				workingDirFunctions.addObject(tmpJson);
				try {
					host = (String) workingDirFunctions.invoke("getHostName");
					process = (String) workingDirFunctions.invoke("getProcessId");
					time = (String) workingDirFunctions.invoke("getTime");
				} catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException
						| NoSuchMethodException e) {
					logger.error("Exception during invokation of method for creating working dir for json " + tmpJson,
							e);
					return data;
				}

				Map<String, TreeMap<UniqueStartTime, Json>> hostMap;
				if (data.containsKey(host)) {
					hostMap = data.get(host);
				} else {
					hostMap = new HashMap<>();
					data.put(host, hostMap);
				}
				TreeMap<UniqueStartTime, Json> processMap;
				if (hostMap.containsKey(process)) {
					processMap = hostMap.get(process);
				} else {
					processMap = new TreeMap<>();
					hostMap.put(process, processMap);
				}
				processMap.put(new UniqueStartTime(Long.parseLong(time)), tmpJson);

				line = br.readLine();
			}

		} catch (IOException e) {
			logger.error("Exception during read from " + file.getName(), e);
			e.printStackTrace();
		}

		return data;
	}
}
