package iotrace.model.analysis.trace.id;

import iotrace.model.analysis.file.FileRange.RangeType;
import iotrace.model.analysis.file.FileOffset;
import iotrace.model.analysis.trace.traceables.FileTrace;

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
