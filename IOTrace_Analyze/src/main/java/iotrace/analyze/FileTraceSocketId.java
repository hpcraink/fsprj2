package iotrace.analyze;

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
