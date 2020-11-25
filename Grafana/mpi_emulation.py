from prometheus_client import CollectorRegistry, Gauge, push_to_gateway



val = 400
hostname = "homer"
processid = "66666"
threadid = "66666"
functionname = "MPI_File_write"
registry = CollectorRegistry()
my_val= Gauge('function_data_written_bytes', 'bytes written by mpi', ["hostname", "functionname", "processid", "threadid"], registry=registry)

# usage
my_val.labels(hostname=hostname, functionname=functionname, processid=processid, threadid=threadid).set(val)
push_to_gateway('localhost:9091', job='iotrace.log', registry=registry)