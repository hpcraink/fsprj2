#!/bin/bash

OPENFOAM_VERSION=10
GCC_VERSION="12.1"
OPENMPI_VERSION="4.1"


# download sources
git clone --depth 1 https://github.com/OpenFOAM/OpenFOAM-${OPENFOAM_VERSION}.git
git clone --depth 1 https://github.com/OpenFOAM/ThirdParty-${OPENFOAM_VERSION}.git

# load modules
if command -v module &> /dev/null; then
    module load compiler/gnu/${GCC_VERSION}
    module load mpi/openmpi/${OPENMPI_VERSION}
fi

# source environment
source ./OpenFOAM-${OPENFOAM_VERSION}/etc/bashrc

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
# no linger necessary: OpenFOAM Allwmake calls ThirdParty Allwmake
#icd ThirdParty-${OPENFOAM_VERSION}
#./Allwmake
#cd ..

# compile OpenFoam
cd OpenFOAM-${OPENFOAM_VERSION}
./Allwmake -j
