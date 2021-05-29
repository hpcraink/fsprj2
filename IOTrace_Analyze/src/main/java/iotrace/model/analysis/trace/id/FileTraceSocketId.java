package iotrace.model.analysis.trace.id;

import iotrace.model.analysis.file.FileOffset;
import iotrace.model.analysis.trace.traceables.FileTrace;

public class FileTraceSocketId extends FileTraceStringId {
	private boolean connectionBased;

	public FileTraceSocketId(IdType idType, String id, FileTrace fileTrace, FileOffset fileOffset, boolean connectionBased) {
		super(idType, id, fileTrace, fileOffset);
		this.connectionBased = connectionBased;
	}

	public boolean isConnectionBased() {
		return connectionBased;
	}
}
