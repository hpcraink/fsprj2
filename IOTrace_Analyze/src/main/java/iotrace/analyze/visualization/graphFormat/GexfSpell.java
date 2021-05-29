package iotrace.analyze.visualization.graphFormat;

import java.io.BufferedWriter;
import java.io.IOException;

public class GexfSpell {
	private String start;
	private String end;

	protected GexfSpell(String start, String end) {
		super();
		this.start = start;
		this.end = end;
	}

	protected void write(BufferedWriter bufferedWriter) throws IOException {
		bufferedWriter.write(Gexf.getSpell(start, end));
	}
}
