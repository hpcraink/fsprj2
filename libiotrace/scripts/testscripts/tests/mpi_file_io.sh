#!/bin/bash
IOTRACE_DATABASE_IP=$1
IOTRACE_WHITELIST=./whitelist
LD_PRELOAD=${test_libiotrace_so}
TESTNAME=${test_program}
TESTARGUMENTS=$test_number_of_writes

format="\t%E\t%U\t%S\t%D\t%K\t%M"

processes=$(( ${5} * ${6} ))
echo "    process count: ${processes}"

rm -f ${performance_data_file}
rm -f $IOTRACE_WHITELIST
echo -e "name\ttest\treal h:m:s\tuser CPU-seconds\tsys CPU-seconds\tavg unshared data kb\tavg data+stack+text kb\tmax kb">>${performance_data_file}

# test with libiotrace and MPI functions in whitelist
echo -e "MPI_File_write\nMPI_File_seek">$IOTRACE_WHITELIST
#echo -e "">$IOTRACE_WHITELIST
for ((i = 0; i < test_iterations; i += 1)); do
    rm -f mpi_file_io.txt
    rm -f $IOTRACE_LOG_NAME*
    /usr/bin/time -o ${performance_data_file} -a -f "mpi_file_write_active\t$i$format" mpirun -N ${6} -H ${4} -np $processes -x OMP_NUM_THREADS=$test_threads -x IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME -x IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP -x IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT -x IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION -x IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET -x IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN -x IOTRACE_WHITELIST=$IOTRACE_WHITELIST -x LD_PRELOAD=$LD_PRELOAD $TESTNAME $TESTARGUMENTS
done
echo "    with libiotrace and active wrapper: done"

# test with libiotrace and empty whitelist
echo -e "">$IOTRACE_WHITELIST
for ((i = 0; i < test_iterations; i += 1)); do
    rm -f mpi_file_io.txt
    rm -f $IOTRACE_LOG_NAME*
    /usr/bin/time -o ${performance_data_file} -a -f "all_wrapper_inactive\t$i$format" mpirun -N ${6} -H ${4} -np $processes -x OMP_NUM_THREADS=$test_threads -x IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME -x IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP -x IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT -x IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION -x IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET -x IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN -x IOTRACE_WHITELIST=$IOTRACE_WHITELIST -x LD_PRELOAD=$LD_PRELOAD $TESTNAME $TESTARGUMENTS
done
echo "    with libiotrace and inactive wrapper: done"

# test without libiotrace
for ((i = 0; i < test_iterations; i += 1)); do
    rm -f mpi_file_io.txt
    rm -f $IOTRACE_LOG_NAME*
    /usr/bin/time -o ${performance_data_file} -a -f "without_libiotrace\t$i$format" mpirun -N ${6} -H ${4} -np $processes -x OMP_NUM_THREADS=$test_threads $TESTNAME $TESTARGUMENTS
done
echo "    without libiotrace: done"

rm -f $IOTRACE_WHITELIST
rm -f mpi_file_io.txt
