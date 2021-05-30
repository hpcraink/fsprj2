package iotraceanalyze.model.analysis.file;

public class FileOffset {
	long offset;
	long ungetOffset = 0;
	long address;

	public FileOffset(long offset) {
		this.offset = offset;
	}

	public FileOffset(long offset, long address) {
		this.offset = offset;
		this.address = address;
	}

	public long getOffset() {
		return offset - ungetOffset;
	}

	public long getAddress() {
		return address;
	}

	public void setOffset(long offset) {
		if (ungetOffset < 0) {
			if (this.offset - offset > ungetOffset) {
				ungetOffset += offset - this.offset;
			} else {
				ungetOffset = 0;
				this.offset = offset;
			}
		} else {
			this.offset = offset;
		}
	}

	public void setUngetOffset(long offset) {
		if (offset < 0) {
			ungetOffset -= offset;
		}
	}

	public void deleteUngetOffset() {
		ungetOffset = 0;
	}
}
