#!/bin/bash
IOTRACE_DATABASE_IP=$1
MPI_PATH=$(which mpirun)
MPI_PATH="${MPI_PATH%/*}/../lib64/libmpi.so"
#IOTRACE_LD_PRELOAD=${MPI_PATH}:${test_libiotrace_so}
IOTRACE_LD_PRELOAD=${test_libiotrace_so}
SCRATCH_TEST_DIR=${3}/${2}
IOTRACE_WHITELIST=${SCRATCH_TEST_DIR}/whitelist
format="\t%E\t%U\t%S\t%D\t%K\t%M"

PROCESS_COUNT=$(( ${5} * ${6} ))
echo "    process count: ${PROCESS_COUNT}"

echo "    activate conda env"
module load devel/miniconda/${CODE_SATURNE_MINICONDA_VERSION}
conda activate ${CODE_SATURNE_CONDA_ENV}

echo "    create test directory"
rm -rf ${SCRATCH_TEST_DIR}
mkdir -p ${SCRATCH_TEST_DIR}
cd ${SCRATCH_TEST_DIR}

if [ ! -d ${CODE_SATURNE_STUDY_NAME} ]; then
    echo "    create directory structure"
    ${base_path}/${CODE_SATURNE_INSTALL_DIR}/${CODE_SATURNE_SOURCE_DIR}/arch/Linux_x86_64/bin/code_saturne create --study=${CODE_SATURNE_STUDY_NAME} -c ${CODE_SATURNE_CASE_NAME}
    cp ${base_path}/${SALOME_INSTALL_DIR}/${CODE_SATURNE_MESH_FILE} ${CODE_SATURNE_STUDY_NAME}/MESH/
    rm ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    cp ${test_base_script_dir}/install/setup_s64_n1_c1_1p0.xml ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<mesh name=\"mesh_xtend_z_64_64_64_1p0.med\"/>;<mesh name=\"${CODE_SATURNE_MESH_FILE}\"/>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<iterations>200</iterations>;<iterations>100</iterations>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<read_method>stdio serial</read_method>;<rank_step>${CODE_SATURNE_IO_RANK_STEP}</rank_step><read_method>${CODE_SATURNE_IO_METHOD}</read_method>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<write_method>stdio serial</write_method>;<write_method>${CODE_SATURNE_IO_METHOD}</write_method>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
    sed -i "s;<min_block_size>131072</min_block_size>;<min_block_size>${CODE_SATURNE_IO_STEP_SIZE}</min_block_size>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml 
    sed -i "s;<iterations>100</iterations>;<iterations>${CODE_SATURNE_ITERATIONS}</iterations>;g" ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/DATA/setup.xml
fi

echo "    create result directory"
rm -rf ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}/RESU/${CODE_SATURNE_RESULT_DIR}
export OMP_NUM_THREADS=${test_threads_per_process}
cd ${CODE_SATURNE_STUDY_NAME}/${CODE_SATURNE_CASE_NAME}
${base_path}/${CODE_SATURNE_INSTALL_DIR}/${CODE_SATURNE_SOURCE_DIR}/arch/Linux_x86_64/bin/code_saturne run --param DATA/setup.xml --stage --initialize --nprocs ${PROCESS_COUNT} --id ${CODE_SATURNE_RESULT_DIR}
cd RESU/${CODE_SATURNE_RESULT_DIR}

# library search path for dependencies (sourced from generated script)
source <(grep "export LD_LIBRARY_PATH" run_solver)

#####################################################

# prepare output file
rm -f ${performance_data_file}
echo -e "name\ttest\treal h:m:s\tuser CPU-seconds\tsys CPU-seconds\tavg unshared data kb\tavg data+stack+text kb\tmax kb">>${performance_data_file}

# prepare whitelist
rm -f $IOTRACE_WHITELIST

# define libiotrace and necessary environment variables for use with and without mpirun
LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD"
MPI_LIBIOTRACE_WITH_ENV="-x "${LIBIOTRACE_WITH_ENV// / -x }

echo "    start test"

# test with libiotrace and MPI functions in whitelist
echo -e "${IOTRACE_WHITELIST_FUNCTIONS}">$IOTRACE_WHITELIST
for ((i = 0; i < test_iterations; i += 1)); do
    echo "        running cs_solver"
    echo "            mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi"
    /usr/bin/time -o ${performance_data_file} -a -f "mpi_active\t$i$format" mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi
done
echo "    with libiotrace and active wrapper: done"

# test with libiotrace and empty whitelist
echo -e "">$IOTRACE_WHITELIST
for ((i = 0; i < test_iterations; i += 1)); do
    echo "        running cs_solver"
    echo "            mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi"
    /usr/bin/time -o ${performance_data_file} -a -f "all_inactive\t$i$format" mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi
done
echo "    with libiotrace and inactive wrapper: done"

# test without libiotrace
for ((i = 0; i < test_iterations; i += 1)); do
    echo "        running cs_solver"
    echo "            mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ./cs_solver --trace --logp --mpi"
    /usr/bin/time -o ${performance_data_file} -a -f "without_libiotrace\t$i$format" mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ./cs_solver --trace --logp --mpi
done
echo "    without libiotrace: done"
