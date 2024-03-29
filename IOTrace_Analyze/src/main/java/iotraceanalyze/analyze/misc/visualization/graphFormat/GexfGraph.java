package iotraceanalyze.analyze.misc.visualization.graphFormat;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import iotraceanalyze.analyze.misc.visualization.graphFormat.Gexf.ClassType;
import iotraceanalyze.analyze.misc.visualization.graphFormat.Gexf.DefaultEdgeType;
import iotraceanalyze.analyze.misc.visualization.graphFormat.Gexf.ModeType;
import iotraceanalyze.analyze.misc.visualization.graphFormat.Gexf.TimeFormat;

public class GexfGraph {
	private static final Logger logger = LogManager.getLogger(GexfGraph.class);

	private Date lastModified;
	private String creator;
	private String description;
	private ModeType mode;
	private DefaultEdgeType defaultEdgeType;
	private TimeFormat timeFormat;

	private List<GexfAttributes> attributesList = new LinkedList<>();
	private List<GexfNode> nodes = new LinkedList<>();
	private List<GexfEdge> edges = new LinkedList<>();

	public GexfGraph(Date lastModified, String creator, String description, ModeType mode,
			DefaultEdgeType defaultEdgeType, TimeFormat timeFormat) {
		super();
		this.lastModified = lastModified;
		this.creator = creator;
		this.description = description;
		this.mode = mode;
		this.defaultEdgeType = defaultEdgeType;
		this.timeFormat = timeFormat;
	}

	public GexfAttributes createAttributes(ClassType classType, ModeType modeType) {
		GexfAttributes gexfAttributes = new GexfAttributes(classType, modeType);

		attributesList.add(gexfAttributes);

		return gexfAttributes;
	}

	public GexfNode createNode(String id, String label) {
		GexfNode gexfNode = new GexfNode(id, label);

		nodes.add(gexfNode);

		return gexfNode;
	}

	public GexfEdge createEdge(String id, GexfNode sourceNode, GexfNode targetNode) {
		GexfEdge gexfNode = new GexfEdge(id, sourceNode, targetNode);

		edges.add(gexfNode);

		return gexfNode;
	}

	public void write(File file) {
		try (FileWriter fileWriter = new FileWriter(file)) {
			BufferedWriter bufferedWriter = new BufferedWriter(fileWriter);

			bufferedWriter.write(Gexf.getStart(lastModified, creator, description, mode, defaultEdgeType, timeFormat));

			for (GexfAttributes a : attributesList) {
				a.write(bufferedWriter);
			}

			bufferedWriter.write(Gexf.getNodesStart());
			for (GexfNode n : nodes) {
				n.write(bufferedWriter);
			}
			bufferedWriter.write(Gexf.getNodesEnd());

			bufferedWriter.write(Gexf.getEdgesStart());
			for (GexfEdge e : edges) {
				e.write(bufferedWriter);
			}
			bufferedWriter.write(Gexf.getEdgesEnd());

			bufferedWriter.write(Gexf.getEnd());

			bufferedWriter.close();
		} catch (IOException e) {
			logger.error("Exception during write of gexf file", e);
		}
	}
}
