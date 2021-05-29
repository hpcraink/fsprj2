package iotrace.model.analysis.trace.traceables;

import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;

import iotrace.model.analysis.trace.traceables.FileTrace.FileType;
import iotrace.model.analysis.trace.traceables.Traceable;

public class BasicTrace<T extends Traceable> implements Traceable {
	private String name;
	private TreeMap<String, T> traces = new TreeMap<>();

	public BasicTrace(String name) {
		super();
		this.name = name;
	}

	@Override
	public String getTraceName() {
		return name;
	}

	public boolean containsTrace(String id) {
		return traces.containsKey(id);
	}

	public T addTrace(String id, T trace) {
		return traces.put(id, trace);
	}

	public T getTrace(String id) {
		return traces.get(id);
	}

	public TreeMap<String, T> getTraces() {
		return traces;
	}

	@SuppressWarnings("unchecked")
	@Override
	public Map<String, Traceable> getSubTraces(ArrayList<FileType> kinds) {
		return (Map<String, Traceable>) traces;
	}

	@Override
	public long getTimePeriod(ArrayList<FileType> kinds) {
		long time = 0;

		for (Traceable t : traces.values()) {
			time += t.getTimePeriod(kinds);
		}

		return time;
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileType> kinds) {
		long time = 0;

		for (Traceable t : traces.values()) {
			time += t.getWrapperTimePeriod(kinds);
		}

		return time;
	}

	@Override
	public long getByteCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getByteCount(kinds);
		}

		return count;
	}

	@Override
	public long getFunctionCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getFunctionCount(kinds);
		}

		return count;
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getOverlappingRangeCount(kinds);
		}

		return count;
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getOverlappingFunctionCount(kinds);
		}

		return count;
	}

	@Override
	public long getCountSubTraces(ArrayList<FileType> kinds) {
		return traces.size();
	}
}
