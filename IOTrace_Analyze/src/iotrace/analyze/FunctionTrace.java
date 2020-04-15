package iotrace.analyze;

import java.util.ArrayList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

import iotrace.analyze.FileTrace.FileKind;

public class FunctionTrace implements Traceable {
	private TreeMap<FileGroupId, FileGroupTrace> functionEvents = new TreeMap<>();
	private String name;

	public FunctionTrace(String name) {
		super();
		this.name = name;
	}

	public void addFunctionEvent(FunctionEvent e) {
		ArrayList<FunctionEvent> eventGroup = new ArrayList<>();

		eventGroup.add(e);
		if (e.getSame() != null) {
			for (FunctionEvent fe : e.getSame()) {
				eventGroup.add(fe);
			}
		}

		FileGroupId fg = new FileGroupId(eventGroup);

		FileGroupTrace fileGroupTrace;
		if (functionEvents.containsKey(fg)) {
			fileGroupTrace = functionEvents.get(fg);
		} else {
			fileGroupTrace = new FileGroupTrace();
			functionEvents.put(fg, fileGroupTrace);
		}

		fileGroupTrace.add(e);
	}

	@Override
	public long getTimePeriod(ArrayList<FileKind> kinds) {
		long time = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				time += e.getValue().getTimePeriod(kinds);
			}
		}

		return time;
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileKind> kinds) {
		long time = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				time += e.getValue().getWrapperTimePeriod(kinds);
			}
		}

		return time;
	}

	@Override
	public long getByteCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getByteCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getFunctionCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getFunctionCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getOverlappingRangeCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getOverlappingFunctionCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getCountSubTraces(ArrayList<FileKind> kinds) {
		// TODO: cache
		return getSubTraces(kinds).size();
	}

	@Override
	public Map<String, Traceable> getSubTraces(ArrayList<FileKind> kinds) {
		Map<String, Traceable> subTraces = new TreeMap<>();

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				subTraces.put(e.getKey().getFirstFileName(kinds), e.getValue());
			}
		}

		return subTraces;
	}

	@Override
	public String getTraceName() {
		return name;
	}
}
