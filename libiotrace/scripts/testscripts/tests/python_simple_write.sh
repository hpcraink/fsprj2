#!/bin/bash
IOTRACE_DATABASE_IP=$1
IOTRACE_LD_PRELOAD=${test_libiotrace_so}

# define libiotrace and necessary environment variables 
LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD"

# start test
echo "    start test"
python_script='import os
import time

data_size = 1 << 30
data_buffer = bytearray(data_size)
start_time = time.time()

with open("output.txt", "wb") as file:
    file.write(data_buffer)

end_time = time.time()

os.remove("output.txt")

elapsed_time = end_time - start_time
print(f"Wrote {data_size} bytes to file in {elapsed_time:.4f} seconds.")
'
python3 -c "${python_script}"
env ${LIBIOTRACE_WITH_ENV} python3 -c "${python_script}"
