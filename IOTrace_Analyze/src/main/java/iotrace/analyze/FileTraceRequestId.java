package iotrace.analyze;

import iotrace.analyze.FileRange.RangeType;

public class FileTraceRequestId extends FileTraceStringId {

	private RangeType rangeType;
	
	public FileTraceRequestId(IdType idType, String id, FileTrace fileTrace, FileOffset fileOffset, RangeType rangeType) {
		super(idType, id, fileTrace, fileOffset);
		this.rangeType = rangeType;
	
	}

	public RangeType getRangeType() {
		return rangeType;
	}

		

}
