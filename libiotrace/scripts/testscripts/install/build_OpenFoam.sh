#!/bin/bash

openfoam_config="$(dirname "$0")/../config/openfoam"
if [ -f ${openfoam_config} ]; then
    source ${openfoam_config}
    openfoam_config="$(dirname "$0")/../config/${openfoam_config_file}"
    if [ -f ${openfoam_config} ]; then
        source ${openfoam_config}
    else
        echo "file ${openfoam_config} does not exist"
        exit
    fi
else
    echo "file ${openfoam_config} does not exist"
    exit
fi

gcc_mpi_config="$(dirname "$0")/../config/gcc_mpi"
if [ -f ${gcc_mpi_config} ]; then
    source ${gcc_mpi_config}
else
    echo "file ${gcc_mpi_config} does not exist"
    exit
fi

scotch_config="$(dirname "$0")/../config/scotch"
if [ -f ${scotch_config} ]; then
    source ${scotch_config}
else
    echo "file ${scotch_config} does not exist"
    exit
fi

# download sources
if ! [ -d OpenFOAM-${OPENFOAM_VERSION} ]; then
    git clone --depth 1 ${OPENFOAM_CLONE_URL}
fi
if ! [ -d ThirdParty-${OPENFOAM_VERSION} ]; then
    git clone --depth 1 ${OPENFOAM_THIRDPARTY_CLONE_URL}
fi
cd ThirdParty-${OPENFOAM_VERSION}
if ! [ -d scotch_${SCOTCH_VERSION} ]; then
    git clone --depth 1 ${SCOTCH_CLONE_URL}
fi
cd ..

# check if module command is available
if command -v module &> /dev/null; then
    # load modules
    module load compiler/gnu/${GCC_VERSION}
    module load mpi/openmpi/${OPENMPI_VERSION}
fi

# source environment
source ./OpenFOAM-${OPENFOAM_VERSION}/etc/bashrc

# check if module command is available
if command -v module &> /dev/null; then
    # workaround: OpenFOAMs /etc/bashrc writes /usr/lib64 to
    # LD_LIBRARY_PATH prior to the openmpi directories; to
    # set the openmpi directories in front of the /usr/lib64
    # a reload of the openmpi module is necessary
    module purge
    module load compiler/gnu/${GCC_VERSION}
    module load mpi/openmpi/${OPENMPI_VERSION}
fi

# compile ThirdParty software
# no longer necessary: OpenFOAM Allwmake calls ThirdParty Allwmake
#cd ThirdParty-${OPENFOAM_VERSION}
#./Allwmake
#cd ..

./ThirdParty-${OPENFOAM_VERSION}/makeSCOTCH

# compile OpenFoam
cd OpenFOAM-${OPENFOAM_VERSION}
./Allwmake -j
