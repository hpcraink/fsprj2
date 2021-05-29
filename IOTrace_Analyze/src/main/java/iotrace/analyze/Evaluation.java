package iotrace.analyze;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.ResourceBundle;
import java.util.Set;
import java.util.SortedMap;

import iotrace.analyze.function.AnalyzeFunctionPool;
import iotrace.model.analysis.*;
import iotrace.model.analysis.file.FileRange.RangeType;
import iotrace.model.analysis.file.*;
import iotrace.model.analysis.trace.id.FileTraceId;
import iotrace.model.analysis.trace.id.FileTraceMemoryId;
import iotrace.model.analysis.trace.traceables.BasicTrace;
import iotrace.model.analysis.trace.traceables.FileTrace;
import iotrace.model.analysis.trace.traceables.ThreadTrace;
import iotrace.model.logs.Json;
import iotrace.model.analysis.trace.*;
import iotrace.model.analysis.trace.traceables.FileTrace.FileType;
import iotrace.model.analysis.trace.id.FileTraceId.IdGroup;
import iotrace.model.analysis.trace.id.FileTraceId.IdType;
import iotrace.model.analysis.FunctionEvent.FunctionType;
import iotrace.model.logs.UniqueStartTime;

import java.util.TreeMap;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class Evaluation {

	private static final Logger logger = LogManager.getLogger(Evaluation.class);

	public static final String JSON_GET_START_TIME = "getStartTime";

	/**
	 * Set of all methods needed to build {@link FileTrace}'s and
	 * {@link ThreadTrace}'s. Each method is available over the function name from a
	 * {@link Json}. The methods extract json elements from the current {@link Json}
	 * and build the objects {@link FileTrace}, {@link ThreadTrace},
	 * {@link FunctionEvent} (to connect {@link FileTrace} and {@link ThreadTrace})
	 * and {@link FileOffset}.
	 */
	private AnalyzeFunctionPool fileTraceFunctions;
	/**
	 * Set of all methods needed to build {@link FileRange}'s. Each method is
	 * available over the function name from a {@link Json}. The methods add
	 * {@link FileRange} objects based on {@link FileOffset} to each
	 * {@link FunctionEvent} if a {@link FileOffset} is provided.
	 */
	private AnalyzeFunctionPool fileRangeFunctions;
	/**
	 * Set of all methods needed to build {@link FileLock}'s. Each method is
	 * available over the function name from a {@link Json}. The methods add
	 * {@link FileLock} objects based on {@code lockMode} from {@link FileTraceId}
	 * to each {@link FunctionEvent}.
	 */
	private AnalyzeFunctionPool fileLockFunctions;
	/**
	 * Set of all methods needed to set the working directory. Each method is
	 * available over the function name from a {@link Json}.
	 */
	private AnalyzeFunctionPool workingDirFunctions;

	private int countTmpFiles = 0;
	private int countNotAFile = 0;
	private int countUnnamedSockets = 0;
	/**
	 * All read in {@link Json}-objects representing a function call sorted by start
	 * time of the function call. TreeMap for efficient sorting.
	 */
	private Map<UniqueStartTime, Json> jsons = new TreeMap<>();
	/**
	 * All read in {@link Json}-objects representing a working dir sorted by host,
	 * process and time.
	 */
	private Map<String, Map<String, TreeMap<UniqueStartTime, Json>>> workingDirs;
	/**
	 * Grouping traces by cluster, host, process, thread, function and file.
	 * Cluster, host and process as {@link BasicTrace}. Thread, function and file in
	 * {@link ThreadTrace}.
	 */
	private BasicTrace<BasicTrace<BasicTrace<ThreadTrace>>> cluster;
	/**
	 * Grouping traces by files, file, host, process, thread, functions and
	 * function. Files as {@link BasicTrace}. File, host, process, thread, functions
	 * and function in {@link FileTrace}.
	 */
	private BasicTrace<FileTrace> fileTraces;
	/**
	 * Contains process specific information as {@link ProcessTrace}-object. First
	 * map uses name of the host as key. Second map uses processId as key. With
	 * {@link HashMap} as data structure fast lookup for a specific process in a
	 * specific host is possible.
	 */
	private Map<String, HashMap<String, ProcessTrace>> processTraces = new HashMap<>();
	private int noFileTrace = 0;
	private int ignored = 0;
	private int unknown = 0;

	/**
	 * Current {@link ProcessTrace}. Contains the {@link ProcessTrace} for the
	 * current evaluated {@link Json}.
	 */
	private ProcessTrace tmpProcessTrace;
	/**
	 * Current {@link ThreadTrace}. Contains the {@link ThreadTrace} for the current
	 * evaluated {@link Json}.
	 */
	private ThreadTrace tmpThreadTrace;
	/**
	 * Current evaluated {@link Json}.
	 */
	private Json tmpJson;
	/**
	 * Is true if current evaluated {@link Json} has an error state for the wrapped
	 * function call.
	 */
	private boolean tmpError;
	/**
	 * Current host name. Contains the host name for the current evaluated
	 * {@link Json}.
	 */
	private String tmpHostName = "";
	/**
	 * Current processId. Contains the processId for the current evaluated
	 * {@link Json}.
	 */
	private String tmpProcessId = "";
	/**
	 * Current threadId. Contains the threadId for the current evaluated
	 * {@link Json}.
	 */
	private String tmpThreadId = "";
	/**
	 * Function name from the wrapped function call of the current evaluated
	 * {@link Json}.
	 */
	private String tmpFunctionName = "";
	/**
	 * Start time from the wrapped function call of the current evaluated
	 * {@link Json}.
	 */
	private String tmpTimeStart = "";
	/**
	 * End time from the wrapped function call of the current evaluated
	 * {@link Json}.
	 */
	private String tmpTimeEnd = "";

	private boolean hasWrapperInfo;

	private String wrapperTimeStart;
	private String wrapperTimeEnd;

	private Long minTime;
	private Long maxTime;

	private KeyValueTreeNode fork;
	private HashMap<String, HashMap<String, KeyValueTreeNode>> forkedProcesses = new HashMap<>();



	private ResourceBundle legends;



	private String directoryDelimiter = "/";


	/**
	 * Creates a data structure of connected traces for file-IO.
	 * 
	 * @param fileTracePropsPath path and file name for properties to build traces
	 * @param fileRangePropsPath path and file name for properties to build byte
	 *                            ranges
	 * @param fileLockPropsPath  path and file name for properties to build locks
	 * @param fileName            path and file name for input file. This file
	 *                            contains json-objects in separate lines.
	 * @throws IOException
	 * @throws ClassNotFoundException
	 * @throws NoSuchMethodException
	 * @throws SecurityException
	 * @throws IllegalAccessException
	 * @throws IllegalArgumentException
	 * @throws InvocationTargetException
	 */
	public Evaluation(
				String fileTracePropsPath, String fileRangePropsPath, String fileLockPropsPath, String workingDirPropsPath,
				ResourceBundle legends) throws ReflectiveOperationException, IOException,
			SecurityException, IllegalArgumentException {
		super();

		// - Create function-'pools'
		this.fileTraceFunctions = new AnalyzeFunctionPool(fileTracePropsPath);
		this.fileTraceFunctions.addObject(this);

		this.fileRangeFunctions = new AnalyzeFunctionPool(fileRangePropsPath);
		this.fileLockFunctions = new AnalyzeFunctionPool(fileLockPropsPath);
		this.workingDirFunctions = new AnalyzeFunctionPool(workingDirPropsPath);


		this.cluster = new BasicTrace<>(legends.getString("threadTraceClusterTitle"));
		this.fileTraces = new BasicTrace<>(legends.getString("fileTraceFileTitle"));

		this.fork = new KeyValueTreeNode(0, legends.getString("forkTraceMainNode"), legends.getString("forkTraceOther"));



		this.legends = legends;
	}



	/**
	 * Creates a {@link Json} from each line in {@code file} and puts it in an
	 * intern buffer for further processing. The intern buffer is ordered by start
	 * time of the wrapped function call.
	 * 
	 * @param file file that holds a JSON-Object representing a function call in
	 *             each line
	 */
	public void addJsons(File file) {
		jsons = Json.jsonLogToObjectMap(file, fileTraceFunctions);
	}

	/**
	 * Creates a {@link Json} from each line in {@code file} and puts it in an
	 * intern buffer for further processing. The intern buffer is ordered by host
	 * name, process id and time.
	 * 
	 * @param file file that holds a JSON-Object representing a working dir in each
	 *             line
	 */
	public void addWorkingDirs(File file) {
		workingDirs = Json.workingDirLogFileToJsonObjectMap(file, workingDirFunctions);
	}

	/**
	 * Processes all {@link Json}-objects from intern buffer. This means processing
	 * of all with method {@link #addJsons(File)}
	 * added objects.
	 * 
	 * After processing of each {@link Json} for all created {@link ThreadTrace} and
	 * {@link FileTrace} objects the method {@code processEvents} is called.
	 */
	public void processJsons() {
		int jsonCountOnePercent = jsons.size() / 100;
		int jsonCountTmp = 0;
		int jsonCount = 0;

		logger.debug("Start processing:");
		for (Entry<UniqueStartTime, Json> currentJson : jsons.entrySet()) {
			processJson(currentJson.getValue());

			if (minTime == null) {
				minTime = (hasWrapperInfo) ? (Long.parseLong(wrapperTimeStart)) : (Long.parseLong(tmpTimeStart));
			}

			jsonCountTmp++;
			if (jsonCountTmp > jsonCountOnePercent) {
				jsonCount += jsonCountTmp;
				jsonCountTmp = 0;
				logger.debug("    {} Function-Calls processed", jsonCount);
			}
		}

		maxTime = (hasWrapperInfo) ? (Long.parseLong(wrapperTimeEnd)) : (Long.parseLong(tmpTimeEnd));
		String.valueOf(maxTime - minTime).length();

		logger.debug("process Events ...");

		for (BasicTrace<BasicTrace<ThreadTrace>> host : cluster.getTraces().values()) {
			for (BasicTrace<ThreadTrace> process : host.getTraces().values()) {
				for (ThreadTrace thread : process.getTraces().values()) {
					thread.processEvents(fileRangeFunctions, fileLockFunctions);
					logger.trace("    {}", thread);
				}
			}
		}
		// processEvents() of FileTrace has to be done after processEvents() of
		// ThreadTrace because the FileRange is defined in processEvents() of
		// ThreadTrace and used in processEvents() of FileTrace
		for (FileTrace file : fileTraces.getTraces().values()) {
			file.processEvents();
			logger.trace("    {}", file);
		}
	}


	/**
	 * Processes the {@link Json} given in parameter {@code json}. Sets the global
	 * fields tmpProcessTrace, tmpThreadTrace, tmpJson, tmpError, tmpHostName,
	 * tmpProcessId, tmpFunctionName, tmpTimeStart and tmpTimeEnd for further calls
	 * to other methods.
	 * 
	 * Adds a new entry in {@link BasicTrace} {@code cluster}, if no fitting entry
	 * is found.
	 * 
	 * Invokes a method from {@code fileTraceFunctions} to generate the necessary
	 * traces.
	 * 
	 * @param json
	 */
	public void processJson(Json json) {
		tmpJson = json;

		fileTraceFunctions.addObject(tmpJson);

		try {
			String functionName = (String) fileTraceFunctions.invoke("getFunctionName");
			String hostName = (String) fileTraceFunctions.invoke("getHostName");
			String processId = (String) fileTraceFunctions.invoke("getProcessId");
			String threadId = (String) fileTraceFunctions.invoke("getThreadId");
			String timeStart = (String) fileTraceFunctions.invoke(JSON_GET_START_TIME);
			String timeEnd = (String) fileTraceFunctions.invoke("getEndTime");
			tmpError = (Boolean) fileTraceFunctions.invoke("getError");
			hasWrapperInfo = (Boolean) fileTraceFunctions.invoke("hasWrapperInfo");
			if (hasWrapperInfo) {
				wrapperTimeStart = (String) fileTraceFunctions.invoke("getWrapperStartTime");
				wrapperTimeEnd = (String) fileTraceFunctions.invoke("getWrapperEndTime");
			}

			if (!tmpTimeStart.equals("0") && tmpTimeStart.equals(timeStart) && tmpTimeEnd.equals(timeEnd)
					&& tmpFunctionName.equals(functionName) && tmpThreadId.equals(threadId)
					&& tmpProcessId.equals(processId) && tmpHostName.equals(hostName)) {
				// if log contains duplicate function calls: ignore them
				return;
			}

			tmpHostName = hostName;
			tmpProcessId = processId;
			tmpThreadId = threadId;
			tmpTimeStart = timeStart;
			tmpTimeEnd = timeEnd;
			tmpFunctionName = functionName;

			BasicTrace<BasicTrace<ThreadTrace>> host;
			if (!cluster.containsTrace(tmpHostName)) {
				host = new BasicTrace<>(legends.getString("threadTraceHostTitle"));
				cluster.addTrace(tmpHostName, host);
			} else {
				host = cluster.getTrace(tmpHostName);
			}

			BasicTrace<ThreadTrace> process;
			if (!host.containsTrace(tmpProcessId)) {
				process = new BasicTrace<>(legends.getString("threadTraceProcessTitle"));
				host.addTrace(tmpProcessId, process);
			} else {
				process = host.getTrace(tmpProcessId);
			}

			if (!process.containsTrace(tmpThreadId)) {
				tmpThreadTrace = new ThreadTrace(tmpHostName, tmpProcessId, tmpThreadId, legends);
				process.addTrace(tmpThreadId, tmpThreadTrace);
			} else {
				tmpThreadTrace = process.getTrace(threadId);
			}

			tmpProcessTrace = getProcessTrace(tmpHostName, tmpProcessId);

			if (fileTraceFunctions.containsFunctionName(tmpFunctionName)) {
				fileTraceFunctions.invoke(tmpFunctionName);
			} else {
				// TODO: default/Warning
				unknown++;
				logger.debug("unknown function-call #{} in json {}", unknown, tmpJson);
			}

		} catch (ReflectiveOperationException | IllegalArgumentException e) {
			logger.error("Exception during invokation of method for creating file trace for function " + tmpFunctionName
					+ " for json " + tmpJson, e);
			return;
		}
	}

	/**
	 * Returns a {@link ProcessTrace} for the parameters {@code hostName} and
	 * {@code processId}. If no existing {@link ProcessTrace} is found, a new one is
	 * created.
	 * 
	 * @param hostName  name of the host of the {@link ProcessTrace}
	 * @param processId id of the process of the {@link ProcessTrace}
	 * @return an existing or a new {@link ProcessTrace}
	 */
	private ProcessTrace getProcessTrace(String hostName, String processId) {
		return getOrSetProcessTrace(hostName, processId, null);
	}

	private FileName getFileName() {
		TreeMap<UniqueStartTime, Json> dirs = workingDirs.get(tmpHostName).get(tmpProcessId);
		UniqueStartTime currentTime = new UniqueStartTime(Long.parseLong(tmpTimeStart));

		Entry<UniqueStartTime, Json> entry = dirs.floorEntry(currentTime);

		Json json = (entry != null) ? (entry.getValue()) : (dirs.firstEntry().getValue());
		workingDirFunctions.addObject(json);

		try {
			String dir = (String)workingDirFunctions.invoke("getDir");

			// TODO: delimiter and root for windows etc.
			return new FileName(dir, directoryDelimiter, "");

		} catch (ReflectiveOperationException | IllegalArgumentException e) {
			logger.error("Exception during invokation of method for creating working dir", e);
			return null;
		}
	}

	/**
	 * Returns a {@link ProcessTrace} for the parameters {@code hostName} and
	 * {@code processId}. If no existing {@link ProcessTrace} is found, a new one is
	 * created. Additional in the parameter {@code processTrace} a new
	 * {@link ProcessTrace} can be provided. If thats the case no existing trace is
	 * searched. Instead the provided trace is internally cached and returned.
	 * 
	 * @param hostName     name of the host of the {@link ProcessTrace}
	 * @param processId    id of the process of the {@link ProcessTrace}
	 * @param processTrace a new {@link ProcessTrace} or null
	 * @return an existing or a new {@link ProcessTrace}
	 */
	private ProcessTrace getOrSetProcessTrace(String hostName, String processId, ProcessTrace processTrace) {
		HashMap<String, ProcessTrace> processes;

		if (!processTraces.containsKey(hostName)) {
			processes = new HashMap<>();
			processTraces.put(hostName, processes);
		} else {
			processes = processTraces.get(hostName);
		}

		if (processTrace == null) {
			if (!processes.containsKey(processId)) {
				processTrace = new ProcessTrace(getFileName());
				processes.put(processId, processTrace);
			} else {
				processTrace = processes.get(processId);
			}
		} else {
			processes.put(processId, processTrace);
		}

		return processTrace;
	}

	/**
	 * Creates a new {@link ProcessTrace} with the id's (file descriptors and memory
	 * mappings) of an existing one. This is needed for wrapped function calls of
	 * the fork function.
	 * 
	 * @param processIdOld id of the existing {@link ProcessTrace}
	 * @param processIdNew id of the new {@link ProcessTrace}
	 */
	public void openProcess(String processIdOld, String processIdNew) {
		ProcessTrace processTraceOld = getProcessTrace(tmpHostName, processIdOld);
		ProcessTrace processTraceNew = processTraceOld.cloneWithIds(getFileName());

		getOrSetProcessTrace(tmpHostName, processIdNew, processTraceNew);

		KeyValueTreeNode hostNode;
		HashMap<String, KeyValueTreeNode> host;
		if (!fork.hasRealChildWithKey(tmpHostName)) {
			hostNode = new KeyValueTreeNode(0, tmpHostName, legends.getString("forkTraceOther"));
			fork.addChild(hostNode);
			host = new HashMap<>();
			forkedProcesses.put(tmpHostName, host);
		} else {
			hostNode = fork.getRealChild(tmpHostName);
			host = forkedProcesses.get(tmpHostName);
		}

		KeyValueTreeNode oldProcess;
		if (!host.containsKey(processIdOld)) {
			oldProcess = new KeyValueTreeNode(0, processIdOld, legends.getString("forkTraceOther"));
			host.put(processIdOld, oldProcess);
			hostNode.addChild(oldProcess);
		} else {
			oldProcess = host.get(processIdOld);
		}
		KeyValueTreeNode newProcess = new KeyValueTreeNode(0, processIdNew, legends.getString("forkTraceOther"));
		host.put(processIdNew, newProcess);
		oldProcess.addChild(newProcess);
	}

	/**
	 * Creates a new {@link FileTrace} if no existing trace for the parameter
	 * {@code fileId} could be found. The new created or existing {@link FileTrace}
	 * is cached in the current {@link ProcessTrace} with the parameter {@code id}
	 * as key. For the cache entry in the {@link ProcessTrace} a new
	 * {@link FileOffset} is created and cached. The new offset is initially set to
	 * {@code 0}.
	 * 
	 * The {@link FileTrace} is registered in the current ProcessTrace as an
	 * currently open file if the current error is not set to {@code true}.
	 * 
	 * New events are added to the {@link ThreadTrace} and {@link FileTrace}. This
	 * is done via method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTrace fileTrace, FileOffset fileOffset)}.
	 * 
	 * @param fileId   id (hostName, deviceId and fileId) of the file
	 * @param idType   type of the id of the {@link FileTrace} (necessary for
	 *                 searching or creating the {@link FileTrace})
	 * @param id       id for caching the {@link FileTrace}
	 * @param tmpFile  if set to {@code true} a unique file name is created and used
	 *                 instead of the parameter {@code fileName}
	 * @param fileName name of the file for the {@link fileTrace}
	 */
	public void openTrace(FileId fileId, IdType idType, String id, boolean tmpFile, String fileName) {
		openTrace(fileId, idType, id, tmpFile, fileName, false, null);
	}

	/**
	 * Creates a new {@link FileTrace} if no existing trace for the parameter
	 * {@code fileId} could be found. The new created or existing {@link FileTrace}
	 * is cached in the current {@link ProcessTrace} with the parameter {@code id}
	 * as key. For the cache entry in the {@link ProcessTrace} a new
	 * {@link FileOffset} is created and cached. The new offset is initially set to
	 * {@code 0}.
	 * 
	 * The {@link FileTrace} is registered in the current ProcessTrace as an
	 * currently open file if the current error is not set to {@code true}.
	 * 
	 * New events are added to the {@link ThreadTrace} and {@link FileTrace}. This
	 * is done via method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTrace fileTrace, FileOffset fileOffset)}.
	 * 
	 * @param fileId       id (hostName, deviceId and fileId) of the file
	 * @param idType       type of the id of the {@link FileTrace} (necessary for
	 *                     searching or creating the {@link FileTrace})
	 * @param id           id for caching the {@link FileTrace}
	 * @param tmpFile      if set to {@code true} a unique file name is created and
	 *                     used instead of the parameter {@code fileName}
	 * @param fileName     name of the file for the {@link fileTrace}
	 * @param relativeToId if set to {@code true} the parameter {@link workingDirId}
	 *                     is used to expand the parameter {@link fileName} to an
	 *                     absolute path
	 * @param workingDirId if parameter {@code relativeToId} is set to {@code true},
	 *                     the parameter {@code workingDirId} must be set to an id
	 *                     of an existing {@link FileTrace}. The directory of this
	 *                     {@link FileTrace} is used to expand the parameter
	 *                     {@code fileName} to an absolute path.
	 */
	public void openTrace(FileId fileId, IdType idType, String id, boolean tmpFile, String fileName,
			boolean relativeToId, String workingDirId) {
		FileOffset fileOffset = new FileOffset(0);
		FileTrace tmpFileTrace = getFileTrace(fileId, idType, tmpFile, fileName, relativeToId, workingDirId);

		FileTraceId fileTraceId = tmpProcessTrace.setFileTraceId(idType, id, tmpFileTrace, fileOffset);

		addEvent(fileTraceId);

		if (tmpError) {
			closeFileTrace(tmpFileTrace);
		}
	}

	/**
	 * Creates a new {@link FileTrace} via method {@link FileTrace
	 * iotrace.analyze.Data.openTrace()}. The trace gets an unique file name and
	 * isn't registered as an currently open file in the current
	 * {@link ProcessTrace}. New events for the current {@link Json} are added to
	 * the {@link ThreadTrace} and {@link FileTrace}. The new trace is cached in the
	 * current {@link ProcessTrace} with the parameter {@code id1} as key and the
	 * parameter {@code id2} as key. The {@link FileOffset} for each cache entry is
	 * set to {@code null}.
	 * 
	 * New events are added to the {@link ThreadTrace} and {@link FileTrace}. This
	 * is done via method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTraceId fileTraceId, FileTraceId
	 * fileTraceId2)}.
	 * 
	 * @param kind    FileKind of the new {@link FileTrace}
	 * @param idType1 type of the id1 of the {@link FileTrace}
	 * @param id1     id for caching the {@link FileTrace}
	 * @param idType2 type of the id2 of the {@link FileTrace}
	 * @param id2     id for caching the {@link FileTrace}
	 */
	public void openTrace(FileType kind, IdType idType1, String id1, IdType idType2, String id2) {
		switch (kind) {
			case SOCKET:
				countUnnamedSockets++;
				openTrace(kind, new FileId(new FileIdSocket(tmpHostName, FileIdSocket.FAMILY_UNNAMED_SOCKET,
					Integer.toString(countUnnamedSockets))), idType1, id1, idType2, id2);
				break;
			default:
				openTrace(kind, new FileId(new FileIdRegular(tmpHostName, -1, -1)), idType1, id1, idType2, id2);
		}
	}

	/**
	 * Creates a new {@link FileTrace} via method {@link FileTrace
	 * iotrace.analyze.Data.openTrace()}. The trace isn't registered as an currently
	 * open file in the current {@link ProcessTrace}. New events for the current
	 * {@link Json} are added to the {@link ThreadTrace} and {@link FileTrace}. The
	 * new trace is cached in the current {@link ProcessTrace} with the parameter
	 * {@code id1} as key and the parameter {@code id2} as key. The
	 * {@link FileOffset} for each cache entry is set to {@code null}.
	 * 
	 * New events are added to the {@link ThreadTrace} and {@link FileTrace}. This
	 * is done via method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTraceId fileTraceId, FileTraceId
	 * fileTraceId2)}.
	 * 
	 * @param kind    FileKind of the new {@link FileTrace}
	 * @param fileId  id (hostName, deviceId and fileId) of the file
	 * @param idType1 type of the id1 of the {@link FileTrace}
	 * @param id1     id for caching the {@link FileTrace}
	 * @param idType2 type of the id2 of the {@link FileTrace}
	 * @param id2     id for caching the {@link FileTrace}
	 */
	public void openTrace(FileType kind, FileId fileId, IdType idType1, String id1, IdType idType2, String id2) {
		FileTrace tmpFileTrace = openTrace(kind, fileId);

		FileTraceId fileTraceId = tmpProcessTrace.setFileTraceId(idType1, id1, tmpFileTrace, null);
		FileTraceId fileTraceId2 = tmpProcessTrace.setFileTraceId(idType2, id2, tmpFileTrace, null);

		addEvent(fileTraceId, fileTraceId2);
	}

	/**
	 * Creates a new {@link FileTrace} with an unique file name. The trace isn't
	 * registered as an currently open file in the current {@link ProcessTrace}. New
	 * events for the current {@link Json} are added to the {@link ThreadTrace} and
	 * {@link FileTrace}. The new trace is cached in the current
	 * {@link ProcessTrace} with the parameter {@code id} as key. The
	 * {@link FileOffset} for each cache entry is set to {@code null}.
	 * 
	 * New events are added to the {@link ThreadTrace} and {@link FileTrace}. This
	 * is done via method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTraceId fileTraceId)}.
	 * 
	 * @param kind   FileKind of the new {@link FileTrace}
	 * @param fileId id (hostName, sockaddrFamily and sockaddrData) of the file
	 * @param idType type of the id1 of the {@link FileTrace}
	 * @param id     id for caching the {@link FileTrace}
	 */
	public void openTrace(FileType kind, FileId fileId, IdType idType, String id) {
		String fileTraceIdStr = fileId.toString();
		FileTrace tmpFileTrace;

		if (!fileTraces.containsTrace(fileTraceIdStr)) {
			tmpFileTrace = new FileTrace(fileId, getUniqueFileName(), kind, directoryDelimiter, legends);
			fileTraces.addTrace(fileTraceIdStr, tmpFileTrace);
		} else {
			tmpFileTrace = fileTraces.getTrace(fileTraceIdStr);
		}

		FileTraceId fileTraceId = tmpProcessTrace.setFileTraceId(idType, id, tmpFileTrace, null);

		addEvent(fileTraceId);
	}

	/**
	 * Creates a new {@link FileTrace} via method {@link FileTrace
	 * iotrace.analyze.Data.openTrace()}. The trace gets an unique file name and
	 * isn't registered as an currently open file in the current
	 * {@link ProcessTrace}. New events for the current {@link Json} are added to
	 * the {@link ThreadTrace} and {@link FileTrace}. The new trace is cached in the
	 * current {@link ProcessTrace} with the parameter {@code id} as key. The
	 * {@link FileOffset} for the cache entry is set to {@code null}.
	 * 
	 * New events are added to the {@link ThreadTrace} and {@link FileTrace}. This
	 * is done via method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTraceId fileTraceId)}.
	 * 
	 * @param kind   FileKind of the new {@link FileTrace}
	 * @param idType type of the id of the {@link FileTrace}
	 * @param id     id for caching the {@link FileTrace}
	 */
	public void openTrace(FileType kind, IdType idType, String id) {
		FileTrace tmpFileTrace;

		switch (kind) {
			case SOCKET:
				tmpFileTrace = openTrace(kind,
					new FileId(new FileIdSocket(tmpHostName, FileIdSocket.FAMILY_UNBOUND_SOCKET, "")));
				break;
			default:
				tmpFileTrace = openTrace(kind, new FileId(new FileIdRegular(tmpHostName, -1, -1)));
		}

		FileTraceId fileTraceId = tmpProcessTrace.setFileTraceId(idType, id, tmpFileTrace, null);

		addEvent(fileTraceId);
	}

	/**
	 * Creates a new {@link FileTrace} via method {@link FileTrace
	 * iotrace.analyze.Data.openTrace()}. The trace gets an unique file name and
	 * isn't registered as an currently open file in the current
	 * {@link ProcessTrace}. New events for the current {@link Json} are added to
	 * the {@link ThreadTrace} and {@link FileTrace}. The new trace is cached in the
	 * current {@link ProcessTrace} with the parameter {@code address} and the
	 * parameter {@code length} as key. The {@link FileOffset} for the cache entry
	 * is set to {@code null}.
	 * 
	 * New events are added to the {@link ThreadTrace} and {@link FileTrace}. This
	 * is done via method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTraceId fileTraceId)}.
	 * 
	 * @param address start address of memory mapped file for caching the
	 *                {@link FileTrace}
	 * @param length  length of memory mapped file for caching the {@link FileTrace}
	 * @param shared  {@code true} if memory is shared
	 */
	private void openTrace(String address, String length, boolean shared) {
		FileTrace tmpFileTrace = openTrace(FileType.MEMORY, new FileId(new FileIdRegular(tmpHostName, -1, -1)));

		FileTraceId fileTraceId = tmpProcessTrace.setFileTraceMemoryId(address, length, tmpFileTrace, null, shared);

		addEvent(fileTraceId);
	}

	/**
	 * Creates a new {@link FileTrace} with an unique file name. The
	 * {@link FileTrace} is not registered in the current {@link ProcessTrace} as an
	 * currently open file. This method should only be called for wrapped functions
	 * which open non standard files (e.g. sockets or pipes).
	 * 
	 * @param kind   FileKind of the new {@link FileTrace}
	 * @param fileId id (hostName, deviceId and fileId) of the file
	 * @return new created {@link FileTrace}
	 */
	private FileTrace openTrace(FileType kind, FileId fileId) {
		FileTrace tmpFileTrace = openFileTrace(fileId, kind, getUniqueFileName());

		closeFileTrace(tmpFileTrace);

		return tmpFileTrace;
	}

	private String getUniqueFileName() {
		countNotAFile++;
		return "not a file no. " + countNotAFile;
	}

	/**
	 * Returns an existing or a new created {@link FileTrace}.
	 * 
	 * To search for an existing {@link FileTrace} the parameter {@code fileName} is
	 * used. For that the {@code fileName} is expanded to an absolute path if the
	 * parameter {@code relativeToId} is set to {@code true}. For expanding to an
	 * path the parameter {@code workingDirId} must be set to an id of an existing
	 * {@link FileTrace}. The directory of the file name of this existing
	 * {@link FileTrace} is used to expand the path.
	 * 
	 * If the parameter {@code tmpFile} is set to {@code true}, an alternative
	 * unique file name is generated.
	 * 
	 * If parameter {@code tmpFile} and parameter {@code relativeToId} are set to
	 * {@code false} only the name of the current host is added to the file name.
	 * 
	 * In each case the existing or new created {@link FileTrace} is added to the
	 * current open files of the current {@link ProcessTrace}.
	 * 
	 * @param fileId       id (hostName, deviceId and fileId) of the file
	 * @param idType       type of the id of the {@link FileTrace} (necessary for
	 *                     searching or creating the {@link FileTrace})
	 * @param tmpFile      if set to {@code true} a unique file name is created and
	 *                     used instead of the parameter {@code fileName}
	 * @param fileName     name of the file for the {@link fileTrace}
	 * @param relativeToId if set to {@code true} the parameter {@code workingDirId}
	 *                     is used to expand the parameter {@code fileName} to an
	 *                     absolute path
	 * @param workingDirId if parameter {@code relativeToId} is set to {@code true},
	 *                     the parameter {@code workingDirId} must be set to an id
	 *                     of an existing {@link FileTrace}. The directory of this
	 *                     {@link FileTrace} is used to expand the parameter
	 *                     {@code fileName} to an absolute path.
	 * @return an existing or a new {@link FileTrace}
	 */
	private FileTrace getFileTrace(FileId fileId, IdType idType, boolean tmpFile, String fileName, boolean relativeToId,
			String workingDirId) {
		FileType kind = FileType.FILE;
		FileName file = tmpProcessTrace.getFileName();

		if (tmpFile) {
			countTmpFiles++;
			fileName = "unnamed temporary regular file no. " + countTmpFiles;
			kind = FileType.TMPFILE;
		} else if (relativeToId) {
			FileTrace fileTrace = getFileTrace(idType, workingDirId);
			if (fileTrace != null) {
				String dir = "";
				for (String s : fileTrace.getFileNames()) {
					dir = s;
					break;
				}
				dir = file.getWorkingDirFromFileName(dir);
				FileName tmpFileName = new FileName(dir, file.getDirectoryDelimiter(), file.getRoot());
				fileName = tmpFileName.getFileName(fileName, tmpHostName);
			}
		} else {
			fileName = file.getFileName(fileName, tmpHostName);
		}

		return openFileTrace(fileId, kind, fileName);
	}

	private FileTrace getWrapperFileTrace() {
		FileId fileId = new FileId(new FileIdWrapper());
		String id = fileId.toString();
		String fileName = fileId.toFileName();
		FileTrace file;

		if (!fileTraces.containsTrace(id)) {
			file = new FileTrace(fileId, fileName, FileType.WRAPPER, directoryDelimiter, legends);
			fileTraces.addTrace(id, file);
		} else {
			file = fileTraces.getTrace(id);
			file.addFileName(fileName);
		}

		return file;
	}

	/**
	 * Returns an existing or a new created {@link FileTrace}. If an existing
	 * {@link FileTrace} is found, it is returned. If no existing {@link FileTrace}
	 * is found a new one is created, internally cached and returned.
	 * 
	 * In each case the {@link FileTrace} is registered in the current
	 * {@link ProcessTrace} as an currently open file. This is for further methods
	 * which manipulate all open files.
	 * 
	 * For searching and caching the parameter {@code fileId} is used.
	 * 
	 * @param fileId   id (hostName, deviceId and fileId) of the file
	 * @param kind     type of the file for the {@link FileTrace}
	 * @param fileName name of the file for the {@link FileTrace}
	 * @return an existing or a new {@link FileTrace}
	 */
	private FileTrace openFileTrace(FileId fileId, FileType kind, String fileName) {
		String id = fileId.toString();
		FileTrace file;

		if (!fileTraces.containsTrace(id)) {
			file = new FileTrace(fileId, fileName, kind, directoryDelimiter, legends);
			fileTraces.addTrace(id, file);
		} else {
			file = fileTraces.getTrace(id);
			file.addFileName(fileName);
		}

		tmpProcessTrace.addOpenFile(file);

		return file;
	}

	/**
	 * Creates events for the current {@link Json} in the {@link FileTrace} given
	 * through the parameter {@code id} and removes the {@link FileTrace} from the
	 * list of open files of the current {@link ProcessTrace}. The events are also
	 * added to the current {@link ThreadTrace}.
	 * 
	 * @param idType type of the id to search for the {@link FileTrace}
	 * @param id     id to search for the {@link FileTrace}
	 */
	public void closeTrace(IdType idType, String id) {
		FileTraceId fileTraceId = tmpProcessTrace.getFileTraceId(idType, id);

		addEvent(fileTraceId);

		if (fileTraceId != null && fileTraceId.getFileTrace() != null) {
			closeFileTrace(fileTraceId.getFileTrace());
		}
	}

	/**
	 * Creates events for the current {@link Json} in all {@link FileTrace}'s given
	 * through the parameter {@code idGroup} and removes the {@link FileTrace}'s
	 * from the list of open files of the current {@link ProcessTrace}. The events
	 * are also added to the current {@link ThreadTrace}. The events are connected
	 * to each other (field {@code theSame} in {@link FunctionEvent}; can be
	 * returned with method {@code getSame()}).
	 * 
	 * The only implemented version is for parameter {@code idGroup} equals
	 * {@code PROCESS}. In this case all open files of the current
	 * {@link ProcessTrace} are searched. Each {@link FileTrace} of an open file is
	 * processed. If no {@link FileTrace} is found only events for the
	 * {@link ThreadTrace} are added.
	 * 
	 * If the parameter {@code idGroup} is not set to {@code PROCESS} a warning
	 * message is generated.
	 * 
	 * @param idType  type of the id to search for the {@link FileTrace}
	 * @param idGroup identifies a group of {@link FileTrace}'s. Only the value
	 *                {@code PROCESS} is implemented.
	 */
	public void closeTrace(IdType idType, IdGroup idGroup) {
		Set<FileTraceId> fileTraceIds = tmpProcessTrace.getOpenFileTracesForIdType(idType);

		if (idGroup.equals(IdGroup.PROCESS) && fileTraceIds != null) {
			if (fileTraceIds.isEmpty()) {
				addEvent(new FileTraceId(idType, null, null));
			} else {
				for (FileTraceId id : fileTraceIds) {
					FileTrace fileTrace = id.getFileTrace();
					closeFileTrace(fileTrace);
				}
				addEvent(fileTraceIds);
			}
		} else {
			ignore();
		}
	}

	/**
	 * TODO
	 * 
	 * @param fileId   id (hostName, deviceId and fileId) of the file
	 * @param idType
	 * @param id
	 * @param tmpFile
	 * @param fileName
	 */
	public void closeOldOpenNewTrace(FileId fileId, IdType idType, String id, boolean tmpFile, String fileName) {
		Set<FileTraceId> fileTraceIds = new HashSet<>();
		FileTraceId oldFileTraceId = tmpProcessTrace.getFileTraceId(idType, id);
		FileTraceId newFileTraceId;

		if (fileName.isEmpty()) {
			newFileTraceId = oldFileTraceId;
		} else {
			FileTrace newFileTrace = getFileTrace(fileId, idType, tmpFile, fileName, false, null);
			FileOffset fileOffset = new FileOffset(0);
			newFileTraceId = tmpProcessTrace.setFileTraceId(idType, id, newFileTrace, fileOffset);
		}

		if (oldFileTraceId != null && oldFileTraceId != newFileTraceId) {
			closeFileTrace(oldFileTraceId.getFileTrace());
			fileTraceIds.add(oldFileTraceId);
		}

		if (tmpError) {
			closeFileTrace(newFileTraceId.getFileTrace());
		}

		fileTraceIds.add(newFileTraceId);
		addEvent(fileTraceIds);
	}

	/**
	 * TODO
	 * 
	 * @param oldIdType
	 * @param oldId
	 * @param newIdType
	 * @param newId
	 */
	public void closeTraceAddId(IdType oldIdType, String oldId, IdType newIdType, String newId) {
		FileTraceId oldFileTraceId = tmpProcessTrace.getFileTraceId(oldIdType, oldId);
		FileTraceId newFileTraceId = tmpProcessTrace.getFileTraceId(newIdType, newId);

		if (newFileTraceId == null
				|| oldFileTraceId != null && oldFileTraceId.getFileTrace().equals(newFileTraceId.getFileTrace())) {
			addEvent(oldFileTraceId);
		} else if (oldFileTraceId != null) {
			Set<FileTraceId> tmpArray = new HashSet<>();
			tmpArray.add(newFileTraceId);
			tmpArray.add(oldFileTraceId);
			addEvent(tmpArray);
		} else {
			addEvent(newFileTraceId);
		}

		if (oldFileTraceId != null && newFileTraceId != null
				&& !newFileTraceId.getFileTrace().equals(oldFileTraceId.getFileTrace())) {
			closeFileTrace(newFileTraceId.getFileTrace());
		}

		if (oldFileTraceId != null) {
			tmpProcessTrace.setFileTraceId(newIdType, newId, oldFileTraceId.getFileTrace(),
					oldFileTraceId.getFileOffset());
		}
	}

	/**
	 * Removes the in parameter {@code fileTrace} given {@link FileTrace} from the
	 * open files in the current {@link ProcessTrace}.
	 * 
	 * @param fileTrace {@link FileTrace} to remove from the open files in current
	 *                  {@link ProcessTrace}
	 */
	private void closeFileTrace(FileTrace fileTrace) {
		tmpProcessTrace.removeOpenFile(fileTrace);
	}

	public void addWrapperEvent() {
		FileTraceId fileTraceId = new FileTraceId(null, getWrapperFileTrace(), null);

		addEvent(fileTraceId);
	}

	/**
	 * Creates a call and a return {@link FunctionEvent} from the current
	 * {@link Json} in the current {@link ThreadTrace}. The new events are connected
	 * to each other (field {@code belongsTo} in {@link FunctionEvent}; can be
	 * returned with method {@code getBelongsTo()}). If an {@link FileTrace} is
	 * found each event points to it and it's {@link FileOffset}. Additional, the
	 * events are registered in the {@link FileTrace}, if an {@link FileTrace} is
	 * found.
	 * 
	 * @param idType type of the id to search for the {@link FileTrace} and the
	 *               {@link FileOffset}
	 * @param id     id to search for the {@link FileTrace} and the
	 *               {@link FileOffset}
	 */
	public void addEvent(IdType idType, String id) {
		FileTraceId fileTraceId = tmpProcessTrace.getFileTraceId(idType, id);

		addEvent(fileTraceId);
	}

	/**
	 * Creates multiple events from the current {@link Json}. This is necessary if
	 * one wrapped function call manipulates more than one file. Creates a call and
	 * a return {@link FunctionEvent} for each {@link FileTrace} found for an id in
	 * parameter {@code ids}. Does nothing if no {@link FileTrace} is found (in that
	 * case a warning is generated). Each new call and return event is connected to
	 * all other new call events (field {@code theSame} in {@link FunctionEvent};
	 * can be returned with method {@code getSame()}). Events are created with
	 * method
	 * {@code void iotrace.analyze.Data.addEvent(Set<FileTraceId> fileTraceIds)}.
	 * For that the {@link FileOffset} from each existing {@link FileTraceId} is
	 * used. So every new {@link FunctionEvent} has the same offset as all
	 * {@link FunctionEvent}'s generated with the existing id. Therefore this method
	 * is only usable for wrapped function calls that share the offset of the
	 * existing id.
	 * 
	 * @param idType type of the id to search for the {@link FileTrace} and the
	 *               {@link FileOffset}
	 * @param ids    set of id's to search for the {@link FileTrace} and the
	 *               {@link FileOffset}
	 */
	public void addEvent(IdType idType, LinkedList<String> ids) {
		if (ids.isEmpty()) {
			ignore();
		} else {
			Set<FileTraceId> fileTraceIds = new HashSet<>();
			for (String id : ids) {
				fileTraceIds.add(tmpProcessTrace.getFileTraceId(idType, id));
			}
			addEvent(fileTraceIds);
		}
	}

	public void addEvent(String address, String length) {
		Set<FileTraceId> fileTraceIds = getFileTraceId(tmpProcessTrace.getFileTraceMemoryId(address, length));
		if (fileTraceIds != null && !fileTraceIds.isEmpty()) {
			addEvent(fileTraceIds);
		} else {
			ignore();
		}
	}

	public void addEvent(String address, String length, boolean dontfork, boolean dofork) {
		HashSet<FileTraceMemoryId> fileTraceMemoryIds = tmpProcessTrace.getFileTraceMemoryId(address, length);

		if (dontfork || dofork) {
			boolean copyOnFork = true;

			if (dontfork) {
				copyOnFork = false;
			}
			if (fileTraceMemoryIds != null) {
				for (FileTraceMemoryId m : fileTraceMemoryIds) {
					m.setCopyOnFork(copyOnFork);
				}
			}
		}

		Set<FileTraceId> fileTraceIds = getFileTraceId(fileTraceMemoryIds);
		if (fileTraceIds != null && !fileTraceIds.isEmpty()) {
			addEvent(fileTraceIds);
		} else {
			ignore();
		}
	}

	public void addEvent(IdGroup idGroup) {
		if (idGroup.equals(IdGroup.HOST)) {
			String from = tmpProcessTrace.getFileName().getHostName(tmpHostName);
			String to = from.substring(0, from.length() - 1)
					+ String.valueOf((char) (from.charAt(from.length() - 1) + 1));

			SortedMap<String, FileTrace> hostFileTraces = fileTraces.getTraces().subMap(from, to);

			if (hostFileTraces.isEmpty()) {
				addEvent(new FileTraceId(null, null, null));
			} else {
				Set<FileTrace> fileTraces = new HashSet<>();
				for (FileTrace e : hostFileTraces.values()) {
					fileTraces.add(e);
				}
				addEventNullOffset(fileTraces);
			}
		} else {
			ignore();
		}
	}

	public void addEvent(IdType idType, IdGroup idGroup) {
		Set<FileTraceId> fileTraceIds = tmpProcessTrace.getOpenFileTracesForIdType(idType);

		if (idGroup == IdGroup.PROCESS) {
			if (fileTraceIds == null || fileTraceIds.isEmpty()) {
				addEvent(new FileTraceId(idType, null, null));
			} else {
				addEvent(fileTraceIds);
			}
		} else {
			ignore();
		}
	}

	public void addEventForProcess(IdType idType, String id) {
		if (id.equals("(nil)")) {
			addEvent(idType, IdGroup.PROCESS);
		} else {
			addEvent(idType, id);
		}
	}

	/**
	 * Creates a call and a return {@link FunctionEvent} from the current
	 * {@link Json} in the current {@link ThreadTrace}. The new events are connected
	 * to each other (field {@code belongsTo} in {@link FunctionEvent}; can be
	 * returned with method {@code getBelongsTo()}). Each event points to the given
	 * {@link FileTrace} and {@link FileOffset} (extracted from
	 * {@param fileTraceId}. Additional, the events are registered in the
	 * {@link FileTrace}, if an {@link FileTrace} is given.
	 * 
	 * @param fileTraceId used to evaluate {@link FileTrace} (used for creating the
	 *                    new events. Can be null if no open {@link FileTrace} for
	 *                    the {@link Json} could be found. This will create a
	 *                    warning message) and {@link FileOffset} (used for creating
	 *                    the new events. Can be null if no offset can be provided.
	 *                    This will make further analysis impossible and should only
	 *                    be done for non file-traces (e.g. sockets and pipes) or
	 *                    for events which don't manipulate file content (e.g. close
	 *                    functions)).
	 * @return the created call event
	 * @see {@link FunctionEvent}
	 */
	private FunctionEvent addEvent(FileTraceId fileTraceId) {
		long wrapperStart;
		long wrapperEnd;
		if (hasWrapperInfo) {
			wrapperStart = Long.parseLong(wrapperTimeStart);
			wrapperEnd = Long.parseLong(wrapperTimeEnd);
		} else {
			wrapperStart = 0;
			wrapperEnd = 0;
		}

		FunctionEvent call = new FunctionEvent(fileTraceId, Long.parseLong(tmpTimeStart), wrapperStart, tmpFunctionName,
				tmpThreadTrace, tmpJson, FunctionType.CALL, tmpError);
		FunctionEvent ret = new FunctionEvent(fileTraceId, Long.parseLong(tmpTimeEnd), wrapperEnd, tmpFunctionName,
				tmpThreadTrace, tmpJson, FunctionType.RETURN, tmpError);

		call.setBelongsTo(ret);
		ret.setBelongsTo(call);

		tmpThreadTrace.addEvent(call);
		tmpThreadTrace.addEvent(ret);

		if (fileTraceId != null && fileTraceId.getFileTrace() != null) {
			FileTrace fileTrace = fileTraceId.getFileTrace();
			fileTrace.addEvent(call);
			fileTrace.addEvent(ret);
		} else {
			noFileTrace++;
			logger.debug("without file-trace #{} in json {}", noFileTrace, tmpJson);
		}

		return call;
	}

	private FunctionEvent addEvent(FileTraceId fileTraceId, FileTraceId fileTraceId2) {
		FunctionEvent functionEvent = addEvent(fileTraceId);
		functionEvent.setFileTraceId2(fileTraceId2);
		functionEvent.getBelongsTo().setFileTraceId2(fileTraceId2);
		return functionEvent;
	}

	/**
	 * Creates multiple events from the current {@link Json} without adding a
	 * {@link FileOffset}. This is necessary if one wrapped function call
	 * manipulates more than one file and no offset could be provided. The missing
	 * offset will make further analysis impossible. This method should therefore
	 * only be used for non file-traces (e.g. sockets and pipes). Creates a call and
	 * a return {@link FunctionEvent} for each entry in parameter
	 * {@code fileTraces}. Does nothing if {@code fileTraces} is null or empty. Each
	 * new call and return event is connected to all other new call events (field
	 * {@code theSame} in {@link FunctionEvent}; can be returned with method
	 * {@code getSame()}). Events are created with method {@link FunctionEvent
	 * iotrace.analyze.Data.addEvent(FileTrace fileTrace, FileOffset fileOffset)}
	 * 
	 * @param fileTraces a set of {@link FileTrace} objects. For each object a
	 *                   separate call and return event will be created.
	 * @see FunctionEvent iotrace.analyze.Data.addEvent(FileTrace fileTrace,
	 *      FileOffset fileOffset)
	 */
	private void addEventNullOffset(Set<FileTrace> fileTraces) {
		ArrayList<FunctionEvent> events = new ArrayList<>();

		if (fileTraces != null && !fileTraces.isEmpty()) {
			for (FileTrace f : fileTraces) {
				FunctionEvent event = addEvent(new FileTraceId(null, f, null));
				events.add(event);
			}

			addSame(events);
		}
	}

	/**
	 * Creates multiple events from the current {@link Json}. This is necessary if
	 * one wrapped function call manipulates more than one file. Creates a call and
	 * a return {@link FunctionEvent} for each entry in parameter
	 * {@code fileTraceIds}. Does nothing if {@code fileTraceIds} is null or empty.
	 * Each new call and return event is connected to all other new call events
	 * (field {@code theSame} in {@link FunctionEvent}; can be returned with method
	 * {@code getSame()}). Events are created with method
	 * {@code FunctionEvent iotrace.analyze.Data.addEvent(FileTraceId fileTraceId)}.
	 * For that the {@link FileOffset} from each existing {@link FileTraceId} is
	 * used. So every new {@link FunctionEvent} has the same offset as all
	 * {@link FunctionEvent}'s generated with the existing id. Therefore this method
	 * is only usable for wrapped function calls that share the offset of the
	 * existing id.
	 * 
	 * @param fileTraceIds a set of {@link FileTraceId} objects. For each object a
	 *                     separate call and return event will be created.
	 * @see addEvent(FileTrace fileTrace, FileOffset fileOffset)
	 */
	private void addEvent(Set<FileTraceId> fileTraceIds) {
		ArrayList<FunctionEvent> events = new ArrayList<>();

		if (fileTraceIds != null && !fileTraceIds.isEmpty()) {
			for (FileTraceId f : fileTraceIds) {
				FunctionEvent event = addEvent(f);
				events.add(event);
			}

			addSame(events);
		}
	}

	/**
	 * Connects all events from parameter {@code events} with each other (field
	 * {@code theSame} in {@link FunctionEvent}; can be returned with method
	 * {@code getSame()}). To each call and return event all other call events are
	 * added.
	 * 
	 * @param events {@link ArrayList} of {@link FunctionEvent}'s to connect with
	 *               each other
	 */
	private void addSame(ArrayList<FunctionEvent> events) {
		for (int i = 0; i < events.size(); i++) {
			@SuppressWarnings("unchecked")
			// clone() is necessary because each FunctionEvent needs it's own
			// list without itself
			ArrayList<FunctionEvent> tmpEvents = (ArrayList<FunctionEvent>) events.clone();
			tmpEvents.remove(i);

			FunctionEvent event = events.get(i);
			event.addSame(tmpEvents);
			event.getBelongsTo().addSame(tmpEvents);
		}
	}

	public void addId(IdType oldIdType, String oldId, IdType newIdType, String newId, boolean ignore) {
		if (ignore) {
			ignore();
			return;
		}

		addId(oldIdType, oldId, newIdType, newId);
	}

	public void addId(IdType oldIdType, String oldId, String address, String length, boolean anonymous, String offset,
			boolean shared) {
		if (anonymous) {
			openTrace(address, length, shared);
		} else {
			addId(oldIdType, oldId, address, length, offset, shared);
		}
	}

	/**
	 * Adds a new id to an existing {@link FileTrace}. The existing
	 * {@link FileTrace} is searched with parameters {@code oldIdType} and
	 * {@code oldId}. If no {@link FileTrace} is found, a new id without a
	 * connection to a {@link FileTrace} is added. In each case a new event for the
	 * current {@link Json} in the current {@link FileTrace} and {@link ThreadTrace}
	 * is added. The new id gets the same {@link FileOffset} as the existing one. So
	 * every new {@link FunctionEvent} for that id has the same offset as all
	 * {@link FunctionEvent}'s generated with the existing id. Therefore this method
	 * is only usable for wrapped function calls that share the offset of the
	 * existing id.
	 * 
	 * @param oldIdType type of an existing id to search for an existing
	 *                  {@link FileTrace}
	 * @param oldId     existing id to search for an existing {@link FileTrace}
	 * @param newIdType type of new id
	 * @param newId     new id
	 * @see FunctionEvent iotrace.analyze.Data.addEvent(FileTrace fileTrace,
	 *      FileOffset fileOffset)
	 */
	public void addId(IdType oldIdType, String oldId, IdType newIdType, String newId) {
		FileTraceId oldFileTraceId = tmpProcessTrace.getFileTraceId(oldIdType, oldId);
		FileTrace fileTrace;
		FileOffset fileOffset;

		if (oldFileTraceId != null) {
			fileTrace = oldFileTraceId.getFileTrace();
			fileOffset = oldFileTraceId.getFileOffset();
		} else {
			fileTrace = null;
			fileOffset = null;
		}

		FileTraceId newFileTraceId = tmpProcessTrace.setFileTraceId(newIdType, newId, fileTrace, fileOffset);
		addEvent(oldFileTraceId, newFileTraceId);
	}
	
	public void addId(IdType oldIdType, String oldId, IdType newIdType, String newId, RangeType rangeType) {
		FileTraceId oldFileTraceId = tmpProcessTrace.getFileTraceId(oldIdType, oldId);
		FileTrace fileTrace;
		FileOffset fileOffset;

		if (oldFileTraceId != null) {
			fileTrace = oldFileTraceId.getFileTrace();
			fileOffset = oldFileTraceId.getFileOffset();
		} else {
			fileTrace = null;
			fileOffset = null;
		}

		FileTraceId newFileTraceId = tmpProcessTrace.setFileTraceId(newIdType, newId, fileTrace, fileOffset, rangeType);
		addEvent(oldFileTraceId, newFileTraceId);
		
	}

	public void addId(IdType oldIdType, String oldId, String address, String length, String offset, boolean shared) {
		FileTraceId fileTraceId = tmpProcessTrace.getFileTraceId(oldIdType, oldId);
		FileTrace fileTrace;

		if (fileTraceId != null) {
			fileTrace = fileTraceId.getFileTrace();
		} else {
			fileTrace = null;
		}

		long longOffset = Long.parseLong(offset);
		long longAddress = Long.decode(address);
		FileOffset fileOffset = new FileOffset(longOffset, longAddress);

		FileTraceMemoryId fileTraceMemoryId = tmpProcessTrace.setFileTraceMemoryId(address, length, fileTrace,
				fileOffset, shared);
		addEvent(fileTraceId, fileTraceMemoryId);
	}

	public void addId(String oldAddress, String oldLength, String newAddress, String newLength) {
		FileTraceMemoryId newMemoryId = tmpProcessTrace.changeFileTraceMemoryId(oldAddress, oldLength, newAddress,
				newLength);

		if (newMemoryId != null) {
			addEvent(newMemoryId);
		} else {
			ignore();
		}
	}

	public void sendId(IdType idType, String id, IdType sendIdType, LinkedList<String> sendIds) {
		FileTraceId fileTraceId = tmpProcessTrace.getFileTraceId(idType, id);
		Set<FileTraceId> fileTraces = new HashSet<>();

		fileTraces.add(fileTraceId);

		if (fileTraceId != null) {
			for (String sendId : sendIds) {
				FileTraceId sendFileTraceId = tmpProcessTrace.getFileTraceId(sendIdType, sendId);

				if (sendFileTraceId == null) {
					noFileTrace++;
					logger.debug("without file-trace #{} in json {}", noFileTrace, tmpJson);
				} else {
					// fileTraces.add(sendFileTraceId);
					fileTraceId.getFileTrace().sendFileTraceId(sendFileTraceId);
				}
			}
		}

		addEvent(fileTraces);
	}

	public void receiveId(IdType idType, String id, IdType receiveIdType, LinkedList<String> receiveIds) {
		FileTraceId fileTraceId = tmpProcessTrace.getFileTraceId(idType, id);
		Set<FileTraceId> fileTraces = new HashSet<>();

		fileTraces.add(fileTraceId);

		if (fileTraceId != null) {
			for (String sendId : receiveIds) {
				if (fileTraceId.getFileTrace().hasSendFileTraceId()) {
					FileTraceId originalfileTraceId = fileTraceId.getFileTrace().receiveFileTraceId();
					FileTrace fileTrace = originalfileTraceId.getFileTrace();
					FileOffset fileOffset = originalfileTraceId.getFileOffset();
					FileTraceId newFileTraceID = tmpProcessTrace.setFileTraceId(receiveIdType, sendId, fileTrace,
							fileOffset);
					// fileTraces.add(newFileTraceID);
				} else {
					// if no send FileTraceId is stored: ignore it
					// (this can be if the sending process is from a other program or the connection
					// between the sending and receiving socket is unknown: e.g. connection is build
					// via call of bind or connect function)
					noFileTrace++;
					logger.debug("without file-trace #{} in json {}", noFileTrace, tmpJson);
				}
			}
		}

		addEvent(fileTraces);
	}

	// TODO: use cmd method from AnalyzeFunctionPool class
	public void cmd(String cmd) throws ReflectiveOperationException,
			IllegalArgumentException {
		String tmpFunctionName = this.tmpFunctionName + "_" + cmd;

		if (fileTraceFunctions.containsFunctionName(tmpFunctionName)) {
			fileTraceFunctions.invoke(tmpFunctionName);
		} else {
			// TODO: default/Warning
			unknown++;
			logger.debug("unknown function-call #{} in json {}", unknown, tmpJson);
		}
	}

	public void ignore() {
		ignored++;
		logger.debug("ignored function-call #{} in json {}", ignored, tmpJson);
	}

	public FileId getRegularFileId(String hostName, String deviceId, String fileId) {
		long deviceId_long = Long.parseLong(deviceId);
		long fileId_long = Long.parseLong(fileId);
		return new FileId(new FileIdRegular(hostName, deviceId_long, fileId_long));
	}

	public FileId getSocketFileId(String hostName, String sockaddrFamily, String sockaddrData) {
		int sockaddrFamily_int = Integer.parseInt(sockaddrFamily);
		return new FileId(new FileIdSocket(hostName, sockaddrFamily_int, sockaddrData));
	}

	/**
	 * Returns the {@link FileTrace} from the current {@link ProcessTrace} for the
	 * given id or null if no such trace exists.
	 * 
	 * @param idType type of the id to search for the {@link FileTrace}
	 * @param id     id to search for the {@link FileTrace}
	 * @return {@link FileTrace} from the current {@link ProcessTrace} for the given
	 *         id or null
	 */
	private FileTrace getFileTrace(IdType idType, String id) {
		FileTraceId fileTraceId = tmpProcessTrace.getFileTraceId(idType, id);
		if (fileTraceId != null) {
			return fileTraceId.getFileTrace();
		}

		return null;
	}

	/**
	 * Returns the {@link FileOffset} from the current {@link ProcessTrace} for the
	 * given id or null if no such offset exists.
	 * 
	 * @param idType type of the id to search for the {@link FileOffset}
	 * @param id     id to search for the {@link FileOffset}
	 * @return {@link FileOffset} from the current {@link ProcessTrace} for the
	 *         given id or null
	 */
	private FileOffset getFileOffset(IdType idType, String id) {
		FileTraceId fileTraceId = tmpProcessTrace.getFileTraceId(idType, id);
		if (fileTraceId != null) {
			return fileTraceId.getFileOffset();
		}

		return null;
	}

	private Set<FileTrace> getFileTrace(String address, String length) {
		HashSet<FileTraceMemoryId> fileTraceMemoryIds = tmpProcessTrace.getFileTraceMemoryId(address, length);

		if (fileTraceMemoryIds != null) {
			Set<FileTrace> fileTraces = new HashSet<>();
			for (FileTraceMemoryId m : fileTraceMemoryIds) {
				FileTrace fileTrace = m.getFileTrace();
				fileTraces.add(fileTrace);
			}
			return fileTraces;
		}

		return null;
	}

	/**
	 * Casts a set of {@link FileTraceMemoryId}'s to a set of {@link FileTraceId}'s.
	 * 
	 * @param fileTraceMemoryIds set of {@link FileTraceMemoryId}'s to cast into set
	 *                           of {@link FileTraceId}'s
	 * @return set of {@link FileTraceId}'s
	 */
	private static Set<FileTraceId> getFileTraceId(Set<FileTraceMemoryId> fileTraceMemoryIds) {
		if (fileTraceMemoryIds != null) {
			Set<FileTraceId> fileTraceIds = new HashSet<FileTraceId>();
			fileTraceIds.addAll(fileTraceMemoryIds);
			return fileTraceIds;
		}

		return null;
	}

	public void printStats() {
		if (ignored > 0) {
			logger.warn("ignored functions-calls: {} (see more with log level debug)", ignored);
		}
		if (unknown > 0) {
			logger.warn("unknown functions-calls: {} (see more with log level debug)", unknown);
		}
		if (noFileTrace > 0) {
			logger.warn("function-calls without file-trace: {} (see more with log level debug)", noFileTrace);
		}

//		System.out.println();
//
//		ArrayList<FileKind> kinds = new ArrayList<>();
//		kinds.add(FileKind.FILE);
//		kinds.add(FileKind.TMPFILE);
//
//		System.out.print(cluster.printSummary(kinds, 1, 6, 1.0));
//
//		System.out.println();
//
//		System.out.print(fileTraces.printSummary(kinds, 1, 6, 1.0));
//
//		System.out.println();
//
//		searchConcurrency(kinds);
	}

	private void searchConcurrency(ArrayList<FileType> kinds) {
		for (Entry<String, FileTrace> fileTrace : fileTraces.getTraces().entrySet()) {
			if (kinds.contains(fileTrace.getValue().getFileType())) {
				Map<FunctionEvent, Set<FunctionEvent>> overlapping = fileTrace.getValue()
						.getOverlappingFunctionEvents();
				if (!overlapping.isEmpty()) {
					System.out.println("Overlapping function calls in " + fileTrace.getKey());
					for (Entry<FunctionEvent, Set<FunctionEvent>> overlapp : overlapping.entrySet()) {
						System.out.println("    during " + overlapp.getKey().getFunctionName() + "(from "
								+ overlapp.getKey().getThreadTrace() + ") " + overlapp.getKey().getJson());
						for (FunctionEvent event : overlapp.getValue()) {
							System.out.println("        " + event.getFunctionName() + " was called (from "
									+ event.getThreadTrace() + ") " + event.getJson());
						}
					}
				}
			}
		}
	}


	// - Getters
	public ResourceBundle getLegends(){
		return legends;
	}

	public BasicTrace<BasicTrace<BasicTrace<ThreadTrace>>> getCluster() {
		return cluster;
	}

	public BasicTrace<FileTrace> getFileTraces() {
		return fileTraces;
	}

	public String getDirectoryDelimiter() {
		return directoryDelimiter;
	}

	public boolean isHasWrapperInfo() {
		return hasWrapperInfo;
	}

	public Map<String, HashMap<String, ProcessTrace>> getProcessTraces() {
		return processTraces;
	}

	public KeyValueTreeNode getFork() {
		return fork;
	}
}
