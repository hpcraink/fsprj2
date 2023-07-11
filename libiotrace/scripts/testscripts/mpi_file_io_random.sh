#!/bin/bash
iterations=1000           
timeBetweenFileCreateMin=0
timeBetweenFileCreateMax=10                
threadMin=1        
threadMax=3F        
fileCountMin=10        
fileCountMax=100        
readMin=1        
readMax=1024        
writeMin=1        
writeMax=1024

IOTRACE_LOG_NAME=mpi_file_io_test1
IOTRACE_DATABASE_IP=$1
IOTRACE_DATABASE_PORT=8086
IOTRACE_INFLUX_ORGANIZATION=hse
IOTRACE_INFLUX_BUCKET=hsebucket
IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng==
LD_PRELOAD=../../build/src/libiotrace.so
TESTNAME=../../build/test/mpi_file_io_random_openmp
TESTARGUMENTS="-v -i$iterations -c$timeBetweenFileCreateMin -C$timeBetweenFileCreateMax -t$threadMin -T$threadMax -f$fileCountMin -F$fileCountMax -r$readMin -R$readMax -w$writeMin -W$writeMax"
TMP_FILES="mpi_file_random_*"


processes=$(( ${6} * ${7} ))
echo "    process count: ${processes}"

mpirun -N ${7} -H ${5} -np $processes -x IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME -x IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP -x IOTRACE_DATABASE_PORT=$IOTRACE_DATABASE_PORT -x IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION -x IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET -x IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN -x LD_PRELOAD=$LD_PRELOAD $PWD/$TESTNAME $TESTARGUMENTS

rm -f $TMP_FILES