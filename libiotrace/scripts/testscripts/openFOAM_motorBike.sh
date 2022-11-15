#!/bin/bash

END_TIME=2500

IOTRACE_LOG_NAME=openFOAM_motorBike_test1
IOTRACE_DATABASE_IP=$1
IOTRACE_DATABASE_PORT=8086
IOTRACE_INFLUX_ORGANIZATION=hse
IOTRACE_INFLUX_BUCKET=hsebucket
IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng==
MPI_PATH=$(which mpirun)
MPI_PATH="${MPI_PATH%/*}/../lib/libmpi.so"
IOTRACE_LD_PRELOAD=${MPI_PATH}:${PWD}/../../build/src/libiotrace_shared.so
IOTRACE_WHITELIST=${3}/${2}/whitelist
IOTRACE_WHITELIST_FUNCTIONS="# newline separated list of functions to trace
read
pread
pread64
write
pwrite
pwrite64
readv
writev
preadv
preadv64
pwritev
pwritev64
preadv2
preadv64v2
pwritev2
pwritev64v2
copy_file_range
getline
getdelim
fread
fwrite
fprintf
fwprintf
vfprintf
vfwprintf
fscanf
fwscanf
vfscanf
vfwscanf"

PROCESS_COUNT=$(( ${6} * ${7} ))
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
sed -i -r "s:^endTime         500;:endTime         ${END_TIME};:" "${3}/${2}/motorBike/system/controlDict"

# prepare whitelist
rm -f $IOTRACE_WHITELIST
echo -e "${IOTRACE_WHITELIST_FUNCTIONS}">$IOTRACE_WHITELIST

# prepare test case
echo "    cleanup"
cd ${3}/${2}/motorBike && ./Allclean
echo "    copy geometry to test case"
cp $WM_PROJECT_DIR/tutorials/resources/geometry/motorBike.obj.gz ${3}/${2}/motorBike/constant/geometry/

# define libiotrace and necessary environment variables for use with and without mpirun
LIBIOTRACE_WITH_ENV="IOTRACE_LOG_NAME=$IOTRACE_LOG_NAME IOTRACE_DATABASE_IP=$IOTRACE_DATABASE_IP IOTRACE_DATABASE_PORT=$IOTRACE_DATABASE_PORT IOTRACE_INFLUX_ORGANIZATION=$IOTRACE_INFLUX_ORGANIZATION IOTRACE_INFLUX_BUCKET=$IOTRACE_INFLUX_BUCKET IOTRACE_INFLUX_TOKEN=$IOTRACE_INFLUX_TOKEN IOTRACE_WHITELIST=$IOTRACE_WHITELIST LD_PRELOAD=$IOTRACE_LD_PRELOAD"
MPI_LIBIOTRACE_WITH_ENV="-x "${LIBIOTRACE_WITH_ENV// / -x }

# start test
echo "    start test"
echo "        running blockMesh"
cd ${3}/${2}/motorBike && /bin/bash -c "${LIBIOTRACE_WITH_ENV} blockMesh < /dev/null > log.blockMesh 2>&1"
echo "        running decomposePar"
cd ${3}/${2}/motorBike && /bin/bash -c "${LIBIOTRACE_WITH_ENV} decomposePar -copyZero < /dev/null > log.decomposePar 2>&1"
echo "        running snappyHexMesh"
cd ${3}/${2}/motorBike && mpirun -N ${7} -H ${5} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} snappyHexMesh -parallel < /dev/null > log.snappyHexMesh 2>&1
echo "        remove *level* data"
find . -type f -iname "*level*" -exec rm {} \;
echo "        running renumberMesh"
cd ${3}/${2}/motorBike && mpirun -N ${7} -H ${5} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} renumberMesh -parallel < /dev/null > log.renumberMesh 2>&1
echo "        running potentialFoam"
cd ${3}/${2}/motorBike && mpirun -N ${7} -H ${5} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} potentialFoam -initialiseUBCs < /dev/null > log.potentialFoam 2>&1
echo "        running simpleFoam"
cd ${3}/${2}/motorBike && mpirun -N ${7} -H ${5} -np ${PROCESS_COUNT} ${MPI_LIBIOTRACE_WITH_ENV} simpleFoam < /dev/null > log.simpleFoam 2>&1

