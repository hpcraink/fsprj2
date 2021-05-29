package iotrace.model.analysis.file;

public class FileIdWrapper implements FileComparable<FileIdWrapper> {
	private static int id = 1;

	public FileIdWrapper() {
		super();
	}

	@Override
	public int hashCode() {
		return id;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		return true;
	}

	@Override
	public int compareTo(FileIdWrapper arg0) {
		return 0;
	}

	@Override
	public String toString() {
		return "wrapper_framework";
	}

	@Override
	public String toFileName() {
		return this.toString();
	}
}
