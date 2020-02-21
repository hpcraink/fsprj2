package iotrace.analyze;

import java.util.ArrayList;
import java.util.Map;

import iotrace.analyze.FileTrace.FileKind;

public class FileGroupTrace implements Traceable {
	private ArrayList<FunctionEvent> events = new ArrayList<>();

	public void add(FunctionEvent event) {
		events.add(event);
	}

	@Override
	public long getTimePeriod(ArrayList<FileKind> kinds) {
		long time = 0;

		for (FunctionEvent e : events) {
			time += e.getFunctionTimePeriod();
		}

		return time;
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileKind> kinds) {
		long time = 0;

		for (FunctionEvent e : events) {
			time += e.getWrapperTimePeriod();
		}

		return time;
	}

	@Override
	public long getByteCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (FunctionEvent e : events) {
			count += e.getFunctionByteCount();
		}

		return count;
	}

	@Override
	public long getFunctionCount(ArrayList<FileKind> kinds) {
		return events.size();
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (FunctionEvent e : events) {
			if (e.hasOverlappingFileRange()) {
				count++;
			}
		}

		return count;
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileKind> kinds) {
		long count = 0;

		for (FunctionEvent e : events) {
			if (e.hasOverlappingFunction()) {
				count++;
			}
		}

		return count;
	}

	@Override
	public long getCountSubTraces(ArrayList<FileKind> kinds) {
		return 0;
	}

	@Override
	public Map<String, Traceable> getSubTraces(ArrayList<FileKind> kinds) {
		return null;
	}

	@Override
	public String getTraceName() {
		return "files";
	}
}
