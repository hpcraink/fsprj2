#!/bin/bash
IOTRACE_DATABASE_IP=$1
MPI_PATH=$(which mpirun)
MPI_PATH="${MPI_PATH%/*}/../lib64/libmpi.so"
#IOTRACE_LD_PRELOAD=${MPI_PATH}:${test_libiotrace_so}
IOTRACE_LD_PRELOAD=${test_libiotrace_so}
SCRATCH_TEST_DIR=${3}/${2}
IOTRACE_WHITELIST=${SCRATCH_TEST_DIR}/whitelist

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
#echo "SLURM_HOSTFILE: ${SLURM_HOSTFILE}"
#echo "SLURM_JOB_CPUS_PER_NODE: ${SLURM_JOB_CPUS_PER_NODE}"
#echo "SLURM_JOB_NODELIST: ${SLURM_JOB_NODELIST}"
#echo "SLURM_JOB_NUM_NODES: ${SLURM_JOB_NUM_NODES}"
#echo "SLURM_NPROCS: ${SLURM_NPROCS}"
#echo "SLURM_CPUS_PER_TASK: ${SLURM_CPUS_PER_TASK}"
#echo "SLURM_NTASKS: ${SLURM_NTASKS}"
#echo "SLURM_STEP_NUM_TASKS: ${SLURM_STEP_NUM_TASKS}"
#echo "OMPI_COMM_WORLD_RANK: ${OMPI_COMM_WORLD_RANK}"
#echo "SLURM_SRUN_COMM_HOST: ${SLURM_SRUN_COMM_HOST}"
${base_path}/${CODE_SATURNE_INSTALL_DIR}/${CODE_SATURNE_SOURCE_DIR}/arch/Linux_x86_64/bin/code_saturne run --param DATA/setup.xml --stage --initialize --nprocs ${PROCESS_COUNT} --id ${CODE_SATURNE_RESULT_DIR}
cd RESU/${CODE_SATURNE_RESULT_DIR}

# use original solver (not the new compiled solver)
#cp ${base_path}/${CODE_SATURNE_INSTALL_DIR}/${CODE_SATURNE_SOURCE_DIR}/arch/Linux_x86_64/libexec/code_saturne/cs_solver .
#ldd cs_solver

# library search path for dependencies (sourced from generated script)
source <(grep "export LD_LIBRARY_PATH" run_solver)

#####################################################

# prepare whitelist
rm -f $IOTRACE_WHITELIST
echo -e "${IOTRACE_WHITELIST_FUNCTIONS}">$IOTRACE_WHITELIST

# define libiotrace and necessary environment variables for use with and without mpirun
#LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_DEBUG=all LD_PRELOAD=$IOTRACE_LD_PRELOAD"
LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD"
MPI_LIBIOTRACE_WITH_ENV="-x "${LIBIOTRACE_WITH_ENV// / -x }

# start test
echo "    start test"
echo "        running cs_solver"
#mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} --bind-to socket ./cs_solver --trace --logp --mpi
echo "            mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi"
mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi
#mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi &
#while true
#do
#    echo "---------------------------------------------------------------------------------------------------------------------------------------------------------------------"
#    #pstree -pl
#    ##pstree -plT
#    #lsof /home/es/es_es/es_pkoester/test_code_saturne/libiotrace_build/src/libiotrace.so
#    #lsof /opt/bwhpc/common/mpi/openmpi/4.1.6-gnu-14.1/lib64/libmpi.so.40
#    #ps | grep "cs_solver"
#    #ps | grep "cs_solver" | sed 's/^[[:blank:]]*//g'
#    #ps | grep "cs_solver" | sed 's/^[[:blank:]]*//g' | cut -d' ' -f1 -s
#    ps | grep "cs_solver" | sed 's/^[[:blank:]]*//g' | cut -d' ' -f1 -s | while read -r line ; do
#        lsof -p $line
#    done
#    sleep 1
#done
#IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi
#env ${LIBIOTRACE_WITH_ENV} ./cs_solver --trace --logp --mpi
#which ./cs_solver
#mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} --bind-to socket ltrace -f -S -c ./cs_solver --trace --logp --mpi
#mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} --bind-to socket ltrace -f -S ./cs_solver --trace --logp --mpi
#mpiexec -N ${6} -H ${4} -n ${PROCESS_COUNT} --bind-to socket strace -f ./cs_solver --trace --logp --mpi
