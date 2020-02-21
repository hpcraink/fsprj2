package iotrace.analyze;

import java.util.ArrayList;
import java.util.Map;
import java.util.TreeMap;

import iotrace.analyze.FileTrace.FileKind;

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
	public Map<String, Traceable> getSubTraces(ArrayList<FileKind> kinds) {
		return (Map<String, Traceable>) traces;
	}

	@Override
	public long getTimePeriod(ArrayList<FileKind> kinds) {
		long time = 0;

		for (Traceable t : traces.values()) {
			time += t.getTimePeriod(kinds);
		}

		return time;
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileKind> kinds) {
		long time = 0;

		for (Traceable t : traces.values()) {
			time += t.getWrapperTimePeriod(kinds);
		}

		return time;
	}

	@Override
	public long getByteCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getByteCount(kinds);
		}

		return count;
	}

	@Override
	public long getFunctionCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getFunctionCount(kinds);
		}

		return count;
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getOverlappingRangeCount(kinds);
		}

		return count;
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Traceable t : traces.values()) {
			count += t.getOverlappingFunctionCount(kinds);
		}

		return count;
	}

	@Override
	public long getCountSubTraces(ArrayList<FileKind> kinds) {
		return traces.size();
	}
}
