package iotrace.analyze;

import static org.junit.Assert.*;

import org.junit.Test;

import iotrace.model.analysis.file.FileName;

public class FileNameTest {

	@Test
	public void test() {
		FileName fileName = new FileName("/home/master/git/fsprj2/libiotrace/build/test", "/", "");
		assertEquals("file://host/dev/urandom", fileName.getFileName("/dev/urandom", "host"));
		assertEquals("file:///dev/urandom", fileName.getFileName("/dev/urandom", ""));
		assertEquals("file:///dev/urandom", fileName.getFileName("/dev/urandom", null));
		assertEquals("file://host/home/master/git/fsprj2/libiotrace/build/test/test", fileName.getFileName("./test", "host"));
		assertEquals("file://host/home/master/git/fsprj2/test", fileName.getFileName("../../../test", "host"));
		assertEquals("/home/master/git/fsprj2/libiotrace/build", fileName.getWorkingDirFromFileName("/home/master/git/fsprj2/libiotrace/build/test"));
		assertEquals("/test", fileName.getWorkingDirFromFileName("/test/test"));
		assertEquals("/", fileName.getWorkingDirFromFileName("/test"));
		assertEquals("/", fileName.getWorkingDirFromFileName("file:///test"));
		assertEquals("/", fileName.getWorkingDirFromFileName("file://hostName/test"));
	}

}
