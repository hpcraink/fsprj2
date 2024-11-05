#!/bin/bash
IOTRACE_DATABASE_IP=$1
MPI_PATH=$(which mpirun)
MPI_PATH="${MPI_PATH%/*}/../lib64/libmpi.so"
IOTRACE_LD_PRELOAD=${MPI_PATH}:${test_libiotrace_so}
IOTRACE_WHITELIST=${3}/${2}/whitelist

PROCESS_COUNT=$(( ${5} * ${6} ))
echo "    process count: ${PROCESS_COUNT}"

echo "    activate conda env"
module load devel/miniconda/${CODE_SATURNE_MINICONDA_VERSION}
conda activate ${CODE_SATURNE_CONDA_ENV}

echo "    create test directory"
rm -rf ${3}/${2}
mkdir -p ${3}/${2}
cd ${3}/${2}

if [ ! -d ${CODE_SATURNE_STUDY_NAME} ]; then
    echo "    create directory structure"
    ${base_path}/${CODE_SATURNE_INSTALL_DIR}/${CODE_SATURNE_SOURCE_DIR}/arch/Linux_x86_64/bin/code_saturne create --study=${CODE_SATURNE_STUDY_NAME} -c ${CODE_SATURNE_CASE_NAME}
    cp ${base_path}/${SALOME_INSTALL_DIR}/${CODE_SATURNE_MESH_FILE} ${CODE_SATURNE_STUDY_NAME}/MESH/
    rm ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    cp ${test_base_script_dir}/install/setup_s64_n1_c1_1p0.xml ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<mesh name=\"mesh_xtend_z_64_64_64_1p0.med\"/>;<mesh name=\"${CODE_SATURNE_MESH_FILE}\"/>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<iterations>200</iterations>;<iterations>100</iterations>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<read_method>stdio serial</read_method>;<read_method>${CODE_SATURNE_IO_METHOD}</read_method>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<write_method>stdio serial</write_method>;<write_method>${CODE_SATURNE_IO_METHOD}</write_method>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
fi

echo "    create result directory"
rm -rf ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/RESU/${CODE_SATURNE_RESULT_DIR}
export OMP_NUM_THREADS=${test_threads_per_process}
cd ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}
${base_path}/${CODE_SATURNE_INSTALL_DIR}/${CODE_SATURNE_SOURCE_DIR}/arch/Linux_x86_64/bin/code_saturne run --stage --initialize --nprocs ${PROCESS_COUNT} --id ${CODE_SATURNE_RESULT_DIR}
cd RESU/${CODE_SATURNE_RESULT_DIR}

#####################################################

# prepare whitelist
rm -f $IOTRACE_WHITELIST
echo -e "${IOTRACE_WHITELIST_FUNCTIONS}">$IOTRACE_WHITELIST

# define libiotrace and necessary environment variables for use with and without mpirun
LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD"
MPI_LIBIOTRACE_WITH_ENV="-x "${LIBIOTRACE_WITH_ENV// / -x }

# start test
echo "    start test"
echo "        running cs_solver"
mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} --bind-to socket ./cs_solver --trace --logp --mpi

