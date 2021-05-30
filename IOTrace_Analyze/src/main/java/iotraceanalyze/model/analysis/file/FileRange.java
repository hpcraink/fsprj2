package iotraceanalyze.model.analysis.file;

public class FileRange {
	public enum RangeType {
		READ, WRITE
	}

	private RangeType rangeType;
	// first manipulated Byte
	private long startPos;
	// last manipulated Byte
	private long endPos;

	public FileRange(RangeType rangeType, long startPos, long endPos) {
		super();
		this.rangeType = rangeType;
		this.startPos = startPos;
		this.endPos = endPos;
	}

	public RangeType getRangeType() {
		return rangeType;
	}

	public long getStartPos() {
		return startPos;
	}

	public long getEndPos() {
		return endPos;
	}

	public long getByteCount() {
		return endPos - startPos + 1;
	}
}
