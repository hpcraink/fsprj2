#!/bin/bash

# start the script with:
# sbatch -p dev_multiple -N 3 --ntasks-per-node=40 -t 00:30:00 libiotracetest.sh 1-arg 2-arg

# 1-arg:
# a unique test name (to differ between multiple parallel running tests)
# e.g. 3_nodes_test_1

# 2-arg:
# a path to a influxDB installation
# use
# wget https://dl.influxdata.com/influxdb/releases/influxdb2-2.0.6-linux-amd64.tar.gz
# and
# tar –xvzf influxdb2-2.0.6-linux-amd64.tar.gz
# to get an influxDB installation in a choosen directory
# e.g. ~/Projects/influxdb2-2.0.6-linux-amd64

# connect to InfluxDB:
# ssh -L8086:DBNODE-IP:8086 MY_LOGIN@bwunicluster.scc.kit.edu
# DBNODE-IP from file ~/Projects/scripts/ib0_${TEST_NAME}

# check arguments
if [ -z "${1+x}" ]
then
        echo "first argument must be a unique test name"
        return 1
else
        TEST_NAME=${1}
fi
if [ -z "${2+x}" ]
then
        echo "second argument must be a path to a influxDB directory"
        return 2
else
        INFLUXDB_DIR=${2}
fi

# read node name of influxDB node
DBNODE=$(scontrol show hostname | tail -n 1)
echo "influxDB node: ${DBNODE}"

export PATH="$PWD:$PATH"

# start influxDB
tmux new-session -d -s "runsondbnode" srun -N 1 -n 1 -w $DBNODE --pty /bin/bash -c "export PATH="${PATH}" && runsondbnode.sh ${TEST_NAME} ${INFLUXDB_DIR}"
echo "influxDB starting on node ${DBNODE}"
# wait until influxDB is started
sleep 30

# read ib0 address written by runsondbnode.sh
while IFS= read -r line; do
    IB0ADDR=$line
done < ~/Projects/scripts/ib0_${TEST_NAME}
echo "ib0 address of influxDB: ${IB0ADDR}"

echo "starting test ..."
cd $PWD && ./mpi_file_io.sh ${IB0ADDR} ${TEST_NAME}
