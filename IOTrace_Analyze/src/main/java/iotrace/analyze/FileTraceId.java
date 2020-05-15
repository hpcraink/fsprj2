package iotrace.analyze;

public class FileTraceId {
	public static enum IdType {
		STREAM, DESCRIPTOR, MEMORY, ASYNC, MPI_FILE, SO
	}

	public static enum IdGroup {
		HOST, PROCESS
	}

	public static enum LockMode {
		INTERNAL, BYCALLER
	}

	private IdType idType;
	private FileTrace fileTrace;
	private FileOffset fileOffset;
	private LockMode lockMode;

	public FileTraceId(IdType idType, FileTrace fileTrace, FileOffset fileOffset) {
		super();
		this.idType = idType;
		this.fileTrace = fileTrace;
		this.fileOffset = fileOffset;
		lockMode = LockMode.INTERNAL;
	}

	public LockMode getLockMode() {
		return lockMode;
	}

	public void setLockMode(LockMode lockMode) {
		this.lockMode = lockMode;
	}

	public FileTrace getFileTrace() {
		return fileTrace;
	}

	public FileOffset getFileOffset() {
		return fileOffset;
	}

	public static boolean isStandardDescriptor(String descriptor) {
		switch (Integer.parseInt(descriptor)) {
		case 0:
		case 1:
		case 2:
			return true;
		}

		return false;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((fileTrace == null) ? 0 : fileTrace.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		FileTraceId other = (FileTraceId) obj;
		if (fileTrace == null) {
			if (other.fileTrace != null)
				return false;
		} else if (!fileTrace.equals(other.fileTrace))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return idType.toString();
	}
}
