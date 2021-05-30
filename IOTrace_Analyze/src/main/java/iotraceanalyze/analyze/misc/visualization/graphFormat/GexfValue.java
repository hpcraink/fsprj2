package iotraceanalyze.analyze.misc.visualization.graphFormat;

import java.io.BufferedWriter;
import java.io.IOException;

public class GexfValue {
	private String id;
	private String value;
	private String start;
	private String end;
	
	protected GexfValue(GexfAttribute attribute, String value, String start, String end) {
		super();
		this.id = attribute.getId();
		this.value = value;
		this.start = start;
		this.end = end;
	}

	protected void write(BufferedWriter bufferedWriter) throws IOException {
		bufferedWriter.write(Gexf.getAttValue(id, value, start, end));
	}
}
