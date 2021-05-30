package iotraceanalyze.model.logs;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.Map.Entry;

import iotraceanalyze.analyze.Evaluation;
import iotraceanalyze.model.analysis.function.AnalyzeFunctionPool;
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

	private TreeMap<String, LinkedList<String>> arrays = new TreeMap<>();
	private TreeMap<String, LinkedList<Json>> objectArrays = new TreeMap<>();


	public Json(String jsonStr) {
		parseJson(jsonStr);
	}



	// -- ?? only internal use + JUnit tests ?? --
	public boolean containsElement(String name) {
		return elements.containsKey(name);
	}
	public boolean containsValueArray(String name) {
		return arrays.containsKey(name);
	}

	public boolean containsObject(String name) {
		return objects.containsKey(name);
	}
	public boolean containsObjectArray(String name) {
		return objectArrays.containsKey(name);
	}

	public String getElement(String name) {
		return elements.get(name);
	}
	public LinkedList<String> getValueArray(String name) {
		return arrays.get(name);
	}

	public Json getObject(String name) {
		return objects.get(name);
	}
	public LinkedList<Json> getObjectArray(String name) {
		return objectArrays.get(name);
	}
	// -- ?? only internal use + JUnit tests ?? --



	// --------------------------------------- ?? Evaluation properties file stuff ?? ----------------------------------------
	public boolean hasValue(String path, String value) {
		String foundValue = getValue(path);
		if (foundValue != null) {
			return foundValue.equals(value);
		}

		return false;
	}

	public String getValue(String path) {
		String[] pathTokens = path.split("/");
		if (pathTokens.length < 1) {
			return null;
		}

		String elementKey = pathTokens[pathTokens.length - 1];
		if (elementKey.length() <= 0) {
			return null;
		}

		Json json = getObjectFromPath(pathTokens, pathTokens.length - 1);
		if (json == null) {
			return null;
		}

		if (json.containsElement(elementKey)) {
			return json.getElement(elementKey);
		}

		return null;
	}

	public boolean hasArrayValue(String path, String value) {
		LinkedList<String> tmpArray = getValueArrayFromPath(path);

		if (tmpArray != null) {
			return tmpArray.contains(value);
		}

		return false;
	}

	public LinkedList<String> getValueArrayFromPath(String path) {
		Json json;
		String[] pathTokens = path.split("/");

		if (pathTokens.length < 1) {
			return null;
		}

		String array = pathTokens[pathTokens.length - 1];
		if (array.length() <= 0) {
			return null;
		}

		json = getObjectFromPath(pathTokens, pathTokens.length - 1);
		if (json == null) {
			return null;
		}

		if (json.containsValueArray(array)) {
			return json.getValueArray(array);
		}

		return null;
	}

	// - 'Helper'-methods
	private Json getObjectFromPath(String[] pathTokens, int depth) {
		Json json = this;

		for (int i = 0; i < depth; i++) {
			String token = pathTokens[i];
			if (token.length() <= 0) {
				return null;
			}

			if (json.containsObject(token)) {
				json = json.getObject(token);
			} else {
				return null;
			}
		}

		return json;
	}

	// - Utils
	public LinkedList<String> combineObjectValuesToList(String pathToObjectArray, String pathToElementInObject) {
	
		LinkedList<String> tmpList = new LinkedList<>();
		LinkedList<Json> tmpJsonList = null;
		
		Json json = null;
		
		String[] objectArrayPathTokens = pathToObjectArray.split("/");

		if (objectArrayPathTokens.length < 1) {
			return null;
		}

		String array = objectArrayPathTokens[objectArrayPathTokens.length - 1];
		if (array.length() <= 0) {
			return null;
		}

		json = getObjectFromPath(objectArrayPathTokens, objectArrayPathTokens.length - 1);
		if (json == null) {
			return null;
		}
		
		tmpJsonList = json.getObjectArray(objectArrayPathTokens[objectArrayPathTokens.length - 1]);
		
		for(Json j : tmpJsonList) {

			String tmpReturn = j.getValue(pathToElementInObject);
			if (tmpReturn != null) {
				tmpList.add(tmpReturn);
			}
		}
		
		return tmpList;
	}

	public static LinkedList<String> mergeArrays(LinkedList<String> arr1, LinkedList<String> arr2) {
		if (arr1 == null) {
			return arr2;
		}
		if (arr2 == null) {
			return arr1;
		}

		LinkedList<String> tmpArray = new LinkedList<>(arr1);
		for (String s : arr2) {
			tmpArray.remove(s);
			tmpArray.add(s);
		}

		return tmpArray;
	}

	public static String concat(String s1, String s2) {
		return s1 + s2;
	}


	// - Misc.
	public static boolean getTrue() {
		return true;
	}

	public static boolean getFalse() {
		return false;
	}
	// --------------------------------------- ?? Evaluation properties file stuff ?? ----------------------------------------



	// --- ??? ----
	public static Map<UniqueStartTime, Json> jsonLogToObjectMap(File file, AnalyzeFunctionPool fileTraceFunctions) {
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
					time_start = (String) fileTraceFunctions.invoke(Evaluation.JSON_GET_START_TIME);
				} catch (ReflectiveOperationException | IllegalArgumentException e) {
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

	public static Map<String, Map<String, TreeMap<UniqueStartTime, Json>>> workingDirLogFileToJsonObjectMap(File file,
																											AnalyzeFunctionPool workingDirFunctions) {
		Map<String, Map<String, TreeMap<UniqueStartTime, Json>>> data = new HashMap<>();

		try (BufferedReader br = new BufferedReader(new FileReader(file))) {
			String line;
			while ((line = br.readLine()) != null) {
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
				} catch (ReflectiveOperationException | IllegalArgumentException e) {
					logger.error("Exception during invokation of method for creating working dir for json " + tmpJson, e);
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
			}
		} catch (IOException e) {
			logger.error("Exception during read from " + file.getName(), e);
			e.printStackTrace();
		}

		return data;
	}
	// --- ??? ----



	// - 'Helper'-methods
	// JSON-Parsing stuff
	private void parseJson(String jsonStr) {
		jsonStr = jsonStr.trim();

		// Validate Json
		if (jsonStr.charAt(0) != '{')
			throw new IllegalArgumentException("String doesn't represent a valid jsonStr-object. Missing '{'.");

		if (jsonStr.charAt(jsonStr.length() - 1) != '}')
			throw new IllegalArgumentException("String doesn't represent a valid jsonStr-object. Missing '}'.");

		jsonStr = jsonStr.substring(1, jsonStr.length() - 1);
		if (jsonStr.charAt(0) != '"')
			throw new IllegalArgumentException(
					"String doesn't represent a valid jsonStr-object. Missing name for first element.");


		// Parse ...
		boolean nameFound = false;
		boolean stringValueFound = false;
		boolean arrayValueFound = false;
		boolean objectValueFound = false;

		int endPos;

		String name = new String();
		String value = new String();
		for (int i = 0; i < jsonStr.length(); i++) {

			// identifies next token
			switch (jsonStr.charAt(i)) {
				case '"':
					endPos = endPosString(jsonStr, i);
					if (!nameFound) {
						name = jsonStr.substring(i + 1, endPos);
						nameFound = true;
					} else {
						value = jsonStr.substring(i + 1, endPos);
						stringValueFound = true;
					}
					i = endPos;
					break;
				case '[':
					if (!nameFound) {
						throw new IllegalArgumentException(
								"String doesn't represent a valid jsonStr-object. No name for array-value.");
					} else {
						endPos = endPosArray(jsonStr, i);
						value = jsonStr.substring(i + 1, endPos);
						arrayValueFound = true;
						i = endPos;
					}
					break;
				case '{':
					if (!nameFound) {
						throw new IllegalArgumentException(
								"String doesn't represent a valid jsonStr-object. No name for object-value.");
					} else {
						endPos = endPosObject(jsonStr, i);
						value = jsonStr.substring(i, endPos + 1);
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
								"String doesn't represent a valid jsonStr-object. No name for value.");
					} else {
						endPos = endPosValue(jsonStr, i);
						value = jsonStr.substring(i, endPos);
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
					parseArray(name, value);
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

	private void parseArray(String name, String jsonArray) {
		LinkedList<String> array = new LinkedList<>();
		LinkedList<Json> objectArray = new LinkedList<>();

		int endPos;
		String value;
		Json objectValue;
		for (int i = 0; i < jsonArray.length(); i++) {
			switch (jsonArray.charAt(i)) {
				case '"':
					endPos = endPosString(jsonArray, i);
					value = jsonArray.substring(i + 1, endPos);
					array.add(value);
					i = endPos;
					break;
				case '{':
					endPos = endPosObject(jsonArray, i);
					objectValue = new Json(jsonArray.substring(i, endPos + 1));
					objectArray.add(objectValue);
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

		if (!array.isEmpty()) {
			arrays.put(name, array);
		}
		if (!objectArray.isEmpty()) {
			objectArrays.put(name, objectArray);
		}
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


	// - Misc.
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
		StringBuilder tmp = new StringBuilder(100);
		tmp.append("{");

		for (Entry<String, String> e : elements.entrySet()) {
			if (tmp.charAt(tmp.length()-1) != '{') {
				tmp.append(",");
			}
			tmp.append("\"").append(e.getKey()).append("\":\"").append(e.getValue()).append("\"");
		}

		for (Entry<String, LinkedList<String>> e : arrays.entrySet()) {
			if (tmp.charAt(tmp.length()-1) != '{') {
				tmp.append(",");
			}
			tmp.append("\"").append(e.getKey()).append("\":[");
			for (String a : e.getValue()) {
				if (tmp.charAt(tmp.length()-1) != '[') {
					tmp.append(",");
				}
				tmp.append("\"").append(a).append("\"");
			}
			if (objectArrays.containsKey(e.getKey())) {
				for (Json j : objectArrays.get(e.getKey())) {
					if (tmp.charAt(tmp.length()-1) != '[') {
						tmp.append(",");
					}
					tmp.append(j);
				}
			}
			tmp.append("]");
		}

		for (Entry<String, LinkedList<Json>> e : objectArrays.entrySet()) {
			if (!arrays.containsKey(e.getKey())) {
				if (tmp.charAt(tmp.length()-1) != '{') {
					tmp.append(",");
				}
				tmp.append("\"").append(e.getKey()).append("\":[");
				for (Json j : e.getValue()) {
					if (tmp.charAt(tmp.length()-1) != '[') {
						tmp.append(",");
					}
					tmp.append(j);
				}
				tmp.append("]");
			}
		}

		for (Entry<String, Json> e : objects.entrySet()) {
			if (tmp.charAt(tmp.length()-1) != '{') {
				tmp.append(",");
			}
			tmp.append("\"").append(e.getKey()).append("\":").append(e.getValue());
		}

		tmp.append("}");

		return tmp.toString();
	}
}
