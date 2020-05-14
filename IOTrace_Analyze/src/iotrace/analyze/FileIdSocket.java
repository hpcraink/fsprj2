package iotrace.analyze;

public class SocketFileId implements FileComparable<SocketFileId> {
	private String hostName;
	private int sockaddrFamily;
	private String sockaddrData;
	
	public final static int FAMILY_UNBOUND_SOCKET = -1;
	public final static int FAMILY_UNNAMED_SOCKET = -2;

	public SocketFileId(String hostName, int sockaddrFamily, String sockaddrData) {
		super();
		this.hostName = hostName;
		this.sockaddrFamily = sockaddrFamily;
		this.sockaddrData = sockaddrData;
	}

	public String getHostName() {
		return hostName;
	}

	public int getSockaddrFamily() {
		return sockaddrFamily;
	}

	public String getSockaddrData() {
		return sockaddrData;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((hostName == null) ? 0 : hostName.hashCode());
		result = prime * result + ((sockaddrData == null) ? 0 : sockaddrData.hashCode());
		result = prime * result + sockaddrFamily;
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
		SocketFileId other = (SocketFileId) obj;
		if (hostName == null) {
			if (other.hostName != null)
				return false;
		} else if (!hostName.equals(other.hostName))
			return false;
		if (sockaddrData == null) {
			if (other.sockaddrData != null)
				return false;
		} else if (!sockaddrData.equals(other.sockaddrData))
			return false;
		if (sockaddrFamily != other.sockaddrFamily)
			return false;
		return true;
	}

	@Override
	public int compareTo(SocketFileId arg0) {
		int cmp = hostName.compareTo(arg0.hostName);

		if (cmp == 0) {
			if (sockaddrFamily < arg0.sockaddrFamily) {
				return -1;
			} else if (sockaddrFamily > arg0.sockaddrFamily) {
				return 1;
			} else {
				return sockaddrData.compareTo(arg0.sockaddrData);
			}
		} else {
			return cmp;
		}
	}

	@Override
	public String toString() {
		return hostName + ":" + sockaddrFamily + ":" + sockaddrData;
	}

	@Override
	public String toFileName() {
		return hostName + "_" + sockaddrFamily + "_" + sockaddrData.replace("/", "_").replace("\\", "_");
	}
}
