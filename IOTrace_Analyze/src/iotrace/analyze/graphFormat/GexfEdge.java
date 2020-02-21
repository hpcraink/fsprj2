package iotrace.analyze.graphFormat;

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

public class GexfEdge {
	private String id;
	private String sourceId;
	private String targetId;

	private List<GexfValue> values = new LinkedList<>();
	private List<GexfSpell> spells = new LinkedList<>();

	protected GexfEdge(String id, GexfNode sourceNode, GexfNode targetNode) {
		super();
		this.id = id;
		this.sourceId = sourceNode.getId();
		this.targetId = targetNode.getId();
	}

	public void createValue(GexfAttribute attribute, String value) {
		createValue(attribute, value, null, null);
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
		bufferedWriter.write(Gexf.getEdgeStart(id, sourceId, targetId));

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

		bufferedWriter.write(Gexf.getEdgeEnd());
	}
}
