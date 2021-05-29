package iotrace.model.analysis.trace.traceables;

import java.util.ArrayList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.ResourceBundle;
import java.util.TreeMap;

import iotrace.model.analysis.file.FileGroupId;
import iotrace.model.analysis.trace.traceables.FileTrace.FileType;
import iotrace.model.analysis.FunctionEvent;

public class FunctionTrace implements Traceable {
	private TreeMap<FileGroupId, FileGroupTrace> functionEvents = new TreeMap<>();
	private String name;

	private ResourceBundle legends;

	public FunctionTrace(String name, ResourceBundle legends) {
		super();
		this.name = name;
		this.legends = legends;
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
			fileGroupTrace = new FileGroupTrace(legends);
			functionEvents.put(fg, fileGroupTrace);
		}

		fileGroupTrace.add(e);
	}

	@Override
	public long getTimePeriod(ArrayList<FileType> kinds) {
		long time = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				time += e.getValue().getTimePeriod(kinds);
			}
		}

		return time;
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileType> kinds) {
		long time = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				time += e.getValue().getWrapperTimePeriod(kinds);
			}
		}

		return time;
	}

	@Override
	public long getByteCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getByteCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getFunctionCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getFunctionCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getOverlappingRangeCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				count += e.getValue().getOverlappingFunctionCount(kinds);
			}
		}

		return count;
	}

	@Override
	public long getCountSubTraces(ArrayList<FileType> kinds) {
		// TODO: cache
		return getSubTraces(kinds).size();
	}

	@Override
	public Map<String, Traceable> getSubTraces(ArrayList<FileType> kinds) {
		Map<String, Traceable> subTraces = new TreeMap<>();

		for (Entry<FileGroupId, FileGroupTrace> e : functionEvents.entrySet()) {
			if (e.getKey().isKind(kinds)) {
				String tmpKey = e.getKey().getFirstFileName(kinds);

				String newKey = tmpKey;
				int i = 1;
				while (subTraces.containsKey(newKey)) {
					i++;
					newKey = tmpKey + "(" + i + ")";
				}

				subTraces.put(newKey, e.getValue());
			}
		}

		return subTraces;
	}

	@Override
	public String getTraceName() {
		return name;
	}
}
