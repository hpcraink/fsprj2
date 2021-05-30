package iotraceanalyze.analyze.misc.visualization.graphFormat;

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import iotraceanalyze.analyze.misc.visualization.graphFormat.Gexf.AttrType;
import iotraceanalyze.analyze.misc.visualization.graphFormat.Gexf.ClassType;
import iotraceanalyze.analyze.misc.visualization.graphFormat.Gexf.ModeType;

public class GexfAttributes {
	private ClassType classType;
	private ModeType modeType;
	
	private List<GexfAttribute> attributes = new LinkedList<>();
	
	protected GexfAttributes(ClassType classType, ModeType modeType) {
		super();
		this.classType = classType;
		this.modeType = modeType;
	}

	public GexfAttribute createAttribute(String id, String title, AttrType attrType) {
		GexfAttribute gexfAttribute = new GexfAttribute(id, title, attrType);
		
		attributes.add(gexfAttribute);
		
		return gexfAttribute;
	}
	
	protected void write(BufferedWriter bufferedWriter) throws IOException {
		bufferedWriter.write(Gexf.getAttributesStart(classType, modeType));
		
		for (GexfAttribute a : attributes) {
			a.write(bufferedWriter);
		}
		
		bufferedWriter.write(Gexf.getAttributesEnd());
	}
}
