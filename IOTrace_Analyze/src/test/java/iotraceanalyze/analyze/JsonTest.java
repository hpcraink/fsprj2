package iotraceanalyze.analyze;

import static org.junit.Assert.*;

import java.util.LinkedList;

import org.junit.Test;

import iotraceanalyze.model.logs.Json;

public class JsonTest {

    @Test
    public void test() {
        Json tmp = new Json(
                "{\"hostname\":\"test-VirtualBox\",\"process_id\":3471,\"thread_id\":3471,\"function_name\":\"fopen\",\"time_start\":7098154894160,\"time_end\":7098154920432,\"return_state\":\"ok\",\"file_type\":{\"stream\":\"0x7f18a6e15800\"},\"function_data\":{\"mode\":\"read_only\",\"creation\":[],\"status\":[],\"file_mode\":[\"read_by_owner\",\"write_by_owner\",\"read_by_group\",\"write_by_group\",\"read_by_others\",\"write_by_others\"],\"file_name\":\"/usr/lib/firefox/dependentlibs.list\",\"test_array\":[42,\"string\",{\"element_in_object_in_array\":\"it work's\"}]}}");
        Json json = new Json(tmp.toString());
        Json json2 = json;

        assertTrue(json.containsElement("process_id"));
        assertFalse(json.containsElement("process_id2"));

        assertEquals("3471", json.getElement("process_id"));

        assertTrue(json.containsObject("file_type"));
        assertFalse(json.containsObject("file_type2"));

        assertEquals("0x7f18a6e15800", json.getObject("file_type").getElement("stream"));

        assertTrue(json.getObject("function_data").containsValueArray("file_mode"));
        assertFalse(json.getObject("function_data").containsValueArray("file_mode2"));
        assertFalse(json.getObject("function_data").containsValueArray("creation"));

        assertEquals("read_by_owner", json.getObject("function_data").getValueArray("file_mode").get(0));

        assertTrue(json.equals(json2));
        assertFalse(json.equals(tmp));

        assertEquals("fopen", json.getValue("function_name"));
        assertEquals("/usr/lib/firefox/dependentlibs.list", json.getValue("function_data/file_name"));

        assertTrue(json.hasArrayValue("function_data/file_mode", "read_by_others"));
        assertFalse(json.hasArrayValue("function_data/file_mode", "read_by_others2"));
        assertTrue(json.getObject("function_data").hasArrayValue("file_mode", "write_by_others"));
        assertFalse(json.getObject("function_data").hasArrayValue("file_mode", "write_by_others2"));

        assertTrue(json.hasValue("function_data/file_name", "/usr/lib/firefox/dependentlibs.list"));
        assertFalse(json.hasValue("function_data/file_name", "/usr/lib/firefox/dependentlibs.list2"));
        assertTrue(json.getObject("function_data").hasValue("file_name", "/usr/lib/firefox/dependentlibs.list"));
        assertFalse(json.getObject("function_data").hasValue("file_name", "/usr/lib/firefox/dependentlibs.list2"));

        LinkedList<String> a1 = null;
        LinkedList<String> a2 = null;
        assertEquals(null, Json.mergeArrays(a1, a2));
        a2 = new LinkedList<>();
        assertEquals(0, Json.mergeArrays(a1, a2).size());
        a2.add("test");
        a2.add("test2");
        assertEquals(2, Json.mergeArrays(a1, a2).size());
        a1 = new LinkedList<>();
        a1.add("test");
        a1.add("test1");
        assertEquals(3, Json.mergeArrays(a1, a2).size());

        assertTrue(json.getObject("function_data").containsObjectArray("test_array"));
        assertTrue(json.getObject("function_data").containsValueArray("test_array"));
        assertFalse(json.getObject("function_data").containsObjectArray("test_array2"));
        assertTrue(json.hasArrayValue("function_data/test_array", "42"));
        assertTrue(json.hasArrayValue("function_data/test_array", "string"));
        assertFalse(json.hasArrayValue("function_data/test_array", "43"));
        assertTrue(json.getObject("function_data").getObjectArray("test_array").get(0).containsElement("element_in_object_in_array"));
    }

}
