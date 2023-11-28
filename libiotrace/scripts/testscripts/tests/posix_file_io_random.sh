#!/bin/bash
IOTRACE_DATABASE_IP=$1
LD_PRELOAD=${test_libiotrace_so}
TESTNAME=${test_program}
TESTARGUMENTS="-i${test_iterations} -c$test_time_between_file_create_min -C$test_time_between_file_create_max -t$test_thread_min -T$test_thread_max -f$test_file_count_min -F$test_file_count_max -r$test_read_min -R$test_read_max -w$test_write_min -W$test_write_max"
TMP_FILES="mpi_file_random_*"


processes=$(( ${5} * ${6} ))
echo "    process count: ${processes}"

mpirun -N ${6} -H ${4} -np $processes -x IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME -x IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP -x IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT -x IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION -x IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET -x IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN -x LD_PRELOAD=$LD_PRELOAD $TESTNAME $TESTARGUMENTS

rm -f $test_tmp_files
