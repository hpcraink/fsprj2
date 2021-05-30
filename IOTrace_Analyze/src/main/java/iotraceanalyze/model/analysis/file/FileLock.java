package iotraceanalyze.model.analysis.file;

import iotraceanalyze.model.analysis.FunctionEvent;
import iotraceanalyze.model.analysis.trace.traceables.ThreadTrace;

import java.util.Set;
import java.util.TreeSet;

public class FileLock {
	private FileRange fileRange = null;
	private ThreadTrace threadTrace;
	private Set<FunctionEvent> functionEvents = new TreeSet<>();

	public FileLock(FileRange fileRange, ThreadTrace threadTrace) {
		this.fileRange = fileRange;
		this.threadTrace = threadTrace;
	}

	public void addFunctionEvent(FunctionEvent functionEvent) {
		functionEvents.add(functionEvent);
	}

	public Set<FunctionEvent> getFunctionEvents() {
		return functionEvents;
	}

	public FileRange getFileRange() {
		return fileRange;
	}

	public ThreadTrace getThreadTrace() {
		return threadTrace;
	}
}
