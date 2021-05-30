package iotraceanalyze.model.analysis.trace.traceables;

import java.util.ArrayList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.ResourceBundle;

import iotraceanalyze.model.analysis.trace.traceables.FileTrace.FileType;
import iotraceanalyze.model.analysis.trace.KeyValueTreeNode;

public interface Traceable {
	public enum ValueType {
		FUNCTION_TIME, WRAPPER_TIME, BYTES, OVERLAPPING_FILE_RANGE, OVERLAPPING_FUNCTIONS
	}

	public long getTimePeriod(ArrayList<FileType> kinds);

	public long getWrapperTimePeriod(ArrayList<FileType> kinds);

	public long getByteCount(ArrayList<FileType> kinds);

	public long getFunctionCount(ArrayList<FileType> kinds);

	public long getOverlappingRangeCount(ArrayList<FileType> kinds);

	public long getOverlappingFunctionCount(ArrayList<FileType> kinds);

	public long getCountSubTraces(ArrayList<FileType> kinds);

	public Map<String, Traceable> getSubTraces(ArrayList<FileType> kinds);

	public String getTraceName();

	public default String getNameFromId(String id) {
		return id;
	}

	public default KeyValueTreeNode getKeyValueTree(ArrayList<FileType> kinds, ValueType valueType, int startDepth,
													int endDepth, double minPercent, ResourceBundle legends) {
		long fractalBase;
		String center = "";

		switch (valueType) {
			case BYTES:
				fractalBase = getByteCount(kinds);
				center = legends.getString("traceCenterBytes");
				break;
			case FUNCTION_TIME:
				fractalBase = getTimePeriod(kinds);
				center = legends.getString("traceCenterFunctionTime");
				break;
			case WRAPPER_TIME:
				fractalBase = getWrapperTimePeriod(kinds);
				center = legends.getString("traceCenterWrapperTime");
				break;
			case OVERLAPPING_FILE_RANGE:
				fractalBase = getOverlappingRangeCount(kinds);
				center = legends.getString("traceCenterOverlappingRange");
				break;
			case OVERLAPPING_FUNCTIONS:
				fractalBase = getOverlappingFunctionCount(kinds);
				center = legends.getString("traceCenterOverlappingFunctions");
				break;
			default:
				return null;
		}

		KeyValueTreeNode tmp = new KeyValueTreeNode(100, center, legends.getString("traceOther"));
		double fractal = (double) 100 / fractalBase;
		getKeyValueTree(kinds, valueType, startDepth, endDepth, 1, fractal, fractal, legends.getString("traceNode"),
				minPercent, tmp, legends);
		return tmp;
	}

	public default void getKeyValueTree(ArrayList<FileType> kinds, ValueType valueType, int startDepth, int endDepth,
										int depth, double fractal, double fractalPart, String id, double minPercent, KeyValueTreeNode tmp,
										ResourceBundle legends) {
		long functionCalls = getFunctionCount(kinds);
		KeyValueTreeNode next = null;

		if (depth > endDepth || functionCalls < 1) {
			return;
		}

		if (depth >= startDepth) {
			long value;

			switch (valueType) {
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
				next = new KeyValueTreeNode(percentToPrint, getTraceName() + ":" + getNameFromId(id),
						legends.getString("traceOther"));
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
				t.getValue().getKeyValueTree(kinds, valueType, startDepth, endDepth, depth + 1, fractal, fractalPart,
						t.getKey(), minPercent, tmp, legends);
			}
		}
	}

//	public default String printSummary(ArrayList<FileKind> kinds, int startDepth, int endDepth, double minPercent) {
//		return printSummary(kinds, startDepth, endDepth, 1, (double) 100 / getTimePeriod(kinds), "1", minPercent);
//	}
//
//	public default String printSummary(ArrayList<FileKind> kinds, int startDepth, int endDepth, int depth,
//			double fractal, String id, double minPercent) {
//		long functionCalls = getFunctionCount(kinds);
//		if (depth > endDepth || functionCalls < 1) {
//			return "";
//		}
//
//		String tmp = "";
//
//		if (depth >= startDepth) {
//			long nanoSeconds = getTimePeriod(kinds);
//			double percent = fractal * nanoSeconds;
//
//			if (percent >= minPercent) {
//				for (int i = 0; i < (depth - startDepth); i++) {
//					tmp += "    ";
//				}
//				tmp += (int) percent + " %, " + getTraceName() + ": " + getNameFromId(id) + ", " + nanoSeconds + " ns, "
//						+ getByteCount(kinds) + " bytes, " + functionCalls + " function-calls, "
//						+ getCountSubTraces(kinds) + " sub-traces" + System.lineSeparator();
//			}
//		}
//
//		Map<String, Traceable> traces = getSubTraces(kinds);
//		if (traces != null && traces.size() > 0) {
//			for (Entry<String, Traceable> t : traces.entrySet()) {
//				tmp += t.getValue().printSummary(kinds, startDepth, endDepth, depth + 1, fractal, t.getKey(),
//						minPercent);
//			}
//		}
//
//		return tmp;
//	}
}
