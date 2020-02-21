package iotrace.analyze;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.TreeSet;

import iotrace.analyze.FileTrace.FileKind;

public class FileGroupId implements Comparable<FileGroupId> {
	private String[] files;
	private HashSet<FileKind> kinds = new HashSet<>();

	public FileGroupId(ArrayList<FunctionEvent> events) {
		TreeSet<String> filesSort = new TreeSet<>();

		for (FunctionEvent e : events) {
			FileTrace fileTrace = e.getFileTrace();
			if (fileTrace == null) {
				filesSort.add("unknown file");
				kinds.add(FileKind.UNKNOWN);
			} else {
				filesSort.add(fileTrace.toString());
				kinds.add(fileTrace.getKind());
			}
		}

		this.files = new String[filesSort.size()];
		this.files = filesSort.toArray(this.files);
	}

	public boolean isKind(ArrayList<FileKind> kinds) {
		for (FileKind k : kinds) {
			if (this.kinds.contains(k)) {
				return true;
			}
		}
		return false;
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
