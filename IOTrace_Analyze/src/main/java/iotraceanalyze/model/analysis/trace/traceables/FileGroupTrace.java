package iotraceanalyze.model.analysis.trace.traceables;

import java.util.ArrayList;
import java.util.Map;
import java.util.ResourceBundle;

import iotraceanalyze.model.analysis.FunctionEvent;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace.FileType;

public class FileGroupTrace implements Traceable {
	private ArrayList<FunctionEvent> events = new ArrayList<>();
	
	private ResourceBundle legends;
	
	public FileGroupTrace(ResourceBundle legends) {
		super();
		this.legends = legends;
	}

	public void add(FunctionEvent event) {
		events.add(event);
	}

	@Override
	public long getTimePeriod(ArrayList<FileType> kinds) {
		long time = 0;

		for (FunctionEvent e : events) {
			time += e.getFunctionTimePeriod();
		}

		return time;
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileType> kinds) {
		long time = 0;

		for (FunctionEvent e : events) {
			time += e.getWrapperTimePeriod();
		}

		return time;
	}

	@Override
	public long getByteCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (FunctionEvent e : events) {
			count += e.getFunctionByteCount();
		}

		return count;
	}

	@Override
	public long getFunctionCount(ArrayList<FileType> kinds) {
		return events.size();
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (FunctionEvent e : events) {
			if (e.hasOverlappingFileRange()) {
				count++;
			}
		}

		return count;
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileType> kinds) {
		long count = 0;

		for (FunctionEvent e : events) {
			if (e.hasOverlappingFunction()) {
				count++;
			}
		}

		return count;
	}

	@Override
	public long getCountSubTraces(ArrayList<FileType> kinds) {
		return 0;
	}

	@Override
	public Map<String, Traceable> getSubTraces(ArrayList<FileType> kinds) {
		return null;
	}

	@Override
	public String getTraceName() {
		return legends.getString("fileTraceFileTitle");
	}
}
