#!/bin/bash
IOTRACE_DATABASE_IP=$1
MPI_PATH=$(which mpirun)
MPI_PATH="${MPI_PATH%/*}/../lib64/libmpi.so"
IOTRACE_LD_PRELOAD=${MPI_PATH}:${test_libiotrace_so}
IOTRACE_WHITELIST=${3}/${2}/whitelist

format="\t%E\t%U\t%S\t%D\t%K\t%M"

PROCESS_COUNT=$(( ${5} * ${6} ))
echo "    process count: ${PROCESS_COUNT}"

# source tutorial run functions
echo "    source tutorial run functions"
. $WM_PROJECT_DIR/bin/tools/RunFunctions

# copy testcase to test directory
echo "    copy test case"
rm -rf ${3}/${2}
mkdir -p ${3}/${2}
ln -s ${test_source_1} ${3}/${2}/
ln -s ${test_source_constant} ${3}/${2}/
cp -r ${test_source_system} ${3}/${2}/

# patch test case
echo "    patch test case (use ${PROCESS_COUNT} subdomains with ${PROCESS_COUNT} processes)"
sed -i -r "s:^numberOfSubdomains 280;:numberOfSubdomains  ${PROCESS_COUNT};:" "${3}/${2}/system/decomposeParDict"
case ${PROCESS_COUNT} in
    4)
        SUBDOMAINS="2 2 1"
        ;;
    20)
        SUBDOMAINS="20 1 1"
        ;;
    40)
        SUBDOMAINS="40 1 1"
        ;;
    80)
        SUBDOMAINS="80 1 1"
        ;;
    160)
        SUBDOMAINS="160 1 1"
        ;;
    168)
        SUBDOMAINS="168 1 1"
        ;;
    320)
        SUBDOMAINS="320 1 1"
        ;;
    *)
        echo "unknown subdomain count: ${PROCESS_COUNT}"
        exit 1
        ;;
esac
sed -i -r "s:^    n               \(168 1 1\);:    n               (${SUBDOMAINS});:" "${3}/${2}/system/decomposeParDict"
sed -i -r "s:^SimuEnd\t\t100;:SimuEnd         ${test_end_time};:" "${3}/${2}/system/controlDict"

# prepare whitelist
rm -f $IOTRACE_WHITELIST
# prepare output file
rm -f ${performance_data_file}
echo -e "name\ttest\treal h:m:s\tuser CPU-seconds\tsys CPU-seconds\tavg unshared data kb\tavg data+stack+text kb\tmax kb">>${performance_data_file}

# define libiotrace and necessary environment variables for use with and without mpirun
LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD"
MPI_LIBIOTRACE_WITH_ENV="-x "${LIBIOTRACE_WITH_ENV// / -x }

# start test
echo "    start test"
echo "        running redistributePar"
#cd ${3}/${2}/ && mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} redistributePar -decompose -parallel < /dev/null > log.redistributePar 2>&1
cd ${3}/${2}/ && mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} redistributePar -decompose -parallel < /dev/null > log.redistributePar 2>&1

# test with libiotrace and POSIX functions in whitelist
echo -e "${IOTRACE_WHITELIST_FUNCTIONS}">$IOTRACE_WHITELIST
for ((i = 0; i < test_iterations; i += 1)); do
    echo "        running buoyantPimpleFoam"
    cd ${3}/${2}/ && /usr/bin/time -o ${performance_data_file} -a -f "posix_active\t$i$format" mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} buoyantPimpleFoam -parallel < /dev/null > log.buoyantPimpleFoam 2>&1
    rm -rf ${3}/${2}/processor[0-9]*/1.*
    rm -f log.*
    rm -rf postProcessing
done
echo "    with libiotrace and active wrapper: done"

# test with libiotrace and empty whitelist
echo -e "">$IOTRACE_WHITELIST
for ((i = 0; i < test_iterations; i += 1)); do
    echo "        running buoyantPimpleFoam"
    cd ${3}/${2}/ && /usr/bin/time -o ${performance_data_file} -a -f "all_inactive\t$i$format" mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} buoyantPimpleFoam -parallel < /dev/null > log.buoyantPimpleFoam 2>&1
    rm -rf ${3}/${2}/processor[0-9]*/1.*
    rm -f log.*
    rm -rf postProcessing
done
echo "    with libiotrace and inactive wrapper: done"

# test without libiotrace
for ((i = 0; i < test_iterations; i += 1)); do
    echo "        running buoyantPimpleFoam"
    cd ${3}/${2}/ && /usr/bin/time -o ${performance_data_file} -a -f "without_libiotrace\t$i$format" mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} buoyantPimpleFoam -parallel < /dev/null > log.buoyantPimpleFoam 2>&1
    rm -rf ${3}/${2}/processor[0-9]*/1.*
    rm -f log.*
    rm -rf postProcessing
done
echo "    without libiotrace: done"

rm -f $IOTRACE_WHITELIST
