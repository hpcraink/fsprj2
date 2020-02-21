package iotrace.analyze;

import java.util.ArrayList;
import java.util.Set;
import java.util.TreeSet;

import iotrace.analyze.FileRange.RangeType;
import iotrace.analyze.FileTrace.FileKind;

public class FunctionEvent implements Comparable<FunctionEvent> {
	public static enum FunctionKind {
		CALL, RETURN
	}

	private static long count = 0;

	private FileTraceId fileTraceId;
	private FileTraceId fileTraceId2 = null;
	private ThreadTrace threadTrace;
	private FileRange fileRange;
	private FileLock fileLock;
	private Set<FunctionEvent> overlappingFileRange = new TreeSet<>();
	private Set<FunctionEvent> overlappingFunctionCall = new TreeSet<>();
	private boolean overlappingFunctionCallEvaluated = false;
	private FunctionKind kind;
	private FunctionEvent belongsTo = null;
	private ArrayList<FunctionEvent> theSame = null;

	private FunctionEvent previousInThreadTrace = null;
	private FunctionEvent nextInThreadTrace = null;

	private FunctionEvent previousInFileTrace = null;
	private FunctionEvent nextInFileTrace = null;

	private long id;
	private long time;
	private long wrapperTime;
	private String functionName;
	private Json json;
	private boolean error;

	public FunctionEvent(FileTraceId fileTraceId, long time, long wrapperTime, String functionName,
			ThreadTrace threadTrace, Json json, FunctionKind kind, boolean error) {
		super();
		id = count++;
		this.fileTraceId = fileTraceId;
		this.time = time;
		this.wrapperTime = wrapperTime;
		this.functionName = functionName;
		this.threadTrace = threadTrace;
		this.json = json;
		this.kind = kind;
		this.error = error;
	}

	public String getId() {
		return String.valueOf(id);
	}

	public void setFileTraceId2(FileTraceId fileTraceId) {
		this.fileTraceId2 = fileTraceId;
	}

	public String printFileTraceId() {
		String tmp = "";

		if (fileTraceId != null) {
			tmp += fileTraceId;
		}
		if (fileTraceId2 != null) {
			tmp += "," + fileTraceId2;
		}

		return tmp;
	}

	public FileOffset getFileOffset() {
		return fileTraceId.getFileOffset();
	}

	public FileRange getFileRange() {
		return fileRange;
	}

	public void addOverlappingFileRange(FunctionEvent event) {
		overlappingFileRange.add(event);
	}

	public void addOverlappingFileRange(Set<FunctionEvent> events) {
		overlappingFileRange.addAll(events);
	}

	public Set<FunctionEvent> getOverlappingFileRange() {
		return overlappingFileRange;
	}

	public boolean hasOverlappingFileRange() {
		return !overlappingFileRange.isEmpty();
	}

	public void addSame(ArrayList<FunctionEvent> functionEvents) {
		theSame = functionEvents;
	}

	public ArrayList<FunctionEvent> getSame() {
		return theSame;
	}

	public boolean hasSame() {
		if (theSame != null && !theSame.isEmpty()) {
			return true;
		}
		return false;
	}

	public Set<FunctionEvent> getOverlappingFunctionEvents() {
		if (overlappingFunctionCallEvaluated) {
			return overlappingFunctionCall;
		}

		FunctionEvent start;
		FunctionEvent end;

		if (this.kind == FunctionKind.CALL) {
			start = this;
			end = belongsTo;
		} else {
			start = belongsTo;
			end = this;
		}

		start = start.nextInFileTrace;
		while (start != end) {
			if (isNextCall(start)) {
				overlappingFunctionCall.add(start);
			}
			start = start.nextInFileTrace;
		}

		overlappingFunctionCallEvaluated = true;
		return overlappingFunctionCall;
	}

	public boolean hasOverlappingFunction() {
		if (!overlappingFunctionCallEvaluated) {
			getOverlappingFunctionEvents();
		}
		return !overlappingFunctionCall.isEmpty();
	}

	public FunctionEvent getBelongsTo() {
		return belongsTo;
	}

	public void setBelongsTo(FunctionEvent belongsTo) {
		this.belongsTo = belongsTo;
	}

	public FunctionEvent getPreviousInThreadTrace() {
		return previousInThreadTrace;
	}

	public void setPreviousInThreadTrace(FunctionEvent previousInThreadTrace) {
		this.previousInThreadTrace = previousInThreadTrace;
	}

	public FunctionEvent getNextInThreadTrace() {
		return nextInThreadTrace;
	}

	public void setNextInThreadTrace(FunctionEvent nextInThreadTrace) {
		this.nextInThreadTrace = nextInThreadTrace;
	}

	public FunctionEvent getPreviousInFileTrace() {
		return previousInFileTrace;
	}

	public void setPreviousInFileTrace(FunctionEvent previousInFileTrace) {
		this.previousInFileTrace = previousInFileTrace;
	}

	public FunctionEvent getNextInFileTrace() {
		return nextInFileTrace;
	}

	public void setNextInFileTrace(FunctionEvent nextInFileTrace) {
		this.nextInFileTrace = nextInFileTrace;
	}

	public ThreadTrace getThreadTrace() {
		return threadTrace;
	}

	public FileTrace getFileTrace() {
		if (fileTraceId != null) {
			return fileTraceId.getFileTrace();
		} else {
			return null;
		}
	}

	public long getTime() {
		return time;
	}

	public long getWrapperTime() {
		return wrapperTime;
	}

	public String getFunctionName() {
		return functionName;
	}

	public long getFunctionByteCount() {
		if (fileRange != null) {
			return fileRange.getByteCount();
		} else {
			return 0;
		}
	}

	public long getFunctionTimePeriod() {
		long startTime;
		long endTime;

		if (kind == FunctionKind.CALL) {
			startTime = time;
			endTime = belongsTo.time;
		} else {
			startTime = belongsTo.time;
			endTime = time;
		}

		return endTime - startTime;
	}

	public long getWrapperTimePeriod() {
		long startTime;
		long endTime;

		if (kind == FunctionKind.CALL) {
			startTime = wrapperTime;
			endTime = belongsTo.wrapperTime;
		} else {
			startTime = belongsTo.wrapperTime;
			endTime = wrapperTime;
		}

		return endTime - startTime - getFunctionTimePeriod();
	}

	public long getStartTime() {
		if (kind == FunctionKind.CALL) {
			return time;
		} else {
			return belongsTo.time;
		}
	}

	public long getEndTime() {
		if (kind == FunctionKind.RETURN) {
			return time;
		} else {
			return belongsTo.time;
		}
	}

	public Json getJson() {
		return json;
	}

	public FunctionKind getKind() {
		return kind;
	}

	public boolean hasError() {
		return error;
	}

	private boolean isNextCall(FunctionEvent event) {
		if (event != null && event.getKind() == FunctionKind.CALL && (event.getJson() != json)) {
			return true;
		}
		return false;
	}

	public FunctionEvent getNextCallInThreadTrace() {
		FunctionEvent event = nextInThreadTrace;

		while (event != null) {
			if (isNextCall(event)) {
				return event;
			}
			event = event.getNextInThreadTrace();
		}

		return null;
	}

	public FunctionEvent getNextCallInFileTrace() {
		FunctionEvent event = nextInFileTrace;

		while (event != null) {
			if (isNextCall(event)) {
				return event;
			}
			event = event.getNextInFileTrace();
		}

		return null;
	}

	@Override
	public int compareTo(FunctionEvent arg0) {
		int cmp = Long.valueOf(time).compareTo(arg0.time);

		if (cmp == 0) {
			if (id < arg0.id) {
				return -1;
			} else if (id > arg0.id) {
				return 1;
			} else {
				return 0;
			}
		}

		return cmp;
	}

	public void setRange(RangeType rangeType, String bytes, String offset, boolean useFileOffset) {
		if (useFileOffset) {
			setRange(rangeType, bytes);
		} else {
			setRange(rangeType, bytes, offset);
		}
	}

	public void setRange(RangeType rangeType, String bytes) {
		if (fileTraceId != null && fileTraceId.getFileTrace().getKind() != FileKind.OTHER) {
			long longBytes = Long.parseLong(bytes);

			if (longBytes > 0) {
				FileOffset fileOffset = fileTraceId.getFileOffset();
				long startPos = 0;
				long endPos;

				// if no offset is given use 0 as default (e.g. for eventfd descriptors)
				if (fileOffset != null) {
					startPos = fileOffset.getOffset();
				}
				endPos = startPos + longBytes - 1;

				if (fileOffset != null) {
					fileOffset.setOffset(endPos + 1);
				}

				setRange(new FileRange(rangeType, startPos, endPos));
			}
		}
	}

	public void setRangeCheckError(RangeType rangeType, String bytes, String offset) {
		if (!error) {
			setRange(rangeType, bytes, offset);
		}
	}

	public void setRange(RangeType rangeType, String bytes, String offset) {
		long longOffset = Long.parseLong(offset);

		setRange(rangeType, bytes, longOffset);
	}

	public void setRange(RangeType rangeType, String bytes, long offset) {
		long longBytes = Long.parseLong(bytes);

		if (longBytes > 0) {
			long startPos = offset;
			long endPos = startPos + longBytes - 1;

			setRange(new FileRange(rangeType, startPos, endPos));
		}
	}

	public void setRangeFromAddress(RangeType rangeType, String bytes, String address) {
		// TODO: shared ???
		if (fileTraceId.getFileTrace() != null && fileTraceId.getFileTrace().getKind() != FileKind.OTHER) {
			long longAddress = Long.decode(address);

			FunctionEvent event = this;
			if (theSame != null) {
				for (FunctionEvent e : theSame) {
					if (e.getFileOffset().getAddress() < event.getFileOffset().getAddress()) {
						event = e;
					}
				}
			}

			longAddress -= event.getFileOffset().getAddress();

			setRange(rangeType, bytes, event.getFileOffset().getOffset() + longAddress);
		}
	}

	public void setOffset(String offset) {
		long longOffset = Long.parseLong(offset);

		if (fileTraceId != null && fileTraceId.getFileOffset() != null) {
			FileOffset fileOffset = fileTraceId.getFileOffset();
			fileOffset.deleteUngetOffset();
			fileOffset.setOffset(longOffset);
		}
	}

	public void changeOffset(String offset) {
		long longOffset = Long.parseLong(offset);

		if (fileTraceId != null) {
			FileOffset fileOffset = fileTraceId.getFileOffset();
			fileOffset.deleteUngetOffset();
			fileOffset.setOffset(fileOffset.getOffset() + longOffset);
		}
	}

	public void rewindOffset() {
		if (fileTraceId != null) {
			FileOffset fileOffset = fileTraceId.getFileOffset();
			fileOffset.deleteUngetOffset();
			fileOffset.setOffset(0);
		}
	}

	public void setUngetOffset(String offset) {
		long longOffset = Long.parseLong(offset);

		if (fileTraceId != null) {
			FileOffset fileOffset = fileTraceId.getFileOffset();
			fileOffset.setUngetOffset(longOffset);
		}
	}

	public void deleteUngetOffset() {
		if (fileTraceId != null) {
			FileOffset fileOffset = fileTraceId.getFileOffset();
			fileOffset.deleteUngetOffset();
		}
	}

	private void setRange(FileRange fileRange) {
		this.fileRange = fileRange;

		if (theSame != null) {
			for (FunctionEvent e : theSame) {
				e.fileRange = fileRange;
			}
		}
	}

	public void changeLockState(iotrace.analyze.FileTraceId.LockMode lockState) {
		fileTraceId.setLockMode(lockState);
	}

	public void changeLockState() {
		// don't change lock state
	}

	public void setLock() {
		switch (fileTraceId.getLockMode()) {
		case INTERNAL:
			break;
		case BYCALLER:
			break;
		default:
			break;
		}
	}

	public void setThreadLock() {
		// don't add a thread lock
	}
}
