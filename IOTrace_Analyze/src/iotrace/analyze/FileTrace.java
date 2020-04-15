package iotrace.analyze;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.ResourceBundle;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

public class FileTrace implements Traceable, Comparable<FileTrace> {
	public static enum FileKind {
		FILE, TMPFILE, SOCKET, PIPE, MEMORY, OTHER, UNKNOWN
	}

	private FileId fileId;
	private Set<String> fileNames = new TreeSet<>();
	private FunctionEvent firstFunctionEvent = null;
	private FunctionEvent lastFunctionEvent = null;
	private TreeSet<FunctionEvent> events = new TreeSet<>();
	private BasicTrace<BasicTrace<BasicTrace<BasicTrace<FunctionTrace>>>> file;
	private FileKind kind;
	private LinkedList<FileTraceId> sendFileTraceId = new LinkedList<>();
	private TreeMap<Long, ArrayList<FunctionEvent>> fileRangesByStartPos = new TreeMap<>();
	private TreeMap<Long, ArrayList<FunctionEvent>> fileRangesByEndPos = new TreeMap<>();

	private TreeMap<Long, ArrayList<FileLock>> fileLocksByStartPos = new TreeMap<>();
	private TreeMap<Long, ArrayList<FileLock>> fileLocksByEndPos = new TreeMap<>();

	private String directoryDelimiter;

	private ResourceBundle legends;

	public FileTrace(FileId fileId, String fileName, FileKind kind, String directoryDelimiter, ResourceBundle legends) {
		super();
		this.fileId = fileId;
		fileNames.add(fileName);
		this.kind = kind;
		this.legends = legends;
		this.directoryDelimiter = directoryDelimiter;
		file = new BasicTrace<>(legends.getString("fileTraceFileTitle"));
	}

	public void sendFileTraceId(FileTraceId fileTraceId) {
		sendFileTraceId.add(fileTraceId);
	}

	public FileTraceId receiveFileTraceId() {
		return sendFileTraceId.removeFirst();
	}

	public boolean hasSendFileTraceId() {
		return !sendFileTraceId.isEmpty();
	}

	public void addFileLock(FileLock fileLock) {
		FileRange fileRange = fileLock.getFileRange();

		Set<FileLock> overlapping = getOverlappingFileLocks(fileRange);
		for (FileLock lock : overlapping) {
			if (lock.getThreadTrace().getProcessId().equals(fileLock.getThreadTrace().getProcessId())) {
				/*
				 * TODO: A single process can hold only one type of lock on a file region; if a
				 * new lock is applied to an already-locked region, then the existing lock is
				 * converted to the new lock type. (Such conversions may involve splitting,
				 * shrinking, or coalescing with an existing lock if the byte range specified by
				 * the new lock does not precisely coincide with the range of the existing
				 * lock.)
				 */
			}
		}

		ArrayList<FileLock> byStartPos;
		if (fileLocksByStartPos.containsKey(fileRange.getStartPos())) {
			byStartPos = fileLocksByStartPos.get(fileRange.getStartPos());
		} else {
			byStartPos = new ArrayList<>();
			fileLocksByStartPos.put(fileRange.getStartPos(), byStartPos);
		}
		byStartPos.add(fileLock);

		ArrayList<FileLock> byEndPos;
		if (fileLocksByEndPos.containsKey(fileRange.getEndPos())) {
			byEndPos = fileLocksByEndPos.get(fileRange.getEndPos());
		} else {
			byEndPos = new ArrayList<>();
			fileLocksByEndPos.put(fileRange.getEndPos(), byEndPos);
		}
		byEndPos.add(fileLock);
	}

	public void removeFileLock(FileLock fileLock) {
		FileRange fileRange = fileLock.getFileRange();

		// TODO overlapping locks ?
		if (fileLocksByStartPos.containsKey(fileRange.getStartPos())
				&& fileLocksByEndPos.containsKey(fileRange.getEndPos())) {
			fileLocksByStartPos.remove(fileRange.getStartPos());
			fileLocksByEndPos.remove(fileRange.getEndPos());
		}
	}

	public Set<FileLock> getFileLocks(FileRange fileRange) {
		return getOverlappingFileLocks(fileRange);
	}

	private Set<FileLock> getOverlappingFileLocks(FileRange fileRange) {
		Set<FileLock> overlapping = new TreeSet<>();

		for (ArrayList<FileLock> e : fileLocksByStartPos.subMap(fileRange.getStartPos(), fileRange.getEndPos())
				.values()) {
			overlapping.addAll(e);
		}
		for (ArrayList<FileLock> e : fileLocksByEndPos.subMap(fileRange.getStartPos(), fileRange.getEndPos())
				.values()) {
			overlapping.addAll(e);
		}

		return overlapping;
	}

	public void addEvent(FunctionEvent event) {
		events.add(event);
	}

	public TreeSet<FunctionEvent> getEvents() {
		return events;
	}

	public void processEvents() {
		if (events.isEmpty()) {
			return;
		}

		firstFunctionEvent = events.first();
		lastFunctionEvent = events.last();

		FunctionEvent before = null;
		for (FunctionEvent event : events) {
			if (before != null) {
				before.setNextInFileTrace(event);
				event.setPreviousInFileTrace(before);
			} else {
				event.setPreviousInFileTrace(null);
			}
			event.setNextInFileTrace(null);

			before = event;
		}

		groupEvents();
	}

	private void groupEvents() {
		FunctionEvent event = firstFunctionEvent;

		while (event != null) {
			String hostName = event.getThreadTrace().getHostName();
			String processId = event.getThreadTrace().getProcessId();
			String threadId = event.getThreadTrace().getThreadId();
			String functionName = event.getFunctionName();

			BasicTrace<BasicTrace<BasicTrace<FunctionTrace>>> host;
			if (!file.containsTrace(hostName)) {
				host = new BasicTrace<>(legends.getString("fileTraceHostTitle"));
				file.addTrace(hostName, host);
			} else {
				host = file.getTrace(hostName);
			}

			BasicTrace<BasicTrace<FunctionTrace>> process;
			if (!host.containsTrace(processId)) {
				process = new BasicTrace<>(legends.getString("fileTraceProcessTitle"));
				host.addTrace(processId, process);
			} else {
				process = host.getTrace(processId);
			}

			BasicTrace<FunctionTrace> thread;
			if (!process.containsTrace(threadId)) {
				thread = new BasicTrace<>(legends.getString("fileTraceThreadTitle"));
				process.addTrace(threadId, thread);
			} else {
				thread = process.getTrace(threadId);
			}

			FunctionTrace function;
			if (!thread.containsTrace(functionName)) {
				function = new FunctionTrace(legends.getString("fileTraceFunctionTitle"));
				thread.addTrace(functionName, function);
			} else {
				function = thread.getTrace(functionName);
			}

			function.addFunctionEvent(event);

			processFileRange(event);

			event = event.getNextCallInFileTrace();
		}
	}

	private void processFileRange(FunctionEvent event) {
		FileRange fileRange = event.getFileRange();

		if (fileRange != null) {
			ArrayList<FunctionEvent> events;

			if (fileRangesByStartPos.containsKey(fileRange.getStartPos())) {
				events = fileRangesByStartPos.get(fileRange.getStartPos());
			} else {
				events = new ArrayList<>();
				fileRangesByStartPos.put(fileRange.getStartPos(), events);
			}
			events.add(event);

			if (fileRangesByEndPos.containsKey(fileRange.getEndPos())) {
				events = fileRangesByEndPos.get(fileRange.getEndPos());
			} else {
				events = new ArrayList<>();
				fileRangesByEndPos.put(fileRange.getEndPos(), events);
			}
			events.add(event);

			Set<FunctionEvent> overlapping = getOverlappingFileRange(fileRange);
			if (overlapping != null) {
				for (FunctionEvent e : overlapping) {
					e.addOverlappingFileRange(event);
				}
				event.addOverlappingFileRange(overlapping);
			}
		}
	}

	private Set<FunctionEvent> getOverlappingFileRange(FileRange fileRange) {
		Set<FunctionEvent> events = new TreeSet<FunctionEvent>();

		Collection<ArrayList<FunctionEvent>> eventsByStartPos = new LinkedList<ArrayList<FunctionEvent>>();
		eventsByStartPos.addAll(fileRangesByStartPos.subMap(fileRange.getStartPos(), fileRange.getEndPos()).values());
		if (fileRangesByStartPos.containsKey(fileRange.getEndPos())) {
			eventsByStartPos.add(fileRangesByStartPos.get(fileRange.getEndPos()));
		}
		Collection<ArrayList<FunctionEvent>> eventsByEndPos = new LinkedList<ArrayList<FunctionEvent>>();
		eventsByEndPos.addAll(fileRangesByEndPos.subMap(fileRange.getStartPos(), fileRange.getEndPos()).values());
		if (fileRangesByEndPos.containsKey(fileRange.getEndPos())) {
			eventsByEndPos.add(fileRangesByEndPos.get(fileRange.getEndPos()));
		}
		for (ArrayList<FunctionEvent> e : eventsByStartPos) {
			events.addAll(e);
		}
		for (ArrayList<FunctionEvent> e : eventsByEndPos) {
			events.addAll(e);
		}

		if (!events.isEmpty()) {
			return events;
		} else {
			return null;
		}
	}

	public FileKind getKind() {
		return kind;
	}

	public Set<String> getFileNames() {
		return fileNames;
	}

	public void addFileName(String fileName) {
		fileNames.add(fileName);
	}

	public FileId getFileId() {
		return fileId;
	}

	public void setFileId(FileId fileId) {
		this.fileId = fileId;
	}

	public FunctionEvent getFirstFunctionEvent() {
		return firstFunctionEvent;
	}

	public void setFirstFunctionEvent(FunctionEvent firstFunctionEvent) {
		this.firstFunctionEvent = firstFunctionEvent;
	}

	public FunctionEvent getLastFunctionEvent() {
		return lastFunctionEvent;
	}

	public void setLastFunctionEvent(FunctionEvent lastFunctionEvent) {
		this.lastFunctionEvent = lastFunctionEvent;
	}

	@Override
	public String toString() {
		return fileId + ":" + fileNames;
	}

	@Override
	public int hashCode() {
		return fileId.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		FileTrace other = (FileTrace) obj;
		return fileId.equals(other.fileId);
	}

	@Override
	public int compareTo(FileTrace arg0) {
		return this.fileId.compareTo(arg0.fileId);
	}

	@Override
	public long getTimePeriod(ArrayList<FileKind> kinds) {
		return file.getTimePeriod(kinds);
	}

	@Override
	public long getWrapperTimePeriod(ArrayList<FileKind> kinds) {
		return file.getWrapperTimePeriod(kinds);
	}

	@Override
	public long getByteCount(ArrayList<FileKind> kinds) {
		return file.getByteCount(kinds);
	}

	@Override
	public long getFunctionCount(ArrayList<FileKind> kinds) {
		return file.getFunctionCount(kinds);
	}

	@Override
	public long getCountSubTraces(ArrayList<FileKind> kinds) {
		return file.getCountSubTraces(kinds);
	}

	@Override
	public long getOverlappingRangeCount(ArrayList<FileKind> kinds) {
		return file.getOverlappingRangeCount(kinds);
	}

	@Override
	public long getOverlappingFunctionCount(ArrayList<FileKind> kinds) {
		return file.getOverlappingFunctionCount(kinds);
	}

	@Override
	public Map<String, Traceable> getSubTraces(ArrayList<FileKind> kinds) {
		return file.getSubTraces(kinds);
	}

	@Override
	public String getTraceName() {
		return file.getTraceName();
	}

	public Map<FunctionEvent, Set<FunctionEvent>> getOverlappingFunctionEvents() {
		Map<FunctionEvent, Set<FunctionEvent>> overlapping = new HashMap<FunctionEvent, Set<FunctionEvent>>();
		FunctionEvent event = firstFunctionEvent;

		while (event != null) {
			Set<FunctionEvent> tmpOverlapping = event.getOverlappingFunctionEvents();

			if (!tmpOverlapping.isEmpty()) {
				overlapping.put(event, tmpOverlapping);
			}

			event = event.getNextCallInFileTrace();
		}

		return overlapping;
	}

	@Override
	public String getNameFromId(String id) {
		String file = (String) fileNames.toArray()[0];
		int index = file.lastIndexOf(directoryDelimiter);
		if (index > -1) {
			file = file.substring(index);
		}
		return file;
	}
}
