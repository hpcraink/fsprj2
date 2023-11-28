#!/bin/bash
IOTRACE_DATABASE_IP=$1
MPI_PATH=$(which mpirun)
MPI_PATH="${MPI_PATH%/*}/../lib64/libmpi.so"
IOTRACE_LD_PRELOAD=${MPI_PATH}:${test_libiotrace_so}
IOTRACE_WHITELIST=${3}/${2}/whitelist

PROCESS_COUNT=$(( ${5} * ${6} ))
echo "    process count: ${PROCESS_COUNT}"

# source tutorial run functions
echo "    source tutorial run functions"
. $WM_PROJECT_DIR/bin/tools/RunFunctions

# copy testcase to test directory
echo "    copy test case"
rm -rf ${3}/${2}
mkdir -p ${3}/${2}
cp -r $WM_PROJECT_DIR/tutorials/incompressible/pisoFoam/LES/motorBike/motorBike/ ${3}/${2}/

# patch test case
echo "    patch test case (use ${PROCESS_COUNT} subdomains with ${PROCESS_COUNT} processes)"
sed -i -r "s:^numberOfSubdomains  8;:numberOfSubdomains  ${PROCESS_COUNT};:" "${3}/${2}/motorBike/system/decomposeParDict"
case ${PROCESS_COUNT} in
    4)
        SUBDOMAINS="2 2 1"
        ;;
    8)
        SUBDOMAINS="4 2 1"
                ;;
    16)
        SUBDOMAINS="8 2 1"
                ;;
    32)
        SUBDOMAINS="8 4 1"
        ;;
    64)
        SUBDOMAINS="4 4 4"
                ;;
    *)
        echo "unknown subdomain count: ${PROCESS_COUNT}"
        exit 1
        ;;
esac
sed -i -r "s:^    n               \(4 2 1\);:    n               (${SUBDOMAINS});:" "${3}/${2}/motorBike/system/decomposeParDict"
sed -i -r "s:^endTime         500;:endTime         ${test_end_time};:" "${3}/${2}/motorBike/system/controlDict"

# prepare whitelist
rm -f $IOTRACE_WHITELIST
echo -e "${IOTRACE_WHITELIST_FUNCTIONS}">$IOTRACE_WHITELIST

# prepare test case
echo "    cleanup"
cd ${3}/${2}/motorBike && ./Allclean
echo "    copy geometry to test case"
cp $WM_PROJECT_DIR/tutorials/resources/geometry/motorBike.obj.gz ${3}/${2}/motorBike/constant/geometry/

# define libiotrace and necessary environment variables for use with and without mpirun
LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_INFLUX_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD"
MPI_LIBIOTRACE_WITH_ENV="-x "${LIBIOTRACE_WITH_ENV// / -x }

# start test
echo "    start test"
echo "        running blockMesh"
cd ${3}/${2}/motorBike && /bin/bash -c "${LIBIOTRACE_WITH_ENV} blockMesh < /dev/null > log.blockMesh 2>&1"
echo "        running decomposePar"
cd ${3}/${2}/motorBike && /bin/bash -c "${LIBIOTRACE_WITH_ENV} decomposePar -copyZero < /dev/null > log.decomposePar 2>&1"
echo "        running snappyHexMesh"
cd ${3}/${2}/motorBike && mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} snappyHexMesh -parallel < /dev/null > log.snappyHexMesh 2>&1
echo "        remove *level* data"
find . -type f -iname "*level*" -exec rm {} \;
echo "        running renumberMesh"
cd ${3}/${2}/motorBike && mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} renumberMesh -parallel < /dev/null > log.renumberMesh 2>&1
echo "        running potentialFoam"
cd ${3}/${2}/motorBike && mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} potentialFoam -initialiseUBCs < /dev/null > log.potentialFoam 2>&1
echo "        running simpleFoam"
cd ${3}/${2}/motorBike && mpirun -N ${6} -H ${4} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} simpleFoam < /dev/null > log.simpleFoam 2>&1

