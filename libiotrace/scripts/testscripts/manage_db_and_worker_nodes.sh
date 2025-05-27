#!/bin/bash

# check arguments
if [ -z "${1+x}" ]
then
    echo "first argument must be a testscript"
    return 1
else
    # a script to run on worker nodes
    TEST_SCRIPT=${1}
fi
if [ -z "${2+x}" ]
then
    echo "second argument must be a unique test name"
    return 2
else
    # a unique test name (to differ between multiple parallel running tests)
    # e.g. 3_nodes_test_1
    TEST_NAME=${2}
fi
if [ -z "${3+x}" ]
then
    echo "third argument must be a path to a influxDB directory"
    return 3
else
    # a path to a influxDB installation
    INFLUXDB_ORIG_DIR=${3}
fi
if [ -z "${4+x}" ]
then
    echo "fourth argument must be a path to a global available directory for tmp files"
    return 4
else
    # a path to store files needed to communicate between nodes
    # (must be a directory available from all nodes)
    GLOBAL_TMP_DIR=${4}
fi
if [ -z "${5+x}" ]
then
    echo "fifth argument must be the number of processes per worker node"
    return 5
else
    # processes per worker node
    PROCESSES_PER_WORKER_NODE=${5}
fi

# remove files signaling something finished
rm -f ${GLOBAL_TMP_DIR}/finished_${TEST_NAME}
rm -f ${GLOBAL_TMP_DIR}/influx_saved_${TEST_NAME}

# read node name of influxDB node
WORKER_NODE_COUNT=$(scontrol show hostname | wc -l)
WORKER_NODE_COUNT=$(( WORKER_NODE_COUNT - 1))
echo "count of worker nodes: ${WORKER_NODE_COUNT}"
DBNODE=$(scontrol show hostname | tail -n 1)
WORK_NODES=$(scontrol show hostname | head -n ${WORKER_NODE_COUNT})
WORK_NODES=${WORK_NODES//$'\n'/:${PROCESSES_PER_WORKER_NODE},}:${PROCESSES_PER_WORKER_NODE}
echo "influxDB node: ${DBNODE}"
echo "worker nodes: ${WORK_NODES}"

export PATH="$PWD:$PATH"

# remove ib0 address
rm -f ${GLOBAL_TMP_DIR}/ib0_${TEST_NAME}

# start influxDB
#bash -c "srun -N 1 -n 1 -c ${test_processes_per_influxdb} -w $DBNODE bash -c \"export PATH="${PATH}" && ${test_base_script_dir}/runs_on_db_node.sh ${TEST_NAME} ${INFLUXDB_ORIG_DIR} ${GLOBAL_TMP_DIR}\"" &
srun -N 1 -n 1 -c ${test_processes_per_influxdb} --mem=0 -w $DBNODE bash -c "export PATH="${PATH}" && ${test_base_script_dir}/runs_on_db_node.sh ${TEST_NAME} ${INFLUXDB_ORIG_DIR} ${GLOBAL_TMP_DIR}" &

echo "influxDB starting on node ${DBNODE}"

# wait until influxDB is started
until [ -e ${GLOBAL_TMP_DIR}/ib0_${TEST_NAME} ]; do
    #echo "    waiting for influxDB ..."
    sleep 10
done

# read ib0 address written by runs_on_db_node.sh
while IFS= read -r line; do
    IB0ADDR=$line
done < ${GLOBAL_TMP_DIR}/ib0_${TEST_NAME}
echo "ib0 address of influxDB: ${IB0ADDR}"

# start worker nodes
echo "starting test $(date +"%Y-%m-%d %H:%M:%S") ..."
cd $PWD && ${TEST_SCRIPT} ${IB0ADDR} ${TEST_NAME} ${GLOBAL_TMP_DIR} ${WORK_NODES} ${WORKER_NODE_COUNT} ${PROCESSES_PER_WORKER_NODE}
echo "test finished $(date +"%Y-%m-%d %H:%M:%S")"

# signal to DB-node
echo "make backup of influxDB"
touch ${GLOBAL_TMP_DIR}/finished_${TEST_NAME}

# sleep until influxDB backup is made
until [ -e ${GLOBAL_TMP_DIR}/influx_saved_${TEST_NAME} ]; do
    sleep 10
done
echo "finished"
