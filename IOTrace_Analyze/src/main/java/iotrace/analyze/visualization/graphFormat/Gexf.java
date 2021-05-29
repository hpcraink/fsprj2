package iotrace.analyze.visualization.graphFormat;

import java.util.Date;

public class Gexf {

	public static enum ModeType {
		STATIC, DYNAMIC
	}

	private static String getModeType(ModeType modeType) {
		String tmp;

		switch (modeType) {
		case DYNAMIC:
			tmp = "dynamic";
			break;
		case STATIC:
			tmp = "static";
			break;
		default:
			tmp = "";
			break;
		}

		return tmp;
	}

	public static enum ClassType {
		NODE, EDGE
	}

	private static String getClassType(ClassType classType) {
		String tmp;

		switch (classType) {
		case NODE:
			tmp = "node";
			break;
		case EDGE:
			tmp = "edge";
			break;
		default:
			tmp = "";
			break;
		}

		return tmp;
	}

	public static enum DefaultEdgeType {
		DIRECTED, UNDIRECTED, MUTUAL
	}

	private static String getDefaultEdgeType(DefaultEdgeType defaultEdgeType) {
		String tmp;

		switch (defaultEdgeType) {
		case DIRECTED:
			tmp = "directed";
			break;
		case UNDIRECTED:
			tmp = "undirected";
			break;
		case MUTUAL:
			tmp = "mutual";
			break;
		default:
			tmp = "";
			break;
		}

		return tmp;
	}

	public static enum TimeFormat {
		INTEGER, DOUBLE, DATE, DATETIME
	}

	private static String getTimeFormat(TimeFormat timeFormat) {
		String tmp;

		switch (timeFormat) {
		case INTEGER:
			tmp = "integer";
			break;
		case DOUBLE:
			tmp = "double";
			break;
		case DATE:
			tmp = "date";
			break;
		case DATETIME:
			tmp = "datetime";
			break;
		default:
			tmp = "";
			break;
		}

		return tmp;
	}

	public static enum AttrType {
		INTEGER, LONG, DOUBLE, FLOAT, BOOLEAN, STRING
	}

	private static String getAttrType(AttrType attrType) {
		String tmp;

		switch (attrType) {
		case INTEGER:
			tmp = "integer";
			break;
		case LONG:
			tmp = "long";
			break;
		case DOUBLE:
			tmp = "double";
			break;
		case FLOAT:
			tmp = "float";
			break;
		case BOOLEAN:
			tmp = "boolean";
			break;
		case STRING:
			tmp = "string";
			break;
		default:
			tmp = "";
			break;
		}

		return tmp;
	}

	public static String getStart(Date lastModified, String creator, String description, ModeType mode,
			DefaultEdgeType defaultEdgeType, TimeFormat timeFormat) {
		String tmp = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + System.lineSeparator()
				+ "<gexf xmlns=\"http://www.gexf.net/1.2draft\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.gexf.net/1.2draft http://www.gexf.net/1.2draft/gexf.xsd\" version=\"1.2\">"
				+ System.lineSeparator() + "    <meta lastmodifieddate=\"" + lastModified + "\">"
				+ System.lineSeparator() + "        <creator>" + creator + "</creator>" + System.lineSeparator()
				+ "        <description>" + description + "</description>" + System.lineSeparator() + "    </meta>\r\n"
				+ System.lineSeparator() + "    <graph mode=\"" + getModeType(mode) + "\" defaultedgetype=\""
				+ getDefaultEdgeType(defaultEdgeType) + "\" timeformat=\"" + getTimeFormat(timeFormat) + "\">"
				+ System.lineSeparator();

		return tmp;
	}

	public static String getAttributesStart(ClassType classType, ModeType modeType) {
		String tmp = "        <attributes class=\"" + getClassType(classType) + "\" mode=\"" + getModeType(modeType)
				+ "\">" + System.lineSeparator();

		return tmp;
	}

	public static String getAttribute(String id, String title, AttrType attrType) {
		String tmp = "            <attribute id=\"" + id + "\" title=\"" + title + "\" type=\"" + getAttrType(attrType)
				+ "\"/>" + System.lineSeparator();

		return tmp;
	}

	public static String getAttributesEnd() {
		return "        </attributes>" + System.lineSeparator();
	}

	public static String getNodesStart() {
		return "        <nodes>" + System.lineSeparator();
	}

	public static String getNodeStart(String id, String label) {
		String tmp = "            <node id=\"" + id + "\" label=\"" + label + "\">" + System.lineSeparator();

		return tmp;
	}

	public static String getAttValuesStart() {
		return "                <attvalues>" + System.lineSeparator();
	}

	public static String getAttValue(String id, String value, String start, String end) {
		String tmp = "                    <attvalue for=\"" + id + "\" value=\"" + value;

		if (start != null) {
			tmp += "\" start=\"" + start;
		}
		if (end != null) {
			tmp += "\" endopen=\"" + end;
		}

		return tmp + "\"/>" + System.lineSeparator();
	}

	public static String getAttValuesEnd() {
		return "                </attvalues>" + System.lineSeparator();
	}

	public static String getSpellsStart() {
		return "                <spells>" + System.lineSeparator();
	}

	public static String getSpell(String start, String end) {
		String tmp = "                    <spell start=\"" + start + "\" end=\"" + end + "\" />"
				+ System.lineSeparator();

		return tmp;
	}

	public static String getSpellsEnd() {
		return "                </spells>" + System.lineSeparator();
	}

	public static String getNodeEnd() {
		return "            </node>" + System.lineSeparator();
	}

	public static String getNodesEnd() {
		return "        </nodes>" + System.lineSeparator();
	}

	public static String getEdgesStart() {
		return "        <edges>" + System.lineSeparator();
	}

	public static String getEdgeStart(String id, String sourceId, String targetId) {
		String tmp = "            <edge id=\"" + id + "\" source=\"" + sourceId + "\" target=\"" + targetId + "\">"
				+ System.lineSeparator();

		return tmp;
	}

	public static String getEdgeEnd() {
		return "            </edge>" + System.lineSeparator();
	}

	public static String getEdgesEnd() {
		return "        </edges>" + System.lineSeparator();
	}

	public static String getEnd() {
		return "    </graph>" + System.lineSeparator() + "</gexf>";
	}
}
