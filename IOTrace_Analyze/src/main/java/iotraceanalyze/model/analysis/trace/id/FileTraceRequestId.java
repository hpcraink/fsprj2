package iotraceanalyze.model.analysis.trace.id;

import iotraceanalyze.model.analysis.file.FileRange.RangeType;
import iotraceanalyze.model.analysis.file.FileOffset;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;

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
