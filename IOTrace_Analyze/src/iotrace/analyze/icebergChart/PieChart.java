package iotrace.analyze.icebergChart;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import javax.imageio.ImageIO;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.frontangle.ichart.pie.MultiLevelPieChart;
import com.frontangle.ichart.pie.Segment;

import iotrace.analyze.KeyValueTreeNode;

public class PieChart {
	private static final Logger logger = LogManager.getLogger(PieChart.class);

	private static List<Color> colors = new LinkedList<>();
	static {
		colors.add(new Color(0, 190, 0));
		colors.add(new Color(190, 100, 0));
		colors.add(new Color(190, 0, 0));
		colors.add(new Color(0, 190, 100));
		colors.add(new Color(0, 100, 190));
	}
	private int nextColor = 0;

	public PieChart(KeyValueTreeNode node, String title, String filePrefix, String fileName, boolean reduce,
			boolean showReducedLabels, int initialWidth, int incrementWidth, int border, int fontSize, int titleSize) {

		ArrayList<Segment> values = new ArrayList<Segment>();
		String tmpName = "";
		if (reduce) {
			while (node.getChildren().size() <= 1) {
				if (showReducedLabels) {
					tmpName += getName(node) + " ";
				}
				if (node.getChildren().isEmpty()) {
					// tree has no further elements
					// => don't print empty pie chart
					return;
				}
				node = node.getChildren().first();
			}
			tmpName += getName(node);
		} else {
			tmpName = getName(node);
		}
		Segment center = getCenterSegmentFromNode(node, tmpName);
		values.add(center);

		int level = getSubSegmentsFromNode(node, 1, center);

		MultiLevelPieChart chart = new MultiLevelPieChart(values, title);

		int sizeWidth = initialWidth + (incrementWidth * 2 * level) + (border * 7);
		int sizeHeight = initialWidth + (incrementWidth * 2 * level) + (border * 4);
		chart.setSize(sizeWidth, sizeHeight);

		chart.topOffset = border;
		chart.leftOffset = border;
		chart.rightOffset = border * 4;
		chart.bottomOffset = border;
		chart.initialWidth = initialWidth;
		chart.incrementWidth = incrementWidth;
		chart.textColorOnSlices = Color.BLACK;
		chart.textFontOnSlices = new Font("Arial", Font.PLAIN, fontSize);
		chart.setTitleFont(new Font("Arial", Font.PLAIN, titleSize));

		BufferedImage bufferedImage = new BufferedImage(chart.getWidth(), chart.getHeight(),
				BufferedImage.TYPE_INT_RGB);
		Graphics2D graphics2d = bufferedImage.createGraphics();
		graphics2d.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_ON);
		graphics2d.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		chart.paint(graphics2d);
		graphics2d.dispose();
		try {
			ImageIO.write(bufferedImage, "png", new File(filePrefix + fileName + ".png"));
		} catch (IOException e) {
			logger.error("Exception during write of png", e);
		}
	}

	private int getSubSegmentsFromNode(KeyValueTreeNode node, int level, Segment parent) {
		int newLevel = level;
		boolean first = true;
		Color c = null;

		for (KeyValueTreeNode subNode : node.getChildren()) {
			Segment s = getSegmentFromNode(subNode, level, parent);
			if (!parent.color.equals(Color.WHITE)) {
				if (first) {
					first = false;
					c = s.color;
				} else {
					c = shift(c);
					s.color = c;
				}
			}
			if (subNode.hasRealChildren()) {
				newLevel = getSubSegmentsFromNode(subNode, level + 1, s);
			}
		}

		return newLevel;
	}

	private Color nextColor() {
		Color c = colors.get(nextColor);
		nextColor++;
		if (nextColor >= colors.size()) {
			nextColor = 0;
		}

		return c;
	}

	private String getName(KeyValueTreeNode node) {
		String name = node.getKey();

		return name + " (" + format(node.getValue()) + ")";
	}

	private String format(double d) {
		return String.format("%2.2f%%", d);
	}

	private Segment getCenterSegmentFromNode(KeyValueTreeNode node, String name) {
		return new Segment(node.getValue(), name, Color.WHITE);
	}

	private Segment getSegmentFromNode(KeyValueTreeNode node, int level, Segment parent) {
		Color color;
		if (parent.color.equals(Color.WHITE)) {
			color = nextColor();
		} else {
			color = brighten(parent.color);
		}
		Segment s = new Segment(level, parent, node.getValue(), getName(node), color);
		parent.children.add(s);
		return s;
	}

	private static Color brighten(Color c) {
		int add = 20;
		return new Color(Math.min(c.getRed() + add, 255), Math.min(c.getGreen() + add, 255),
				Math.min(c.getBlue() + add, 255));
	}

	private static Color shift(Color c) {
		int add = 20;
		int red = c.getRed();
		int green = c.getGreen();
		int blue = c.getBlue();

		if (c.getRed() <= c.getGreen() && c.getRed() <= c.getBlue()) {
			red = Math.min(c.getRed() + add, 255);
		}
		if (c.getGreen() <= c.getRed() && c.getGreen() <= c.getBlue()) {
			green = Math.min(c.getGreen() + add, 255);
		}
		if (c.getBlue() <= c.getRed() && c.getBlue() <= c.getGreen()) {
			blue = Math.min(c.getBlue() + add, 255);
		}

		return new Color(red, green, blue);
	}
}
