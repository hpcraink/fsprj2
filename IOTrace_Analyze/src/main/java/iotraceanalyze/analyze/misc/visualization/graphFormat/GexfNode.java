package iotraceanalyze.analyze.misc.visualization.graphFormat;

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

public class GexfNode {
	private String id;
	private String label;

	private List<GexfValue> values = new LinkedList<>();
	private List<GexfSpell> spells = new LinkedList<>();

	protected GexfNode(String id, String label) {
		super();
		this.id = id;
		this.label = label;
	}

	protected String getId() {
		return id;
	}

	public void createValue(GexfAttribute attribute, String value) {
		createValue(attribute, value, null, null);
	}

	public void createValue(GexfAttribute attribute, String value, String start) {
		createValue(attribute, value, start, null);
	}
	
	public void createValue(GexfAttribute attribute, String value, String start, String end) {
		GexfValue gexfValue = new GexfValue(attribute, value, start, end);

		values.add(gexfValue);
	}

	public void createSpell(String start, String end) {
		GexfSpell gexfSpell = new GexfSpell(start, end);

		spells.add(gexfSpell);
	}

	protected void write(BufferedWriter bufferedWriter) throws IOException {
		bufferedWriter.write(Gexf.getNodeStart(id, label));

		bufferedWriter.write(Gexf.getAttValuesStart());
		for (GexfValue v : values) {
			v.write(bufferedWriter);
		}
		bufferedWriter.write(Gexf.getAttValuesEnd());

		bufferedWriter.write(Gexf.getSpellsStart());
		for (GexfSpell s : spells) {
			s.write(bufferedWriter);
		}
		bufferedWriter.write(Gexf.getSpellsEnd());

		bufferedWriter.write(Gexf.getNodeEnd());
	}
}
