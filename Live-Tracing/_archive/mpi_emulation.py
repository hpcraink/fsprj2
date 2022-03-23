from prometheus_client import CollectorRegistry, Gauge, push_to_gateway
from random import randint
import time


# Basic settings
hostname = "homer"

for x in range(10):

    #write
    functionname = "MPI_File_write"
    registry = CollectorRegistry()
    my_val= Gauge('function_data_written_bytes', 'bytes written by mpi', ["hostname", "functionname", "processid"], registry=registry)
    my_val.labels(hostname=hostname, functionname=functionname, processid="66664").set(randint(300,600))
    push_to_gateway('localhost:9091', job='iotrace.log', registry=registry)
    time.sleep(10)

    #write
    functionname = "MPI_File_read"
    registry = CollectorRegistry()
    my_val= Gauge('function_data_read_bytes', 'bytes written by mpi', ["hostname", "functionname", "processid"], registry=registry)
    my_val.labels(hostname=hostname, functionname=functionname, processid="66664").set(randint(700,999))
    push_to_gateway('localhost:9091', job='iotrace.log', registry=registry)
    time.sleep(10)




