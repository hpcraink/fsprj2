package iotrace.analyze;

import java.util.ArrayList;
import java.util.Map;
import java.util.Map.Entry;
import iotrace.analyze.FileTrace.FileKind;

public interface Traceable {
	public static enum ValueKind {
		FUNCTION_TIME, WRAPPER_TIME, BYTES, OVERLAPPING_FILE_RANGE, OVERLAPPING_FUNCTIONS
	}

	public long getTimePeriod(ArrayList<FileKind> kinds);

	public long getWrapperTimePeriod(ArrayList<FileKind> kinds);

	public long getByteCount(ArrayList<FileKind> kinds);

	public long getFunctionCount(ArrayList<FileKind> kinds);

	public long getOverlappingRangeCount(ArrayList<FileKind> kinds);
	
	public long getOverlappingFunctionCount(ArrayList<FileKind> kinds);

	public long getCountSubTraces(ArrayList<FileKind> kinds);

	public Map<String, Traceable> getSubTraces(ArrayList<FileKind> kinds);

	public String getTraceName();

	public default String getNameFromId(String id) {
		return id;
	}

	public default KeyValueTreeNode getKeyValueTree(ArrayList<FileKind> kinds, ValueKind valueKind, int startDepth,
			int endDepth, double minPercent) {
		long fractalBase;
		String center = "";

		switch (valueKind) {
		case BYTES:
			fractalBase = getByteCount(kinds);
			center += " read and written bytes";
			break;
		case FUNCTION_TIME:
			fractalBase = getTimePeriod(kinds);
			center += " function time";
			break;
		case WRAPPER_TIME:
			fractalBase = getWrapperTimePeriod(kinds);
			center += " wrapper time";
			break;
		case OVERLAPPING_FILE_RANGE:
			fractalBase = getOverlappingRangeCount(kinds);
			center += " overlapping file ranges";
			break;
		case OVERLAPPING_FUNCTIONS:
			fractalBase = getOverlappingFunctionCount(kinds);
			center += " overlapping functions";
			break;
		default:
			return null;
		}

		KeyValueTreeNode tmp = new KeyValueTreeNode(100, center);
		double fractal = (double) 100 / fractalBase;
		getKeyValueTree(kinds, valueKind, startDepth, endDepth, 1, fractal, fractal, "node", minPercent, tmp);
		return tmp;
	}

	public default void getKeyValueTree(ArrayList<FileKind> kinds, ValueKind valueKind, int startDepth, int endDepth,
			int depth, double fractal, double fractalPart, String id, double minPercent, KeyValueTreeNode tmp) {
		long functionCalls = getFunctionCount(kinds);
		KeyValueTreeNode next = null;

		if (depth > endDepth || functionCalls < 1) {
			return;
		}

		if (depth >= startDepth) {
			long value;

			switch (valueKind) {
			case BYTES:
				value = getByteCount(kinds);
				break;
			case FUNCTION_TIME:
				value = getTimePeriod(kinds);
				break;
			case WRAPPER_TIME:
				value = getWrapperTimePeriod(kinds);
				break;
			case OVERLAPPING_FILE_RANGE:
				value = getOverlappingRangeCount(kinds);
				break;
			case OVERLAPPING_FUNCTIONS:
				value = getOverlappingFunctionCount(kinds);
				break;
			default:
				value = 0;
				break;
			}

			double percent = fractal * value;

			if (percent >= minPercent) {
				double percentToPrint = fractalPart * value;
				next = new KeyValueTreeNode(percentToPrint, getTraceName() + ":" + getNameFromId(id));
				tmp.addChild(next);
			} else {
				double percentToPrint = fractalPart * value;
				tmp.addOtherChild(percentToPrint);
				return;
			}

			fractalPart = (double) 100 / value;
		}

		Map<String, Traceable> traces = getSubTraces(kinds);
		if (traces != null && traces.size() > 0) {
			if (next != null) {
				tmp = next;
			}

			for (Entry<String, Traceable> t : traces.entrySet()) {
				t.getValue().getKeyValueTree(kinds, valueKind, startDepth, endDepth, depth + 1, fractal, fractalPart,
						t.getKey(), minPercent, tmp);
			}
		}
	}

	public default String printSummary(ArrayList<FileKind> kinds, int startDepth, int endDepth, double minPercent) {
		return printSummary(kinds, startDepth, endDepth, 1, (double) 100 / getTimePeriod(kinds), "1", minPercent);
	}

	public default String printSummary(ArrayList<FileKind> kinds, int startDepth, int endDepth, int depth,
			double fractal, String id, double minPercent) {
		long functionCalls = getFunctionCount(kinds);
		if (depth > endDepth || functionCalls < 1) {
			return "";
		}

		String tmp = "";

		if (depth >= startDepth) {
			long nanoSeconds = getTimePeriod(kinds);
			double percent = fractal * nanoSeconds;

			if (percent >= minPercent) {
				for (int i = 0; i < (depth - startDepth); i++) {
					tmp += "    ";
				}
				tmp += (int) percent + " %, " + getTraceName() + ": " + getNameFromId(id) + ", " + nanoSeconds + " ns, "
						+ getByteCount(kinds) + " bytes, " + functionCalls + " function-calls, "
						+ getCountSubTraces(kinds) + " sub-traces" + System.lineSeparator();
			}
		}

		Map<String, Traceable> traces = getSubTraces(kinds);
		if (traces != null && traces.size() > 0) {
			for (Entry<String, Traceable> t : traces.entrySet()) {
				tmp += t.getValue().printSummary(kinds, startDepth, endDepth, depth + 1, fractal, t.getKey(),
						minPercent);
			}
		}

		return tmp;
	}
}
