package iotrace.analyze.visualization.graphFormat;

import java.io.BufferedWriter;
import java.io.IOException;

import iotrace.analyze.visualization.graphFormat.Gexf.AttrType;

public class GexfAttribute {
	private String id;
	private String title;
	private AttrType attrType;
	
	protected GexfAttribute(String id, String title, AttrType attrType) {
		super();
		this.id = id;
		this.title = title;
		this.attrType = attrType;
	}
	
	protected String getId() {
		return id;
	}
	
	protected void write(BufferedWriter bufferedWriter) throws IOException {
		bufferedWriter.write(Gexf.getAttribute(id, title, attrType));
	}
}
