package iotrace.analyze.gephi;

import java.awt.Color;
import java.awt.Font;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Map.Entry;
import javax.imageio.ImageIO;
import org.gephi.appearance.api.AppearanceController;
import org.gephi.appearance.api.AppearanceModel;
import org.gephi.appearance.api.Function;
import org.gephi.appearance.api.Partition;
import org.gephi.appearance.api.PartitionFunction;
import org.gephi.appearance.api.RankingFunction;
import org.gephi.appearance.plugin.PartitionElementColorTransformer;
import org.gephi.appearance.plugin.RankingNodeSizeTransformer;
import org.gephi.appearance.plugin.palette.Palette;
import org.gephi.appearance.plugin.palette.PaletteManager;
import org.gephi.appearance.plugin.palette.Preset;
import org.gephi.appearance.spi.Transformer;
import org.gephi.datalab.api.AttributeColumnsController;
import org.gephi.filters.api.FilterController;
import org.gephi.filters.api.Query;
import org.gephi.filters.api.Range;
import org.gephi.filters.plugin.dynamic.DynamicRangeBuilder.DynamicRangeFilter;
import org.gephi.graph.api.Column;
import org.gephi.graph.api.DirectedGraph;
import org.gephi.graph.api.Edge;
import org.gephi.graph.api.Estimator;
import org.gephi.graph.api.GraphController;
import org.gephi.graph.api.GraphModel;
import org.gephi.graph.api.GraphView;
import org.gephi.graph.api.Interval;
import org.gephi.graph.api.Node;
import org.gephi.graph.api.Table;
import org.gephi.graph.api.types.IntervalDoubleMap;
import org.gephi.graph.api.types.IntervalLongMap;
import org.gephi.io.exporter.api.ExportController;
import org.gephi.io.exporter.preview.PNGExporter;
import org.gephi.io.importer.api.Container;
import org.gephi.io.importer.api.EdgeDirectionDefault;
import org.gephi.io.importer.api.ImportController;
import org.gephi.io.processor.plugin.DefaultProcessor;
import org.gephi.layout.plugin.forceAtlas2.ForceAtlas2;
import org.gephi.layout.plugin.forceAtlas2.ForceAtlas2Builder;
import org.gephi.preview.api.PreviewController;
import org.gephi.preview.api.PreviewModel;
import org.gephi.preview.api.PreviewProperties;
import org.gephi.preview.api.PreviewProperty;
import org.gephi.preview.types.EdgeColor;
import org.gephi.project.api.ProjectController;
import org.gephi.project.api.Workspace;
import org.jcodec.api.awt.AWTSequenceEncoder;
import org.jcodec.common.io.NIOUtils;
import org.jcodec.common.io.SeekableByteChannel;
import org.jcodec.common.model.Rational;
import org.openide.util.Lookup;

public class GephiVideo {

	private ExportController exportController;
	private PNGExporter pngExporter;

	private ProjectController projectController;
	private Workspace workspace;
	private GraphModel graphModel;

	private Preset presetNodes;
	private Preset presetEdges;
	private PreviewProperties previewProperties;

	private Partition nodePartition;
	private Partition edgePartition;

	private AppearanceController appearanceController;
	private AppearanceModel appearanceModel;

	private DirectedGraph graph;

	private FilterController filterController;

	private int legendSize;
	private List<Node> legendNodes = new LinkedList<>();
	private List<String> legendNodesLabel = new LinkedList<>();
	private List<Node> legendTexts = new LinkedList<>();
	private long nodeSizeMin;
	private long nodeSizeMax;
	private String nodeSizeText;
	private float nodeLabelMinSizePercent;
	private long edgeSizeMin;
	private long edgeSizeMax;
	private String edgeSizeText;
	private boolean logScaledEdge = false;
	private Node frames = null;
	private Node frame = null;

	private float minX = Float.MAX_VALUE;
	private float maxX = Float.MIN_VALUE;
	private float minY = Float.MAX_VALUE;
	private float maxY = Float.MIN_VALUE;
	private float minZ = Float.MAX_VALUE;
	private float maxZ = Float.MIN_VALUE;

	public GephiVideo(String gexfFile, int height, int width, int maxLayoutPasses, float baseEdgeThickness)
			throws FileNotFoundException {
		super();

		System.out.println("Start initialize ...");

		// Initialize export
		exportController = Lookup.getDefault().lookup(ExportController.class);
		pngExporter = (PNGExporter) exportController.getExporter("png"); // Get PNG exporter
		pngExporter.setHeight(height);
		pngExporter.setWidth(width);

		// Initialize a project - and therefore a workspace
		projectController = Lookup.getDefault().lookup(ProjectController.class);
		projectController.newProject();
		workspace = projectController.getCurrentWorkspace();

		System.out.println("End initialize");

		System.out.println("Start import ...");

		File file = new File(gexfFile);
		try (BufferedReader br = new BufferedReader(new FileReader(file))) {
			// Import file
			ImportController importController = Lookup.getDefault().lookup(ImportController.class);
			Container container;

			// container = importController.importFile(file);
			container = importController.importFile(br, importController.getFileImporter(file));
			container.getLoader().setEdgeDefault(EdgeDirectionDefault.DIRECTED); // Force DIRECTED

			// Append imported data to GraphAPI
			importController.process(container, new DefaultProcessor(), workspace);

			// Get graph model of current workspace
			graphModel = Lookup.getDefault().lookup(GraphController.class).getGraphModel();
		} catch (IOException e) {
			e.printStackTrace();
			return;
		}

		System.out.println("End import");

		// Build clusters and set position of nodes
		graphLayout(maxLayoutPasses);

		// Get graph
		graph = graphModel.getDirectedGraph();

		// Evaluate min and max values of node coordinates
		setBorders();

		// Set colors
		presetNodes = new Preset("Nodes", true, 0, 150, 1.0f, 1.6f, 0.5f, 1.5f);
		presetEdges = new Preset("Edges", true, 200, 350, 1.0f, 1.6f, 0.5f, 1.5f);

		// Get controller for setting color and size of nodes/edges
		appearanceController = Lookup.getDefault().lookup(AppearanceController.class);
		appearanceModel = appearanceController.getModel();

		// Preview configuration
		baseEdgeThickness = scaleEdge(baseEdgeThickness);
		PreviewController previewController = Lookup.getDefault().lookup(PreviewController.class);
		PreviewModel previewModel = previewController.getModel();
		previewProperties = previewModel.getProperties();
		previewProperties.putValue(PreviewProperty.EDGE_COLOR, new EdgeColor(EdgeColor.Mode.ORIGINAL));
		previewProperties.putValue(PreviewProperty.EDGE_CURVED, false);
		previewProperties.putValue(PreviewProperty.NODE_PER_NODE_OPACITY, true);
		previewProperties.putValue(PreviewProperty.SHOW_NODE_LABELS, true);
		previewProperties.putValue(PreviewProperty.NODE_LABEL_PROPORTIONAL_SIZE, false);
		// previewProperties.putValue(PreviewProperty.EDGE_RESCALE_WEIGHT, true);
		// previewProperties.putValue(PreviewProperty.EDGE_RESCALE_WEIGHT_MIN, 2.0f);
		// previewProperties.putValue(PreviewProperty.EDGE_RESCALE_WEIGHT_MAX, 30.0f);
		previewProperties.putValue(PreviewProperty.EDGE_THICKNESS, baseEdgeThickness);

		// Get filter controller
		filterController = Lookup.getDefault().lookup(FilterController.class);
	}

	public float scaleToBorders(float f) {
		return f / 1200 * (maxX - minX);
	}

	public float scaleEdge(float f) {
		return f / 1200 * (maxX - minX);
//		if ((maxX - minX) <= 700) {
//			return scaleToBorders(f);
//		} else {
//			return f;
//		}
	}

	private void graphLayout(int maxLayoutPasses) {
		ForceAtlas2Builder forceAtlas2Builder = new ForceAtlas2Builder();
		ForceAtlas2 forceAtlas2 = new ForceAtlas2(forceAtlas2Builder);

		forceAtlas2.setGraphModel(graphModel); // must be set before layout attributes

		forceAtlas2.setThreadsCount(3);
		forceAtlas2.setJitterTolerance(1.0);
		forceAtlas2.setBarnesHutOptimize(false);
		forceAtlas2.setBarnesHutTheta(1.2);
		forceAtlas2.setScalingRatio(150.0);
		forceAtlas2.setStrongGravityMode(true);
		forceAtlas2.setGravity(1.0);
		forceAtlas2.setOutboundAttractionDistribution(false);
		forceAtlas2.setLinLogMode(false);
		forceAtlas2.setAdjustSizes(true);
		forceAtlas2.setEdgeWeightInfluence(1.0);

		// Run ForceAtlas 2 for maxLayoutPasses passes - The layout always takes the
		// current
		// visible view
		forceAtlas2.initAlgo();
		for (int i = 0; i < maxLayoutPasses && forceAtlas2.canAlgo(); i++) {
			forceAtlas2.goAlgo();
		}
		forceAtlas2.endAlgo();
	}

	public void setNodeColor(String attribute) {
		// Partition nodes
		Column column = graphModel.getNodeTable().getColumn(attribute);
		Function func = appearanceModel.getNodeFunction(graph, column, PartitionElementColorTransformer.class);
		nodePartition = ((PartitionFunction) func).getPartition();
		// Set color for partitions
		Palette palette = PaletteManager.getInstance().generatePalette(nodePartition.size(), presetNodes);
		nodePartition.setColors(palette.getColors());
		appearanceController.transform(func);
	}

	public Function setEdgeColor(String attribute) {
		// Partition edges
		Column column = graphModel.getEdgeTable().getColumn(attribute);
		column.setEstimator(Estimator.LAST);
		Function func = appearanceModel.getEdgeFunction(graph, column, PartitionElementColorTransformer.class);
		edgePartition = ((PartitionFunction) func).getPartition();
		// Set color for partitions
		Palette palette = PaletteManager.getInstance().generatePalette(edgePartition.size(), presetEdges);
		edgePartition.setColors(palette.getColors());
		appearanceController.transform(func);

		return func;
	}

	public Function setNodeSize(String attribute, int minSize, int maxSize, String nodeSizeText) {
		this.nodeSizeText = nodeSizeText;

		// Ranking nodes
		Column column = graphModel.getNodeTable().getColumn(attribute);
		column.setEstimator(Estimator.LAST);
		Function func = appearanceModel.getNodeFunction(graph, column, RankingNodeSizeTransformer.class);
		Transformer transformer = ((RankingFunction) func).getTransformer();
		((RankingNodeSizeTransformer) transformer).setMinSize(minSize);
		((RankingNodeSizeTransformer) transformer).setMaxSize(maxSize);

		getNodeSizeFrame(column);

		return func;
	}

	public void savePNG(String pngfile) {
		File file = new File(pngfile);
		double start = graphModel.getTimeBounds().getLow();
		double end = graphModel.getTimeBounds().getHigh();
		String endString = String.valueOf((long) end);
		int endLength = endString.length();

		frames.setLabel("");
		frame.setLabel("Current frame: " + String.format("%" + endLength + "d", (long) start) + " ns to "
				+ String.format("%" + endLength + "d", (long) end) + " ns.");

		try {
			exportController.exportFile(file, pngExporter);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public void saveMP4(String mp4File, Function funcNodeSize, Function funcEdgeColor, float rangeSizePercent,
			int framesPerSecond, int secondsForAnimation, String labelFromColumn) {
		double start = graphModel.getTimeBounds().getLow();
		double end = graphModel.getTimeBounds().getHigh();
		double duration = end - start;
		int framesForAnimation = framesPerSecond * secondsForAnimation;
		double step = duration / framesForAnimation;
		double rangeSize = duration / 100 * rangeSizePercent;
		String endString = String.valueOf((long) end);
		int endLength = endString.length();

		frames.setLabel(
				framesPerSecond + " frames per second; each frame " + rangeSizePercent + "% of program duration.");

		DynamicRangeFilter dynamicRangeFilter = new DynamicRangeFilter(graphModel);

		SeekableByteChannel outMP4 = null;
		try {
			outMP4 = NIOUtils.writableFileChannel(mp4File);
			// for Android use: AndroidSequenceEncoder
			AWTSequenceEncoder encoder = new AWTSequenceEncoder(outMP4, Rational.R(framesPerSecond, 1));

			GraphView viewSave = graphModel.getVisibleView();

			for (double i = start; i + rangeSize < end; i += step) {
				frame.setLabel("Current frame: " + String.format("%" + endLength + "d", (long) i) + " ns to "
						+ String.format("%" + endLength + "d", (long) (i + rangeSize)) + " ns.");

				// Range Filter
				Range range = new Range(i, i + rangeSize);
				dynamicRangeFilter.setRange(range);
				Query query = filterController.createQuery(dynamicRangeFilter);
				GraphView view = filterController.filter(query);
				graphModel.setVisibleView(view); // Set the filter result as the visible view

				// Refresh ranking of nodes for each export
				appearanceController.transform(funcNodeSize);
				// Reset node size for legend nodes
				resizeLegendNodes();
				// Refresh edge color
				colorEdges(funcEdgeColor);
				// appearanceController.transform(funcEdgeColor);

				// Show labels
				showLabel(labelFromColumn, nodeLabelMinSizePercent, funcNodeSize);

				// Print graph as png to output stream
				ByteArrayOutputStream out = new ByteArrayOutputStream();
				exportController.exportStream(out, pngExporter);

				// Create input stream from output stream
				ByteArrayInputStream input = new ByteArrayInputStream(out.toByteArray());

				// Encode the image to mp4 output
				encoder.encodeImage(ImageIO.read(input));
			}

			// Finalize the encoding, i.e. clear the buffers, write the header, etc.
			encoder.finish();

			graphModel.setVisibleView(viewSave); // undo all set filters

		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			NIOUtils.closeQuietly(outMP4);
		}
	}

	private void colorEdges(Function funcEdgeColor) {
		Partition partition = ((PartitionFunction) funcEdgeColor).getPartition();
		Column column = partition.getColumn();

		Map<Object, Integer> counts = new HashMap<>();

		for (Object value : partition.getValues()) {
			counts.put(value, 0);
		}

		for (Edge edge : graph.getEdges()) {
			Interval[] intervals = edge.getIntervals();
			if (intervals.length > 0) {

				// get count of partition attribute values
				for (Interval interval : intervals) {
					Object object = edge.getAttribute(column, interval);
					Integer count = counts.get(object);
					count++;
					counts.put(object, count);
				}

				// get partition attribute value with highest count
				Object highest = null;
				for (Object key : counts.keySet()) {
					if (highest == null) {
						highest = key;
					} else if (counts.get(key) > counts.get(highest)) {
						highest = key;
					}
				}

				// color edge with color of attribute with highest count
				edge.setColor(partition.getColor(highest));

				// reset counter for next edge
				for (Object key : counts.keySet()) {
					counts.put(key, 0);
				}
			}
		}
	}

	private void showLabel(String labelFromColumn, float nodeMinSize, Function func) {
		Transformer transformer = ((RankingFunction) func).getTransformer();
		float minSize = ((RankingNodeSizeTransformer) transformer).getMinSize();
		float maxSize = ((RankingNodeSizeTransformer) transformer).getMaxSize();
		float range = maxSize - minSize;
		nodeMinSize = (range / 100 * nodeMinSize) + minSize;

		for (Node n : graph.getNodes()) {
			String label = n.getLabel();
			if (n.size() >= nodeMinSize && (label == null || label.isEmpty())) {
				n.setLabel((String) n.getAttribute(labelFromColumn));
			}
		}
	}

	public void clearNodeColumn(String columnName) {
		Table table = graphModel.getNodeTable();
		Column column = table.getColumn(columnName);
		AttributeColumnsController attributeColumnsController = Lookup.getDefault()
				.lookup(AttributeColumnsController.class);
		if (attributeColumnsController.canClearColumnData(column)) {
			attributeColumnsController.clearColumnData(table, column);
		} else {
			for (Node n : graph.getNodes()) {
				n.setAttribute(column, null);
			}
		}
	}

	public void clearEdgeColumn(String columnName) {
		Table table = graphModel.getEdgeTable();
		Column column = table.getColumn(columnName);
		AttributeColumnsController attributeColumnsController = Lookup.getDefault()
				.lookup(AttributeColumnsController.class);
		if (attributeColumnsController.canClearColumnData(column)) {
			attributeColumnsController.clearColumnData(table, column);
		} else {
			for (Edge e : graph.getEdges()) {
				e.setAttribute(column, null);
			}
		}
	}

	public void dublicateNodeColumn(String from, String to) {
		Table table = graphModel.getNodeTable();
		Column column = table.getColumn(from);
		AttributeColumnsController attributeColumnsController = Lookup.getDefault()
				.lookup(AttributeColumnsController.class);
		attributeColumnsController.duplicateColumn(table, column, to, column.getTypeClass());
	}

	public void dublicateEdgeColumn(String from, String to) {
		Table table = graphModel.getEdgeTable();
		Column column = table.getColumn(from);
		AttributeColumnsController attributeColumnsController = Lookup.getDefault()
				.lookup(AttributeColumnsController.class);
		attributeColumnsController.duplicateColumn(table, column, to, column.getTypeClass());
	}

	public void copyEdgeColumn(String from, String to) {
		Table table = graphModel.getEdgeTable();
		AttributeColumnsController attributeColumnsController = Lookup.getDefault()
				.lookup(AttributeColumnsController.class);
		attributeColumnsController.copyColumnDataToOtherColumn(table, table.getColumn(from), table.getColumn(to));
	}

	public void logEdgeColumn(String columnName) {
		logScaledEdge = true;
		Column column = graphModel.getEdgeTable().getColumn(columnName);
		edgeSizeMin = Long.MAX_VALUE;
		edgeSizeMax = Long.MIN_VALUE;

		for (Edge e : graph.getEdges()) {
			Object a = e.getAttribute(column);
			if (a instanceof IntervalDoubleMap) {
				IntervalDoubleMap weights = (IntervalDoubleMap) a;
				for (Interval i : weights.toKeysArray()) {
					double w = weights.getDouble(i);

					if (w > edgeSizeMax) {
						edgeSizeMax = (long) w;
					}
					if (w < edgeSizeMin) {
						edgeSizeMin = (long) w;
					}

					if (w > 1) {
						w = Math.log(w);
					} else {
						w = 0;
					}
					weights.put(i, w);
				}
			}
			// TODO: instanceof Double etc.
		}
	}

	public void normalizeEdgeColumn(String columnName, double min, double max, String edgeSizeText) {
		this.edgeSizeText = edgeSizeText;
		Column column = graphModel.getEdgeTable().getColumn(columnName);
		column.setEstimator(Estimator.AVERAGE);

		double maxWeight = Double.MIN_VALUE;
		double minWeight = Double.MAX_VALUE;
		for (Edge e : graph.getEdges()) {
			Object a = e.getAttribute(column);
			if (a instanceof IntervalDoubleMap) {
				IntervalDoubleMap weights = (IntervalDoubleMap) a;
				double[] values = weights.toDoubleArray();
				for (double d : values) {
					if (d > maxWeight) {
						maxWeight = d;
					}
					if (d < minWeight) {
						minWeight = d;
					}
				}
//				for (double w : weights.toDoubleArray()) {
//					if (w > maxWeight) {
//						maxWeight = w;
//					}
//					if (w < minWeight) {
//						minWeight = w;
//					}
//				}
			}
			// TODO: instanceof Double etc.
		}

		if (!logScaledEdge) {
			edgeSizeMin = (long) minWeight;
			edgeSizeMax = (long) maxWeight;
		}

		for (Edge e : graph.getEdges()) {
			Object a = e.getAttribute(column);
			if (a instanceof IntervalDoubleMap) {
				IntervalDoubleMap weights = (IntervalDoubleMap) a;
				for (Interval i : weights.toKeysArray()) {
					double w = weights.getDouble(i);
					w = min + (((w - minWeight) * (max - min)) / (maxWeight - minWeight));
					weights.put(i, w);
				}
			}
			// TODO: instanceof Double etc.
		}
	}

	public void setLabelSize(int labelSize, float nodeLabelMinSizePercent, String font) {
		this.nodeLabelMinSizePercent = nodeLabelMinSizePercent;

		Font f = new Font(font, Font.PLAIN, labelSize);
		previewProperties.putValue(PreviewProperty.NODE_LABEL_FONT, f);
		previewProperties.putValue(PreviewProperty.NODE_LABEL_OUTLINE_SIZE, labelSize / 5);
	}

	private void setBorders() {
		for (Node n : graph.getNodes()) {
			if (n.x() < minX) {
				minX = n.x();
			}
			if (n.x() > maxX) {
				maxX = n.x();
			}
			if (n.y() < minY) {
				minY = n.y();
			}
			if (n.y() > maxY) {
				maxY = n.y();
			}
			if (n.z() < minZ) {
				minZ = n.z();
			}
			if (n.z() > maxZ) {
				maxZ = n.z();
			}
		}

//		System.out.println("x: " + minX + ":" + maxX);
//		System.out.println("y: " + minY + ":" + maxY);
//		System.out.println("z: " + minZ + ":" + maxZ);
	}

	public void setLegend(int legendXshift, int legendSize, int legendWidth, String percentageFormat) {
		this.legendSize = legendSize;

		int count = 0;
		for (Entry<String, Color> e : getlegendLabels(nodePartition, percentageFormat).entrySet()) {
			Node legendNode = graphModel.factory().newNode();
			legendNode.setX(maxX + legendXshift);
			legendNode.setY(maxY - legendSize / 2 - (count * legendSize * 1.5f));
			legendNode.setZ(minZ + ((maxZ - minZ) / 2));
			legendNode.setFixed(true);
			legendNode.setSize(legendSize);
			legendNode.setLabel(e.getKey());
			legendNode.setColor(e.getValue());
			graph.addNode(legendNode);

			legendNodes.add(legendNode);
			legendNodesLabel.add(e.getKey());

			count++;
		}

		for (Entry<String, Color> e : getlegendLabels(edgePartition, percentageFormat).entrySet()) {
			Node legendNode = graphModel.factory().newNode();
			legendNode.setX(maxX + legendXshift);
			legendNode.setY(maxY - legendSize * 1.5f - (count * legendSize * 1.5f));
			legendNode.setZ(minZ + ((maxZ - minZ) / 2));
			legendNode.setFixed(true);
			legendNode.setSize(legendSize);
			legendNode.setLabel(e.getKey());
			legendNode.setColor(e.getValue());
			graph.addNode(legendNode);

			legendNodes.add(legendNode);
			legendNodesLabel.add(e.getKey());

			count++;
		}

		Node legendEnd = graphModel.factory().newNode();
		legendEnd.setAlpha(0f);
		legendEnd.setX(maxX + legendXshift + legendWidth);
		legendEnd.setFixed(true);
		graph.addNode(legendEnd);

		List<String> texts = getlegendText();

		float x = (maxX + legendXshift + legendWidth - minX) / 2 + minX;
		float y = maxY + (texts.size() * legendSize * 1.5f);

		for (String s : texts) {
			Node text = graphModel.factory().newNode();
			text.setAlpha(0f);
			text.setX(x);
			text.setY(y);
			text.setFixed(true);
			text.setLabel(s);
			graph.addNode(text);
			legendTexts.add(text);

			y = y - legendSize * 1.5f;
		}

		y = minY - (legendSize * 1.5f);
		frames = graphModel.factory().newNode();
		frames.setAlpha(0f);
		frames.setX(x);
		frames.setY(y);
		frames.setFixed(true);
		graph.addNode(frames);

		y = y - (legendSize * 1.5f);
		frame = graphModel.factory().newNode();
		frame.setAlpha(0f);
		frame.setX(x);
		frame.setY(y);
		frame.setFixed(true);
		graph.addNode(frame);
	}

	private LinkedHashMap<String, Color> getlegendLabels(Partition partition, String percentageFormat) {
		LinkedHashMap<String, Color> labels = new LinkedHashMap<>();

		for (Object o : partition.getSortedValues()) {
			float percentage = partition.percentage(o);
			Color color = partition.getColor(o);
			String value = "";

			if (o instanceof String) {
				value = (String) o;
			}

			String label = String.format(percentageFormat, percentage) + " " + value;

			labels.put(label, color);
		}

		return labels;
	}

	private List<String> getlegendText() {
		List<String> texts = new LinkedList<>();
		texts.add("Size of nodes: " + nodeSizeText + " from " + nodeSizeMin + " to " + nodeSizeMax + ".");
		texts.add("Node label is shown if size of node is equal or more than " + nodeLabelMinSizePercent
				+ "% of maximum.");
		String logScaled;
		if (logScaledEdge) {
			logScaled = "(scaled logarithmically)";
		} else {
			logScaled = "";
		}
		texts.add("Size of edges: " + edgeSizeText + " from " + edgeSizeMin + " to " + edgeSizeMax + " " + logScaled
				+ ".");
		texts.add("Edge color: most frequent type of function in current time frame.");

		return texts;
	}

	private void refreshLegend() {
		List<String> texts = getlegendText();

		for (int i = 0; i < legendTexts.size(); i++) {
			legendTexts.get(i).setLabel(texts.get(i));
		}
		for (int i = 0; i < legendNodesLabel.size(); i++) {
			legendNodes.get(i).setLabel(legendNodesLabel.get(i));
		}
	}

	private void resizeLegendNodes() {
		for (Node n : legendNodes) {
			n.setSize(legendSize);
		}
	}

	private void getNodeSizeFrame(Column column) {
		nodeSizeMax = Long.MIN_VALUE;
		nodeSizeMin = Long.MAX_VALUE;

		for (Node n : graph.getNodes()) {
			Object a = n.getAttribute(column);
			if (a instanceof IntervalLongMap) {
				IntervalLongMap weights = (IntervalLongMap) a;
				long[] values = weights.toLongArray();
				for (long l : values) {
					if (l > nodeSizeMax) {
						nodeSizeMax = l;
					}
					if (l < nodeSizeMin) {
						nodeSizeMin = l;
					}
				}
//				for (Interval i : weights.toKeysArray()) {
//					System.out.println(i);
//					long w = weights.getLong(i);
//					if (w > nodeSizeMax) {
//						nodeSizeMax = w;
//					}
//					if (w < nodeSizeMin) {
//						nodeSizeMin = w;
//					}
//				}
			}
			// TODO: instanceof Double etc.
		}
	}

	public static void generate(String pathPrefix, String inputFile, Properties properties) {
		try {
			String input = pathPrefix + inputFile + ".gexf";
			String output = pathPrefix + inputFile + "_1.mp4";

			int maxLayoutPasses = Integer.parseInt(properties.getProperty("animationMaxLayoutPasses", "10000"));
			int height = Integer.parseInt(properties.getProperty("animationHeight", "2048"));
			int width = Integer.parseInt(properties.getProperty("animationWidth", "2048"));

			String nodeColorAttribute = "type";
			String nodeSizeAttribute = "sumTime";
			String nodeSizeText = "sum of time used for file I/O from program start to end of current time frame in nano seconds";
			int nodeSizeRangeStart = Integer.parseInt(properties.getProperty("animationNodeSizeRangeStart", "8"));
			int nodeSizeRangeEnd = Integer.parseInt(properties.getProperty("animationNodeSizeRangeEnd", "80"));

			String edgeColorAttribute = "ioType";
			String edgeSizeAttribute = "bytes";
			String edgeSizeText = "average of read and written bytes in current time frame";
			double edgeSizeRangeStart = Double
					.parseDouble(properties.getProperty("animationEdgeSizeRangeStart", "0.05"));
			double edgeSizeRangeEnd = Double.parseDouble(properties.getProperty("animationEdgeSizeRangeEnd", "2"));
			boolean logEdgeSize = true;
			float baseEdgeThickness = Float.parseFloat(properties.getProperty("animationBaseEdgeThickness", "4.0f"));

			int labelSize = Integer.parseInt(properties.getProperty("animationLabelSize", "30"));
			float showLabelNodeStartSizePercent = Float
					.parseFloat(properties.getProperty("animationShowLabelNodeStartSizePercent", "10.0f"));
			String labelFont = "Arial";
			int legendXShift = Integer.parseInt(properties.getProperty("animationLegendXShift", "300"));
			int legendNodeSize = Integer.parseInt(properties.getProperty("animationLegendNodeSize", "40"));
			int legendWidth = Integer.parseInt(properties.getProperty("animationLegendWidth", "200"));
			String percentageFormat = "%2.2f%%";

			float timeFrameRangeSizePercent = Float
					.parseFloat(properties.getProperty("animationTimeFrameRangeSizePercent", "0.5f"));
			int framesPerSecond = Integer.parseInt(properties.getProperty("animationFramesPerSecond", "4"));
			int videoLengthSeconds = Integer.parseInt(properties.getProperty("animationVideoLengthSeconds", "60"));

			String weightColumn = "Weight";
			String weightSaveColumn = "Weight_Save";
			String labelColumn = "Label";
			String labelSaveColumn = "Label_Save";

			GephiVideo gephiVideo = new GephiVideo(input, height, width, maxLayoutPasses, baseEdgeThickness);

			// nodeSizeRangeStart = (int) gephiVideo.scaleToBorders(nodeSizeRangeStart);
			// nodeSizeRangeEnd = (int) gephiVideo.scaleToBorders(nodeSizeRangeEnd);
			// edgeSizeRangeStart = (int) gephiVideo.scaleEdge(edgeSizeRangeStart);
			// edgeSizeRangeEnd = (int) gephiVideo.scaleEdge(edgeSizeRangeEnd);
			labelSize = (int) gephiVideo.scaleToBorders(labelSize);
			legendXShift = (int) gephiVideo.scaleToBorders(legendXShift);
			legendNodeSize = (int) gephiVideo.scaleToBorders(legendNodeSize);
			legendWidth = (int) gephiVideo.scaleToBorders(legendWidth);

			gephiVideo.setNodeColor(nodeColorAttribute);
			Function funcEdgeColor = gephiVideo.setEdgeColor(edgeColorAttribute);
			Function funcNodeSize = gephiVideo.setNodeSize(nodeSizeAttribute, nodeSizeRangeStart, nodeSizeRangeEnd,
					nodeSizeText);
			// save labels and clear label column
			gephiVideo.dublicateNodeColumn(labelColumn, labelSaveColumn);
			gephiVideo.clearNodeColumn(labelColumn);
			// save weight and clear weight column
			gephiVideo.dublicateEdgeColumn(weightColumn, weightSaveColumn);
			gephiVideo.clearEdgeColumn(weightColumn);
			// copy bytes to weight
			gephiVideo.copyEdgeColumn(edgeSizeAttribute, weightColumn);
			if (logEdgeSize) {
				gephiVideo.logEdgeColumn(weightColumn);
			}
			gephiVideo.normalizeEdgeColumn(weightColumn, edgeSizeRangeStart, edgeSizeRangeEnd, edgeSizeText);
			// Label properties
			gephiVideo.setLabelSize(labelSize, showLabelNodeStartSizePercent, labelFont);
			// Legend: created after color and size partitions, so legend nodes aren't part
			// of the partitions
			gephiVideo.setLegend(legendXShift, legendNodeSize, legendWidth, percentageFormat);

			// MP4
			gephiVideo.saveMP4(output, funcNodeSize, funcEdgeColor, timeFrameRangeSizePercent, framesPerSecond,
					videoLengthSeconds, labelSaveColumn);
			// PNG
			// gephiVideo.copyEdgeColumn(weightSaveColumn, weightColumn);
			gephiVideo.savePNG(pathPrefix + inputFile + "_1.png");

			output = pathPrefix + inputFile + "_2.mp4";

			nodeSizeAttribute = "sumBytes";
			nodeSizeText = "sum of read and written bytes from program start to end of current time frame";

			edgeSizeAttribute = "time";
			edgeSizeText = "average of time used for file I/O in current time frame in nano seconds";

			funcNodeSize = gephiVideo.setNodeSize(nodeSizeAttribute, nodeSizeRangeStart, nodeSizeRangeEnd,
					nodeSizeText);
			// clear label column
			gephiVideo.clearNodeColumn(labelColumn);
			// copy time to weight
			gephiVideo.copyEdgeColumn(edgeSizeAttribute, weightColumn);
			if (logEdgeSize) {
				gephiVideo.logEdgeColumn(weightColumn);
			}
			gephiVideo.normalizeEdgeColumn(weightColumn, edgeSizeRangeStart, edgeSizeRangeEnd, edgeSizeText);
			// refresh labels of legend
			gephiVideo.refreshLegend();

			// MP4
			gephiVideo.saveMP4(output, funcNodeSize, funcEdgeColor, timeFrameRangeSizePercent, framesPerSecond,
					videoLengthSeconds, labelSaveColumn);
			// PNG
			// gephiVideo.copyEdgeColumn(weightSaveColumn, weightColumn);
			gephiVideo.savePNG(pathPrefix + inputFile + "_2.png");

		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

//	public static void main(String[] args) {
//		String filePrefix = "C:\\Users\\Phil\\Documents\\Studium\\Angewandte_Informatik_Master\\Semester_3\\firefox_test154_";
//		String filePostfix = ".png";
//		int framesPerSecond = 4;
//		int secondsPerLegendEntry = 2;
//		int secondsForAnimation = 90;
//		int frames = framesPerSecond * (secondsPerLegendEntry + secondsForAnimation);
//		int leadingZeros = Integer.toString(frames).length() + 1;
//		int count = 1;
//
//		// Initialize export
//		ExportController ec = Lookup.getDefault().lookup(ExportController.class);
//		PNGExporter exporter = (PNGExporter) ec.getExporter("png"); // Get PNG exporter
//
//		// Initialize a project - and therefore a workspace
//		ProjectController pc = Lookup.getDefault().lookup(ProjectController.class);
//		pc.newProject();
//		Workspace workspace = pc.getCurrentWorkspace();
//
//		// Import file
//		ImportController importController = Lookup.getDefault().lookup(ImportController.class);
//		Container container;
//		try {
//			File file = new File(
//					"C:\\Users\\Phil\\Documents\\Studium\\Angewandte_Informatik_Master\\Semester_3\\firefox_test154.gexf");
//			container = importController.importFile(file);
//			container.getLoader().setEdgeDefault(EdgeDirectionDefault.DIRECTED); // Force DIRECTED
//		} catch (Exception ex) {
//			ex.printStackTrace();
//			return;
//		}
//
//		System.out.println("data importet");
//
//		// Append imported data to GraphAPI
//		importController.process(container, new DefaultProcessor(), workspace);
//
//		System.out.println("graph build");
//
//		// Get graph model of current workspace
//		GraphModel graphModel = Lookup.getDefault().lookup(GraphController.class).getGraphModel();
//
//		ForceAtlas2Builder forceAtlas2Builder = new ForceAtlas2Builder();
//		ForceAtlas2 forceAtlas2 = new ForceAtlas2(forceAtlas2Builder);
//
//		forceAtlas2.setGraphModel(graphModel); // must be set before layout attributes
//
//		forceAtlas2.setThreadsCount(3);
//		forceAtlas2.setJitterTolerance(1.0);
//		forceAtlas2.setBarnesHutOptimize(false);
//		forceAtlas2.setBarnesHutTheta(1.2);
//		forceAtlas2.setScalingRatio(150.0);
//		forceAtlas2.setStrongGravityMode(true);
//		forceAtlas2.setGravity(1.0);
//		forceAtlas2.setOutboundAttractionDistribution(false);
//		forceAtlas2.setLinLogMode(false);
//		forceAtlas2.setAdjustSizes(false);
//		forceAtlas2.setEdgeWeightInfluence(0.0);
//
//		// Run ForceAtlas 2 for 2000 passes - The layout always takes the current
//		// visible
//		// view
//		forceAtlas2.initAlgo();
//		for (int i = 0; i < 2000 && forceAtlas2.canAlgo(); i++) {
//			forceAtlas2.goAlgo();
//			System.out.println((i + 1) + " step of graph layout");
//		}
//		forceAtlas2.endAlgo();
//
//		float minX = Float.MAX_VALUE;
//		float maxX = Float.MIN_VALUE;
//		float minY = Float.MAX_VALUE;
//		float maxY = Float.MIN_VALUE;
//		float minZ = Float.MAX_VALUE;
//		float maxZ = Float.MIN_VALUE;
//
//		DirectedGraph graph = graphModel.getDirectedGraph();
//		for (Node n : graph.getNodes()) {
//			if (n.x() < minX) {
//				minX = n.x();
//			}
//			if (n.x() > maxX) {
//				maxX = n.x();
//			}
//			if (n.y() < minY) {
//				minY = n.y();
//			}
//			if (n.y() > maxY) {
//				maxY = n.y();
//			}
//			if (n.z() < minZ) {
//				minZ = n.z();
//			}
//			if (n.z() > maxZ) {
//				maxZ = n.z();
//			}
//		}
//
//		int legendXshift = 200;
//		int legendSize = 40;
//		int legendWidth = 200;
//		LinkedList<Node> legendNodes = new LinkedList<>();
//		Node legendStart = graphModel.factory().newNode();
//		legendStart.setAlpha(0f);
////		legend.setX(minX + ((maxX - minX) / 2));
////		legend.setY(minY + ((maxY - minY) / 2));
////		legend.setZ(minZ + ((maxZ - minZ) / 2));
//		legendStart.setX(maxX + legendXshift);
//		legendStart.setY(maxY - legendSize / 2);
//		legendStart.setZ(minZ + ((maxZ - minZ) / 2));
//		legendStart.setFixed(true);
//		legendStart.setSize(legendSize);
//		legendStart.setAttribute(graphModel.getNodeTable().getColumn("type"), "legend");
//		graph.addNode(legendStart);
//		legendNodes.add(legendStart);
//		Node legendEnd = graphModel.factory().newNode();
//		legendEnd.setAlpha(0f);
//		legendEnd.setX(maxX + legendXshift + legendWidth);
//		legendEnd.setY(maxY - legendSize / 2);
//		legendEnd.setZ(minZ + ((maxZ - minZ) / 2));
//		legendEnd.setFixed(true);
//		legendEnd.setSize(legendSize);
//		legendEnd.setAttribute(graphModel.getNodeTable().getColumn("type"), "legend");
//		graph.addNode(legendEnd);
//		legendNodes.add(legendEnd);
//		System.out.println("minX:" + minX + " < " + (minX + ((maxX - minX) / 2)) + " < maxX:" + maxX);
//		System.out.println("minY:" + minY + " < " + (minY + ((maxY - minY) / 2)) + " < maxY:" + maxY);
//		System.out.println("minZ:" + minZ + " < " + (minZ + ((maxZ - minZ) / 2)) + " < maxZ:" + maxZ);
//
//		// Partition
//		AppearanceController appearanceController = Lookup.getDefault().lookup(AppearanceController.class);
//		AppearanceModel appearanceModel = appearanceController.getModel();
////		for (Preset p : PaletteManager.getInstance().getPresets()) {
////			System.out.println(p.getName() + ": " + p.isDark() + ":" + p.gethMin() + ":" + p.gethMax() + ":"
////					+ p.getcMin() + ":" + p.getcMax() + ":" + p.getlMin() + ":" + p.getlMax());
////		}
//		Preset presetNodes = new Preset("Nodes", true, 0, 150, 1.0f, 1.6f, 0.5f, 1.5f);
//		Preset presetEdges = new Preset("Edges", true, 200, 350, 1.0f, 1.6f, 0.5f, 1.5f);
//
//		// Partition nodes
//		Column column = graphModel.getNodeTable().getColumn("type");
//		Function func = appearanceModel.getNodeFunction(graph, column, PartitionElementColorTransformer.class);
//		Partition nodeTypePartition = ((PartitionFunction) func).getPartition();
//		Palette palette = PaletteManager.getInstance().generatePalette(nodeTypePartition.size(), presetNodes);
//		nodeTypePartition.setColors(palette.getColors());
//		appearanceController.transform(func);
//		System.out.println(nodeTypePartition.size() + " node-partitions colored");
//
//		// Partition edges
//		column = graphModel.getEdgeTable().getColumn("ioType");
//		func = appearanceModel.getEdgeFunction(graph, column, PartitionElementColorTransformer.class);
//		Partition partition = ((PartitionFunction) func).getPartition();
//		palette = PaletteManager.getInstance().generatePalette(partition.size(), presetEdges);
//		partition.setColors(palette.getColors());
//		appearanceController.transform(func);
//		System.out.println(partition.size() + " edge-partitions colored");
//
//		// Ranking nodes
//		column = graphModel.getNodeTable().getColumn("sumTime");
//		column.setEstimator(Estimator.LAST);
//		func = appearanceModel.getNodeFunction(graph, column, RankingNodeSizeTransformer.class);
//		Transformer transformer = ((RankingFunction) func).getTransformer();
//		((RankingNodeSizeTransformer) transformer).setMinSize(8);
//		((RankingNodeSizeTransformer) transformer).setMaxSize(80);
//
//		// Scale global for edge scaling (see previewProoerties below)
//		// TODO
////		column = graphModel.getEdgeTable().getColumn("Weight");
////		column.setEstimator(Estimator.AVERAGE);
////		double maxWeight = 0;
////		double minWeight = 0;
////		for (Edge e : graph.getEdges()) {
////			Object a = e.getAttribute(column);
////			if (a instanceof IntervalDoubleMap) {
////				IntervalDoubleMap weights = (IntervalDoubleMap) a;
////				for (double w : weights.toDoubleArray()) {
////					if (w > maxWeight) {
////						maxWeight = w;
////					}
////				}
////			}
////		}
////		System.out.println("min weight=" + minWeight + " max weight=" + maxWeight);
//
//		// Preview configuration
//		PreviewController previewController = Lookup.getDefault().lookup(PreviewController.class);
//		PreviewModel previewModel = previewController.getModel();
//		PreviewProperties previewProperties = previewModel.getProperties();
//		previewProperties.putValue(PreviewProperty.EDGE_COLOR, new EdgeColor(EdgeColor.Mode.ORIGINAL));
//		previewProperties.putValue(PreviewProperty.EDGE_CURVED, false);
//		previewProperties.putValue(PreviewProperty.NODE_PER_NODE_OPACITY, true);
//		previewProperties.putValue(PreviewProperty.SHOW_NODE_LABELS, true);
//		previewProperties.putValue(PreviewProperty.CATEGORY_NODE_LABELS, true);
//		System.out.println("preview configuration set");
//
//		// save labels and clear label column
//		Table table = graphModel.getNodeTable();
//		column = table.getColumn("Label");
//		AttributeColumnsController attributeColumnsController = Lookup.getDefault()
//				.lookup(AttributeColumnsController.class);
//		attributeColumnsController.duplicateColumn(table, column, "Label_Save", String.class);
//		attributeColumnsController.clearColumnData(table, column);
//
//		// print hosts with legend label
//		FilterController filterController = Lookup.getDefault().lookup(FilterController.class);
//		column = graphModel.getEdgeTable().getColumn("ioType");
//		EdgePartitionFilter partitionFilter = new EdgePartitionFilter(column, appearanceModel);
//		partitionFilter.unselectAll();
//		Query query = filterController.createQuery(partitionFilter);
//		GraphView view = filterController.filter(query);
//		graphModel.setVisibleView(view);
//		column = graphModel.getNodeTable().getColumn("type");
//		setTransparencyNode(graph, 0.1f);
//		setTransparencyNode(graph, column, "host", 1);
//		legendStart.setLabel("Hosts");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print processes with legend label and edges between processes and hosts
//		partitionFilter.addPart("process");
//		query = filterController.createQuery(partitionFilter);
//		view = filterController.filter(query);
//		graphModel.setVisibleView(view);
//		setTransparencyNode(graph, column, "process", 1);
//		legendStart.setLabel("Processes");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print threads with legend label and edges between threads and processes
//		partitionFilter.addPart("thread");
//		query = filterController.createQuery(partitionFilter);
//		view = filterController.filter(query);
//		graphModel.setVisibleView(view);
//		setTransparencyNode(graph, column, "thread", 1);
//		legendStart.setLabel("Threads");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print files which aren't read or written with legend label
//		setTransparencyNode(graph, column, "file: no read, no write", 1);
//		legendStart.setLabel("Files which aren't read or written");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print functions which don't read or write with legend label
//		partitionFilter.addPart("other");
//		query = filterController.createQuery(partitionFilter);
//		view = filterController.filter(query);
//		graphModel.setVisibleView(view);
//		legendStart.setLabel("Functions which don't read or write");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print files which are only read with legend label
//		setTransparencyNode(graph, column, "file: read only", 1);
//		legendStart.setLabel("Files which are only read");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print functions which read with legend label
//		partitionFilter.addPart("read");
//		query = filterController.createQuery(partitionFilter);
//		view = filterController.filter(query);
//		graphModel.setVisibleView(view);
//		legendStart.setLabel("Functions which read");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print files which are only written with legend label
//		setTransparencyNode(graph, column, "file: write only", 1);
//		legendStart.setLabel("Files which are only written");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print functions which write with legend label
//		partitionFilter.addPart("write");
//		query = filterController.createQuery(partitionFilter);
//		view = filterController.filter(query);
//		graphModel.setVisibleView(view);
//		legendStart.setLabel("Functions which write");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// print files which are read and written with legend label
//		setTransparencyNode(graph, column, "file: read and write", 1);
//		legendStart.setLabel("Files which are read and written");
//		count = printLegend(filePrefix, filePostfix, leadingZeros, count, framesPerSecond * secondsPerLegendEntry, ec,
//				exporter);
//
//		// copy bytes to weight
//		table = graphModel.getEdgeTable();
//		attributeColumnsController = Lookup.getDefault().lookup(AttributeColumnsController.class);
//		attributeColumnsController.copyColumnDataToOtherColumn(table, table.getColumn("bytes"),
//				table.getColumn("Weight"));
//
//		previewProperties.putValue(PreviewProperty.EDGE_RESCALE_WEIGHT, true);
//		previewProperties.putValue(PreviewProperty.EDGE_RESCALE_WEIGHT_MIN, 2.0f);
//		previewProperties.putValue(PreviewProperty.EDGE_RESCALE_WEIGHT_MAX, 30.0f);
//		previewProperties.putValue(PreviewProperty.EDGE_THICKNESS, 0.5f);
//
//		// Range Filter
//		double start = graphModel.getTimeBounds().getLow();
//		double end = graphModel.getTimeBounds().getHigh();
//		double duration = end - start;
//		int rangeSizePercent = 1;
//		int framesForAnimation = framesPerSecond * secondsForAnimation;
//		double step = duration / framesForAnimation;
//		double rangeSize = duration / 100 * rangeSizePercent;
//		DynamicRangeFilter dynamicRangeFilter = new DynamicRangeFilter(graphModel);
//
//		for (double i = start; i + rangeSize < end; i += step) {
//			Range range = new Range(i, i + rangeSize);
//			dynamicRangeFilter.setRange(range);
//			query = filterController.createQuery(dynamicRangeFilter);
//			view = filterController.filter(query);
//			graphModel.setVisibleView(view); // Set the filter result as the visible view
//
//			// Refresh ranking of nodes for each export
//			appearanceController.transform(func);
//			System.out.println(partition.size() + " node-ranking sized");
//
//			// Print graph to file
//			print(new File(filePrefix + String.format("%0" + leadingZeros + "d", count) + filePostfix), ec, exporter);
//
//			System.out.println(count + " png saved");
//			count++;
//		}
//	}
//
//	private static void test() {
//		SeekableByteChannel out = null;
//		try {
//		    out = NIOUtils.writableFileChannel("/tmp/output.mp4");
//		    // for Android use: AndroidSequenceEncoder
//		    AWTSequenceEncoder encoder = new AWTSequenceEncoder(out, Rational.R(25, 1));
//		    for (...) {
//		        // Generate the image, for Android use Bitmap
//		        BufferedImage image = new BufferedImage;
//		        // Encode the image
//		        encoder.encodeImage(ImageIO.read(input));
//		    }
//		    // Finalize the encoding, i.e. clear the buffers, write the header, etc.
//		    encoder.finish();
//		} finally {
//		    NIOUtils.closeQuietly(out);
//		}
//	}
//
//	private static void print(File file, ExportController exportController, Exporter exporter) {
//		try {
//			exportController.exportFile(file, exporter);
//		} catch (IOException ex) {
//			ex.printStackTrace();
//			System.exit(1);
//		}
//	}
//
//	private static int printLegend(String filePrefix, String filePostfix, int leadingZeros, int count, int frames,
//			ExportController exportController, Exporter exporter) {
//		for (int i = 0; i < frames; i++) {
//			print(new File(filePrefix + String.format("%0" + leadingZeros + "d", count) + filePostfix),
//					exportController, exporter);
//			count++;
//		}
//
//		return count;
//	}
//
//	private static void setTransparencyNode(Graph graph, Column column, String attribute, float transparency) {
//		for (Node n : graph.getNodes()) {
//			if (n.getAttribute(column).equals(attribute)) {
//				n.setAlpha(transparency);
//			}
//		}
//	}
//
//	private static void setTransparencyNode(Graph graph, float transparency) {
//		for (Node n : graph.getNodes()) {
//			n.setAlpha(transparency);
//		}
//	}
}
