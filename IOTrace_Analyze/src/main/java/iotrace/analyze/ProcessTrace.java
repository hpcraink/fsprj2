package iotrace.analyze;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Map.Entry;

import iotrace.analyze.FileRange.RangeType;
import iotrace.analyze.FileTraceId.IdType;

public class ProcessTrace {
	private HashMap<IdType, HashMap<String, FileTraceId>> ids = new HashMap<>();
	private TreeMap<Long, FileTraceMemoryId> memoryIdsByStartAddress = new TreeMap<>();
	private TreeMap<Long, FileTraceMemoryId> memoryIdsByEndAddress = new TreeMap<>();
	private HashMap<FileTrace, Integer> openFiles = new HashMap<>();

	private FileName fileName;

	public ProcessTrace(FileName fileName) {
		super();
		this.fileName = fileName;
	}

	public FileName getFileName() {
		return fileName;
	}

	public FileTraceMemoryId setFileTraceMemoryId(String address, String length, FileTrace fileTrace,
			FileOffset fileOffset, boolean shared) {
		FileTraceMemoryId memoryId = new FileTraceMemoryId(address, length, fileTrace, fileOffset, shared);

		HashSet<FileTraceMemoryId> overlapping = getFileTraceMemoryId(address, length);
		ArrayList<FileTraceMemoryId> newMemoryIds = new ArrayList<>();

		if (overlapping != null) {
			for (FileTraceMemoryId m : overlapping) {
				ArrayList<FileTraceMemoryId> newMemoryIdParts = m.deleteOverlappingPart(memoryId);
				newMemoryIds.addAll(newMemoryIdParts);
				memoryIdsByStartAddress.remove(m.getStartAddress());
				memoryIdsByEndAddress.remove(m.getEndAddress());
			}
		}
		newMemoryIds.add(memoryId);

		for (FileTraceMemoryId m : newMemoryIds) {
			memoryIdsByStartAddress.put(m.getStartAddress(), m);
			memoryIdsByEndAddress.put(m.getEndAddress(), m);
		}

		return memoryId;
	}

	public FileTraceMemoryId changeFileTraceMemoryId(String oldAddress, String oldLength, String newAddress,
			String newLength) {
		FileTraceMemoryId oldMemoryId = new FileTraceMemoryId(oldAddress, oldLength, null, null, false);

		if (memoryIdsByStartAddress.containsKey(oldMemoryId.getStartAddress())) {
			FileTraceMemoryId tmpMemoryId = memoryIdsByStartAddress.get(oldMemoryId.getStartAddress());

			if (tmpMemoryId.getEndAddress() == oldMemoryId.getEndAddress()) {
				memoryIdsByStartAddress.remove(oldMemoryId.getStartAddress());
				memoryIdsByEndAddress.remove(oldMemoryId.getEndAddress());

				FileTraceMemoryId newMemoryId = new FileTraceMemoryId(newAddress, newLength, tmpMemoryId.getFileTrace(),
						tmpMemoryId.getFileOffset(), tmpMemoryId.isShared());
				memoryIdsByStartAddress.put(newMemoryId.getStartAddress(), newMemoryId);
				memoryIdsByEndAddress.put(newMemoryId.getEndAddress(), newMemoryId);

				return newMemoryId;
			}
		}

		return null;
	}

	public HashSet<FileTraceMemoryId> getFileTraceMemoryId(String address, String length) {
		FileTraceMemoryId memoryId = new FileTraceMemoryId(address, length, null, null, false);
		SortedMap<Long, FileTraceMemoryId> afterStartAddress = memoryIdsByStartAddress
				.subMap(memoryId.getStartAddress(), memoryId.getEndAddress());
		SortedMap<Long, FileTraceMemoryId> beforeEndAddress = memoryIdsByEndAddress.subMap(memoryId.getStartAddress(),
				memoryId.getEndAddress());

		HashSet<FileTraceMemoryId> fileTraceMemoryIds = new HashSet<>();
		fileTraceMemoryIds.addAll(afterStartAddress.values());
		fileTraceMemoryIds.addAll(beforeEndAddress.values());

		if (!fileTraceMemoryIds.isEmpty()) {
			return fileTraceMemoryIds;
		}

		return null;
	}

	public void addOpenFile(FileTrace fileTrace) {
		if (openFiles.containsKey(fileTrace)) {
			Integer count = openFiles.get(fileTrace);
			openFiles.put(fileTrace, count + 1);
		} else {
			openFiles.put(fileTrace, 1);
		}
	}

	public void removeOpenFile(FileTrace fileTrace) {
		if (openFiles.containsKey(fileTrace)) {
			Integer count = openFiles.get(fileTrace);
			if (count > 1) {
				openFiles.put(fileTrace, count - 1);
			} else {
				openFiles.remove(fileTrace);
			}
		}
	}

	public Set<FileTrace> getOpenFiles() {
		return openFiles.keySet();
	}

	public FileTraceId getFileTraceId(IdType idType, String id) {
		if (ids.containsKey(idType)) {
			HashMap<String, FileTraceId> tmpMap = ids.get(idType);
			if (tmpMap.containsKey(id)) {
				return tmpMap.get(id);
			}
		}

		return null;
	}

	public FileTraceId setFileTraceId(IdType idType, String id, FileTrace fileTrace, FileOffset fileOffset) {
		HashMap<String, FileTraceId> tmpMap;

		if (!ids.containsKey(idType)) {
			tmpMap = new HashMap<>();
			ids.put(idType, tmpMap);
		} else {
			tmpMap = ids.get(idType);
		}

		FileTraceId fileTraceId = new FileTraceStringId(idType, id, fileTrace, fileOffset);
		tmpMap.put(id, fileTraceId);
		return fileTraceId;
	}
	
	public FileTraceId setFileTraceId(IdType idType, String id, FileTrace fileTrace, FileOffset fileOffset, RangeType rangeType) {
		HashMap<String, FileTraceId> tmpMap;

		if (!ids.containsKey(idType)) {
			tmpMap = new HashMap<>();
			ids.put(idType, tmpMap);
		} else {
			tmpMap = ids.get(idType);
		}

		FileTraceId fileTraceId = new FileTraceRequestId(idType, id, fileTrace, fileOffset, rangeType);
		tmpMap.put(id, fileTraceId);
		return fileTraceId;
	}

	public boolean containsIdType(IdType idType) {
		return ids.containsKey(idType);
	}

	public Collection<FileTraceId> getIdsFromType(IdType idType) {
		return ids.get(idType).values();
	}

	public ProcessTrace cloneWithIds(FileName fileName) {
		ProcessTrace processTrace = new ProcessTrace(fileName);

		HashMap<IdType, HashMap<String, FileTraceId>> cloneIds = new HashMap<>();

		if (ids.containsKey(IdType.DESCRIPTOR)) {
			HashMap<String, FileTraceId> cloneDescriptor = new HashMap<>();

			cloneIds.put(IdType.DESCRIPTOR, cloneDescriptor);

			for (Entry<String, FileTraceId> m : ids.get(IdType.DESCRIPTOR).entrySet()) {
				cloneDescriptor.put(m.getKey(), m.getValue());
			}
		}

		processTrace.ids = cloneIds;

		if (!memoryIdsByStartAddress.isEmpty()) {
			for (Entry<Long, FileTraceMemoryId> m : memoryIdsByStartAddress.entrySet()) {
				FileTraceMemoryId fileTraceMemoryId = m.getValue();

				if (fileTraceMemoryId.copyOnFork()) {
					processTrace.memoryIdsByStartAddress.put(fileTraceMemoryId.getStartAddress(), fileTraceMemoryId);
					processTrace.memoryIdsByEndAddress.put(fileTraceMemoryId.getEndAddress(), fileTraceMemoryId);
				}
			}
		}

		return processTrace;
	}

	public Set<FileTraceId> getOpenFileTracesForIdType(IdType idType) {
		if (containsIdType(idType)) {
			Collection<FileTraceId> idsFromType = getIdsFromType(idType);
			Set<FileTrace> openProcessFiles = getOpenFiles();
			Set<FileTraceId> fileTraceIds = new HashSet<>();

			for (FileTraceId id : idsFromType) {
				FileTrace fileTrace = id.getFileTrace();
				if (openProcessFiles.contains(fileTrace)) {
					fileTraceIds.add(id);
				}
			}

			return fileTraceIds;
		} else {
			return null;
		}
	}
}
