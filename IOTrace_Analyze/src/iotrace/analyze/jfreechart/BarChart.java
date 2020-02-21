package iotrace.analyze.jfreechart;

import java.awt.Font;
import java.awt.image.BufferedImage;
import java.io.*;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import javax.imageio.ImageIO;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryLabelPositions;
import org.jfree.chart.axis.LogarithmicAxis;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.BarRenderer;
import org.jfree.chart.renderer.category.StandardBarPainter;
import org.jfree.data.category.DefaultCategoryDataset;

public class BarChart {

	public BarChart(Map<String, List<Long>> values, List<String> legend, String title, String filePrefix,
			String fileName, int tickLabelFontSize, int labelFontSize, int legendFontSize, int titleFontSize, int width,
			int hight) {
		final DefaultCategoryDataset dataset = new DefaultCategoryDataset();
		for (Entry<String, List<Long>> e : values.entrySet()) {
			for (int i = 0; i < e.getValue().size(); i++) {
				dataset.addValue(e.getValue().get(i), legend.get(i), e.getKey());
			}
		}

		JFreeChart barChart = ChartFactory.createBarChart(title, "Category", "Score", dataset, PlotOrientation.VERTICAL,
				true, false, false);

		final LogarithmicAxis rangeAxis = new LogarithmicAxis("scaled logarithmically");
		rangeAxis.setStrictValuesFlag(false);
		barChart.getCategoryPlot().setRangeAxis(rangeAxis);

		barChart.getCategoryPlot().getDomainAxis().setCategoryLabelPositions(CategoryLabelPositions.UP_45);
		((BarRenderer) barChart.getCategoryPlot().getRenderer()).setItemMargin(0);
		((BarRenderer) barChart.getCategoryPlot().getRenderer()).setBarPainter(new StandardBarPainter());

		Font font = new Font("Arial", Font.PLAIN, tickLabelFontSize);
		barChart.getCategoryPlot().getDomainAxis().setTickLabelFont(font);
		barChart.getCategoryPlot().getRangeAxis().setTickLabelFont(font);
		font = new Font("Arial", Font.PLAIN, labelFontSize);
		barChart.getCategoryPlot().getDomainAxis().setLabelFont(font);
		barChart.getCategoryPlot().getRangeAxis().setLabelFont(font);
		font = new Font("Arial", Font.PLAIN, legendFontSize);
		barChart.getLegend().setItemFont(font);
		font = new Font("Arial", Font.PLAIN, titleFontSize);
		barChart.getTitle().setFont(font);

		BufferedImage bufferedImage = barChart.createBufferedImage(width, hight);
		try {
			ImageIO.write(bufferedImage, "png", new File(filePrefix + fileName + ".png"));
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
