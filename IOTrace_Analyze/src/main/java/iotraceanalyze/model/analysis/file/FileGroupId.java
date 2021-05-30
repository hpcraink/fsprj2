package iotraceanalyze.model.analysis.file;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.TreeSet;

import iotraceanalyze.model.analysis.FunctionEvent;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace.FileType;

public class FileGroupId implements Comparable<FileGroupId> {
	private String[] files;
	private Map<FileType, String> kinds = new HashMap<>();

	public FileGroupId(ArrayList<FunctionEvent> events) {
		TreeSet<String> filesSort = new TreeSet<>();

		for (FunctionEvent e : events) {
			FileTrace fileTrace = e.getFileTrace();
			if (fileTrace == null) {
				String tmp = "unknown file";
				filesSort.add(tmp);
				kinds.put(FileType.UNKNOWN, tmp);
			} else {
				filesSort.add(fileTrace.toString());
				kinds.put(fileTrace.getFileType(), fileTrace.getNameFromId(""));
			}
		}

		this.files = new String[filesSort.size()];
		this.files = filesSort.toArray(this.files);
	}

	public boolean isKind(ArrayList<FileType> kinds) {
		for (FileType k : kinds) {
			if (this.kinds.containsKey(k)) {
				return true;
			}
		}
		return false;
	}

	public String getFirstFileName(ArrayList<FileType> kinds) {
		for (FileType k : kinds) {
			if (this.kinds.containsKey(k)) {
				return this.kinds.get(k);
			}
		}

		return null;
	}

	@Override
	public int compareTo(FileGroupId arg0) {
		if (files.length < arg0.files.length) {
			return -1;
		} else if (files.length > arg0.files.length) {
			return 1;
		} else {
			for (int i = 0; i < files.length; i++) {
				int comp = files[i].compareTo(arg0.files[i]);
				if (comp != 0) {
					return comp;
				}
			}
			return 0;
		}
	}

	@Override
	public String toString() {
		return Arrays.toString(files);
	}
}
