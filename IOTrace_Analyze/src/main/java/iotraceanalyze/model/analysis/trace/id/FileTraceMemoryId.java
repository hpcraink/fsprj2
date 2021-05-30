package iotraceanalyze.model.analysis.trace.id;

import iotraceanalyze.model.analysis.file.FileOffset;
import iotraceanalyze.model.analysis.trace.traceables.FileTrace;

import java.util.ArrayList;

public class FileTraceMemoryId extends FileTraceId {
	private long startAddress;
	private long endAddress;
	private boolean shared; // TODO: needed ???
	private boolean copyOnFork = true; // TODO: needed ???

	public FileTraceMemoryId(String address, String length, FileTrace fileTrace, FileOffset fileOffset,
							 boolean shared) {
		super(IdType.MEMORY, fileTrace, fileOffset);
		this.startAddress = Long.decode(address);
		this.endAddress = startAddress + Long.parseLong(length) - 1;
		this.shared = shared;
	}

	public FileTraceMemoryId(long startAddress, long endAddress, FileTrace fileTrace, FileOffset fileOffset,
			boolean shared) {
		super(IdType.MEMORY, fileTrace, fileOffset);
		this.startAddress = startAddress;
		this.endAddress = endAddress;
		this.shared = shared;
	}

	public ArrayList<FileTraceMemoryId> deleteOverlappingPart(FileTraceMemoryId toDelete) {
		ArrayList<FileTraceMemoryId> tmp = new ArrayList<>();

		if (startAddress < toDelete.startAddress) {
			tmp.add(new FileTraceMemoryId(startAddress, toDelete.startAddress - 1, getFileTrace(), getFileOffset(),
					shared));
		}
		if (endAddress > toDelete.endAddress) {
			FileOffset fileOffset;
			if (getFileOffset() != null) {
				Long offset = (toDelete.endAddress + 1) - startAddress;
				fileOffset = new FileOffset(getFileOffset().getOffset() + offset, toDelete.endAddress + 1);
			} else {
				fileOffset = null;
			}
			tmp.add(new FileTraceMemoryId(toDelete.endAddress + 1, endAddress, getFileTrace(), fileOffset, shared));
		}

		return tmp;
	}

	public boolean isShared() {
		return shared;
	}

	public boolean copyOnFork() {
		return copyOnFork;
	}

	public void setCopyOnFork(boolean copyOnFork) {
		this.copyOnFork = copyOnFork;
	}

	public long getStartAddress() {
		return startAddress;
	}

	public long getEndAddress() {
		return endAddress;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + (int) (endAddress ^ (endAddress >>> 32));
		result = prime * result + (int) (startAddress ^ (startAddress >>> 32));
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (!super.equals(obj))
			return false;
		if (getClass() != obj.getClass())
			return false;
		FileTraceMemoryId other = (FileTraceMemoryId) obj;
		if (endAddress != other.endAddress)
			return false;
		if (startAddress != other.startAddress)
			return false;
		return true;
	}

	@Override
	public String toString() {
		return super.toString() + ":" + startAddress + "-" + endAddress;
	}
}
