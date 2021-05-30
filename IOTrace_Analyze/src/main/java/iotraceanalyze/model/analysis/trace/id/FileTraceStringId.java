package iotraceanalyze.model.analysis.trace.id;

import iotraceanalyze.model.analysis.file.FileOffset;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;

public class FileTraceStringId extends FileTraceId {
	private String id;

	public FileTraceStringId(IdType idType, String id, FileTrace fileTrace, FileOffset fileOffset) {
		super(idType, fileTrace, fileOffset);
		this.id = id;
	}

	@Override
	public String toString() {
		return super.toString() + ":" + id;
	}
}
