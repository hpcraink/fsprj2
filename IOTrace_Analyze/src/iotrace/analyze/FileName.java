package iotrace.analyze;

import java.util.ArrayList;

public class FileName {
	private static final String FILE_PREFIX = "file://";

	private String directoryDelimiter;
	private String workingDir;
	private String root;

	public FileName(String workingDir, String directoryDelimiter, String root) {
		super();
		this.directoryDelimiter = directoryDelimiter;
		this.workingDir = workingDir;
		this.root = root;
	}

	public String getDirectoryDelimiter() {
		return directoryDelimiter;
	}

	public String getWorkingDir() {
		return workingDir;
	}

	public String getRoot() {
		return root;
	}

	public String getWorkingDirFromFileName(String fileName) {
		int pos;

		if (fileName.startsWith(FILE_PREFIX)) {
			fileName = fileName.substring(7);
			pos = fileName.indexOf(directoryDelimiter);
			if (pos > 0) {
				fileName = fileName.substring(pos);
			}
		}

		pos = fileName.lastIndexOf(directoryDelimiter);

		if (pos < 0) {
			return fileName;
		}
		fileName = fileName.substring(0, pos);
		if (fileName.length() > 0) {
			return fileName;
		}
		return directoryDelimiter;
	}

	public String getHostName(String hostName) {
		return FILE_PREFIX + hostName + directoryDelimiter;
	}

	// TODO: if fileName includes a hostname, use it instead of parameter hostName
	public String getFileName(String fileName, String hostName) {
		String[] absoluteFile = createAbsolute(workingDir, fileName);
		String tmpfileName = absoluteFile[absoluteFile.length - 1];
		String path = "";
		for (int i = 1; i < absoluteFile.length - 1; i++) {
			path += directoryDelimiter + absoluteFile[i];
		}

		if (hostName == null) {
			hostName = "";
		}
		return FILE_PREFIX + hostName + root + path + directoryDelimiter + tmpfileName;
	}

	private boolean isAbsolute(String fileName) {
		if (fileName.startsWith(root + directoryDelimiter)) {
			return true;
		}
		return false;
	}

	private String[] createAbsolute(String workingDir, String relativePath) {
		String[] relativePathParts = relativePath.split(directoryDelimiter);
		if (isAbsolute(relativePath)) {
			return relativePathParts;
		}

		String[] workingDirParts = workingDir.split(directoryDelimiter);
		ArrayList<String> absolutePathParts = new ArrayList<>();
		for (String part : workingDirParts) {
			absolutePathParts.add(part);
		}

		for (String part : relativePathParts) {
			if (part.equals(".")) {
				// do nothing
			} else if (part.equals("..")) {
				int absolutePathSize = absolutePathParts.size();
				if (absolutePathSize > 1) {
					absolutePathParts.remove(absolutePathSize - 1);
				}
			} else if (part.equals("")) {
				// do nothing
			} else {
				absolutePathParts.add(part);
			}
		}

		String[] tmp = new String[absolutePathParts.size()];
		return absolutePathParts.toArray(tmp);
	}
}
