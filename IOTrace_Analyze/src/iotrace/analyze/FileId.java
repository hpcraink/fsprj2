package iotrace.analyze;

public class FileId implements FileComparable<FileId> {
	FileComparable<?> fileComparable;

	public FileId(FileComparable<?> fileComparable) {
		this.fileComparable = fileComparable;
	}

	@SuppressWarnings("unchecked")
	@Override
	public int compareTo(FileId arg0) {
		if (fileComparable.getClass().equals(arg0.getClass())) {
			return fileComparable.getClass().cast(fileComparable).compareTo(arg0.getClass().cast(arg0));
		} else {
			return this.getClass().getName().compareTo(fileComparable.getClass().getName());
		}
	}

	@Override
	public String toFileName() {
		return fileComparable.toFileName();
	}

	@Override
	public int hashCode() {
		// TODO: include fileComparable.getClass().getName() in hash
		return fileComparable.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		return fileComparable.equals(obj);
	}

	@Override
	public String toString() {
		return fileComparable.toString();
	}
}
