package iotrace.analyze;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Map;
import java.util.ResourceBundle;
import java.util.TreeSet;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import iotrace.analyze.FileTrace.FileKind;

public class ThreadTrace implements Traceable, Comparable<ThreadTrace> {
	private static final Logger logger = LogManager.getLogger(ThreadTrace.class);

	private String hostName;
	private String processId;
	private String threadId;
	private FunctionEvent firstFunctionEvent = null;
	private FunctionEvent lastFunctionEvent = null;
	private TreeSet<FunctionEvent> events = new TreeSet<>();
	private BasicTrace<FunctionTrace> thread;

	private ResourceBundle legends;

	public ThreadTrace(String hostName, String processId, String threadId, ResourceBundle legends) {
		super();
		this.hostName = hostName;
		this.processId = processId;
		this.threadId = threadId;
		this.legends = legends;
		thread = new BasicTrace<>(legends.getString("threadTraceThreadTitle"));
	}

	public String getHostName() {
		return hostName;
	}

	public String getProcessId() {
		return processId;
	}

	public String getThreadId() {
		return threadId;
	}

	public void addEvent(FunctionEvent event) {
		events.add(event);
	}

	public TreeSet<FunctionEvent> getEvents() {
		return events;
	}

	public void processEvents(AnalyzeFunctionPool fileRangeFunctions, AnalyzeFunctionPool fileLockFunctions) {
		if (events.isEmpty()) {
			return;
		}

		firstFunctionEvent = events.first();
		lastFunctionEvent = events.last();

		FunctionEvent before = null;
		for (FunctionEvent event : events) {
			if (before != null) {
				before.setNextInThreadTrace(event);
				event.setPreviousInThreadTrace(before);
			} else {
				event.setPreviousInThreadTrace(null);
			}
			event.setNextInThreadTrace(null);

			before = event;
		}

		groupEvents(fileRangeFunctions, fileLockFunctions);
	}

	private void groupEvents(AnalyzeFunctionPool fileRangeFunctions, AnalyzeFunctionPool fileLockFunctions) {
		fileRangeFunctions.addObject(fileRangeFunctions);
		fileLockFunctions.addObject(fileLockFunctions);
		FunctionEvent event = firstFunctionEvent;

		while (event != null) {
			String functionName = event.getFunctionName();

			if (fileRangeFunctions.containsFunctionName(functionName)) {
				fileRangeFunctions.addObject(event);
				fileRangeFunctions.addObject(event.getJson());
				try {
					fileRangeFunctions.invoke(functionName);
				} catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException
						| NoSuchMethodException e) {

					logger.error("Exception during invokation of method for creating file range for function "
							+ functionName + " for event " + event.getJson(), e);
				}
			} else {
				logger.trace("no method for creating file range for function " + functionName + " for event "
						+ event.getJson());
			}

			if (fileLockFunctions.containsFunctionName(functionName)) {
				fileLockFunctions.addObject(event);
				fileLockFunctions.addObject(event.getJson());
				try {
					fileLockFunctions.invoke(functionName);
				} catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException
						| NoSuchMethodException e) {
					logger.error("Exception during invokation of method for creating file lock for function "
							+ functionName + " for event " + event.getJson(), e);
				}
			}

			FunctionTrace function;
			if (!thread.containsTrace(functionName)) {
				function = new FunctionTrace(legends.getString("threadTraceFunctionTitle"));
				thread.addTrace(functionName, function);
			} else {
				function = thread.getTrace(functionName);
			}

			function.addFunctionEvent(event);

			event = event.getNextCallInThreadTrace();
		}
	}

	@Override
	public long getTimePeriod(ArrayList<FileKind> kinds) {
		return thread.getTimePeriod(kinds);
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileKind> kinds) {
		return thread.getWrapperTimePeriod(kinds);
	}

	@Override
	public long getByteCount(ArrayList<FileKind> kinds) {
		return thread.getByteCount(kinds);
	}

	@Override
	public long getFunctionCount(ArrayList<FileKind> kinds) {
		return thread.getFunctionCount(kinds);
	}

	@Override
	public long getCountSubTraces(ArrayList<FileKind> kinds) {
		return thread.getCountSubTraces(kinds);
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileKind> kinds) {
		return thread.getOverlappingRangeCount(kinds);
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileKind> kinds) {
		return thread.getOverlappingFunctionCount(kinds);
	}

	@Override
	public String getTraceName() {
		return thread.getTraceName();
	}

	@Override
	public Map<String, Traceable> getSubTraces(ArrayList<FileKind> kinds) {
		return thread.getSubTraces(kinds);
	}

	public FunctionEvent getFirstFunctionEvent() {
		return firstFunctionEvent;
	}

	public void setFirstFunctionEvent(FunctionEvent firstFunctionEvent) {
		this.firstFunctionEvent = firstFunctionEvent;
	}

	public FunctionEvent getLastFunctionEvent() {
		return lastFunctionEvent;
	}

	public void setLastFunctionEvent(FunctionEvent lastFunctionEvent) {
		this.lastFunctionEvent = lastFunctionEvent;
	}

	@Override
	public String toString() {
		return "Thread-Trace [hostName=" + hostName + ", processId=" + processId + ", threadId=" + threadId + "]";
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((hostName == null) ? 0 : hostName.hashCode());
		result = prime * result + ((processId == null) ? 0 : hostName.hashCode());
		result = prime * result + ((threadId == null) ? 0 : hostName.hashCode());
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
		ThreadTrace other = (ThreadTrace) obj;
		if (hostName == null) {
			if (other.hostName != null)
				return false;
		} else if (!hostName.equals(other.hostName))
			return false;
		if (processId != other.processId)
			return false;
		if (threadId != other.threadId)
			return false;
		return true;
	}

	@Override
	public int compareTo(ThreadTrace arg0) {
		int cmpHostName = hostName.compareTo(arg0.hostName);
		if (cmpHostName == 0) {
			int cmpProcessId = Integer.valueOf(processId).compareTo(Integer.valueOf(arg0.processId));
			if (cmpProcessId == 0) {
				int cmpThreadId = Integer.valueOf(threadId).compareTo(Integer.valueOf(arg0.threadId));
				return cmpThreadId;
			} else {
				return cmpProcessId;
			}
		} else {
			return cmpHostName;
		}
	}
}
