package iotraceanalyze.model.analysis.file;

public class FileIdRegular implements FileComparable<FileIdRegular> {
	private String hostName;
	private long deviceId;
	private long fileId;

	public FileIdRegular(String hostName, long deviceId, long fileId) {
		super();
		this.hostName = hostName;
		this.deviceId = deviceId;
		this.fileId = fileId;
	}

	public String getHostName() {
		return hostName;
	}

	public long getDeviceId() {
		return deviceId;
	}

	public long getFileId() {
		return fileId;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (int) (deviceId ^ (deviceId >>> 32));
		result = prime * result + (int) (fileId ^ (fileId >>> 32));
		result = prime * result + ((hostName == null) ? 0 : hostName.hashCode());
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
		FileIdRegular other = (FileIdRegular) obj;
		if (deviceId != other.deviceId)
			return false;
		if (fileId != other.fileId)
			return false;
		if (hostName == null) {
			if (other.hostName != null)
				return false;
		} else if (!hostName.equals(other.hostName))
			return false;
		return true;
	}

	@Override
	public int compareTo(FileIdRegular arg0) {
		int cmp = hostName.compareTo(arg0.hostName);

		if (cmp == 0) {
			if (deviceId < arg0.deviceId) {
				return -1;
			} else if (deviceId > arg0.deviceId) {
				return 1;
			} else if (fileId < arg0.fileId) {
				return -1;
			} else if (fileId > arg0.fileId) {
				return 1;
			} else {
				return 0;
			}
		} else {
			return cmp;
		}
	}

	@Override
	public String toString() {
		return hostName + ":" + deviceId + ":" + fileId;
	}

	@Override
	public String toFileName() {
		return hostName + "_" + deviceId + "_" + fileId;
	}
}
