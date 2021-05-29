package iotrace.analyze.visualization.graphFormat;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import iotrace.model.analysis.FunctionEvent;

public class Dot {

	public Dot() {
		// TODO Auto-generated constructor stub
	}

	public static String getDotNode(String id, String label, String type) {
		return getDotNode(id, label, type, null);
	}

	public static String getDotNode(String id, String label, String type, Map<String, String> attributes) {
		String tmp = "\"" + id + "\" [label=\"" + label + "\" type=\"" + type + "\"";

		if (attributes != null) {
			for (Entry<String, String> e : attributes.entrySet()) {
				tmp += " \"" + e.getKey() + "\"=\"" + e.getValue() + "\"";
			}
		}

		tmp += "];";

		return tmp;
	}

	public static String getDotEdge(String fromNode, String toNode) {
		return getDotEdge(fromNode, toNode, null);
	}

	public static String getDotEdge(String fromNode, String toNode, Map<String, String> attributes) {
		String tmp = "\"" + fromNode + "\"->\"" + toNode + "\"";

		if (attributes != null) {
			tmp += " [";
			for (Entry<String, String> e : attributes.entrySet()) {
				tmp += " \"" + e.getKey() + "\"=\"" + e.getValue() + "\"";
			}
			tmp += "]";
		}

		tmp += ";";

		return tmp;
	}

	public static String getDotEdge(FunctionEvent fe) {
		if (fe.getFileTrace() != null) {
			Map<String, String> attributes = new HashMap<>();

			if (fe.getFileRange() != null) {
				switch (fe.getFileRange().getRangeType()) {
				case READ:
					attributes.put("ioType", "read");
					break;
				case WRITE:
					attributes.put("ioType", "write");
					break;
				default:
					break;
				}
			}

			attributes.put("function", fe.getFunctionName());
			attributes.put("startTime", Long.toString(fe.getStartTime()));
			attributes.put("endTime", Long.toString(fe.getEndTime()));
			attributes.put("bytes", String.valueOf(fe.getFunctionByteCount()));
			attributes.put("error", String.valueOf(fe.hasError()));

			String threadId = "Host:" + fe.getThreadTrace().getHostName() + ":Process:"
					+ fe.getThreadTrace().getProcessId() + ":Thread:" + fe.getThreadTrace().getThreadId();
			String file = fe.getFileTrace().getFileId().toString();

			return Dot.getDotEdge(threadId, file, attributes);
		}

		return "";
	}

//	private String getDotNode(String id, String label) {
//		return "\"" + id + "\" [label=\"" + label + "\"];";
//	}
//
//	private String getDotNode(String id, String label, String shape) {
//		return "\"" + id + "\" [label=\"" + label + "\" shape=\"" + shape + "\"];";
//	}
//
//	private String getDotNode(String id, String label, String group, String rank) {
//		return "\"" + id + "\" [label=\"" + label + "\" group=\"" + group + "\" rank=\"" + rank + "\"];";
//	}
//
//	private String getDotSubgraphStart(String id, String label) {
//		return "subgraph cluster_" + id + " {rank=same;label=\"" + label + "\"";
//	}
//
//	private String getDotSubgraphEnd() {
//		return "}";
//	}
}
