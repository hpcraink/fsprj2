#!/bin/bash
iterations=1000
processes=80
threads=1
number_of_writes=1000
IOTRACE_LOG_NAME=mpi_file_io_test1
IOTRACE_DATABASE_IP=$1
IOTRACE_DATABASE_PORT=8086
IOTRACE_INFLUX_ORGANIZATION=hse
IOTRACE_INFLUX_BUCKET=hsebucket
IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng==
IOTRACE_WHITELIST=./whitelist
LD_PRELOAD=../../build/src/libiotrace_shared.so
TESTNAME=../../build/test/mpi_file_io_openmp
TESTARGUMENTS=$number_of_writes

file=performance_test
format="\t%E\t%U\t%S\t%D\t%K\t%M"

module purge
module load compiler/gnu/11.2
module load mpi/openmpi/4.1.2

rm $file
rm -f $IOTRACE_WHITELIST
echo -e "name\ttest\treal h:m:s\tuser CPU-seconds\tsys CPU-seconds\tavg unshared data kb\tavg data+stack+text kb\tmax kb">>$file
echo -e "MPI_File_write\nMPI_File_seek">$IOTRACE_WHITELIST
#echo -e "">$IOTRACE_WHITELIST
for ((i = 0; i < iterations; i += 1)); do

rm -f mpi_file_io.txt
rm -f $IOTRACE_LOG_NAME*
/usr/bin/time -o $file -a -f "mpi_file_write_active\t$i$format" mpirun -np $processes -x OMP_NUM_THREADS=$threads -x IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME -x IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP -x IOTRACE_DATABASE_PORT=$IOTRACE_DATABASE_PORT -x IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION -x IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET -x IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN -x IOTRACE_WHITELIST=$IOTRACE_WHITELIST -x LD_PRELOAD=$LD_PRELOAD $PWD/$TESTNAME $TESTARGUMENTS

done
echo -e "">$IOTRACE_WHITELIST
for ((i = 0; i < iterations; i += 1)); do

rm -f mpi_file_io.txt
rm -f $IOTRACE_LOG_NAME*
/usr/bin/time -o $file -a -f "all_wrapper_inactive\t$i$format" mpirun -np $processes -x OMP_NUM_THREADS=$threads -x IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME -x IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP -x IOTRACE_DATABASE_PORT=$IOTRACE_DATABASE_PORT -x IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION -x IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET -x IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN -x IOTRACE_WHITELIST=$IOTRACE_WHITELIST -x LD_PRELOAD=$LD_PRELOAD $PWD/$TESTNAME $TESTARGUMENTS

done

for ((i = 0; i < iterations; i += 1)); do

rm -f mpi_file_io.txt
rm -f $IOTRACE_LOG_NAME*
/usr/bin/time -o $file -a -f "without_libiotrace\t$i$format" mpirun -np $processes -x OMP_NUM_THREADS=$threads $PWD/$TESTNAME $TESTARGUMENTS

done
