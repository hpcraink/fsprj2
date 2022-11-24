#!/bin/bash

openfoam_version=10
gcc_version="12.1"
openmpi_version="4.1"

# download sources
git clone https://github.com/OpenFOAM/OpenFOAM-${openfoam_version}.git
git clone https://github.com/OpenFOAM/ThirdParty-${openfoam_version}.git

# load dependencies
module load compiler/gnu/${gcc_version}
module load mpi/openmpi/${openmpi_version}

# source environment
source ./OpenFOAM-${openfoam_version}/etc/bashrc

# workaround: OpenFOAMs /etc/bashrc writes /usr/lib64 to
# LD_LIBRARY_PATH prior to the openmpi directories; to
# set the openmpi directories in front of the /usr/lib64
# a reload of the openmpi module is necessary
module purge
module load compiler/gnu/${gcc_version}
module load mpi/openmpi/${openmpi_version}

# compile ThirdParty software
# no linger necessary: OpenFOAM Allwmake calls ThirdParty Allwmake
#icd ThirdParty-${openfoam_version}
#./Allwmake
#cd ..

# compile OpenFoam
cd OpenFOAM-${openfoam_version}
./Allwmake -j

