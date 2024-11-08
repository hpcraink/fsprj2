#!/bin/bash

code_saturne_config="$(dirname "$0")/../config/code_saturne"
if [ -f ${code_saturne_config} ]; then
    source ${code_saturne_config}
else
    echo "file ${code_saturne_config} does not exist"
    exit 1
fi

echo "load modules..."
if command -v module &> /dev/null; then
    module purge
    module load compiler/gnu/${CODE_SATURNE_GNU_VERSION}
    module load mpi/openmpi/${CODE_SATURNE_OPENMPI_VERSION}
    module load devel/miniconda/${CODE_SATURNE_MINICONDA_VERSION}
fi

if [ ! -f ${CODE_SATURNE_SOURCE_FILE} ]; then
    echo "download source file..."
    curl -o ${CODE_SATURNE_SOURCE_FILE} ${CODE_SATURNE_SOURCE_URL}
fi

if [ ! -d ${CODE_SATURNE_SOURCE_DIR} ]; then
    echo "extracting source file..."
    tar -xf ${CODE_SATURNE_SOURCE_FILE}
    echo "    configure..."
    cd ${CODE_SATURNE_SOURCE_DIR}
    aclocal
    autoheader
    automake --force-missing --add-missing
    autoconf
    echo "    patch install script for med download..."
    sed -i "s;url=\"https://files.salome-platform.org/Salome/medfile/%s\");url=\"${CODE_SATURNE_MEDFILE_URL}%s\");g" ./install_saturne.py
    cd -
    cd ${CODE_SATURNE_SOURCE_DIR}/libple/
    mkdir build-aux
    aclocal
    autoheader
    automake --force-missing --add-missing
    autoconf
    cd -
fi

if [ ! -d ${CODE_SATURNE_INSTALL_DIR} ]; then
    echo "build code saturne..."
    mkdir ${CODE_SATURNE_INSTALL_DIR}

    echo "    create conda env..."
    conda create -y --prefix ${CODE_SATURNE_CONDA_ENV}
    conda activate ${CODE_SATURNE_CONDA_ENV}
    echo "        install pyqt5..."
    conda install -y -c conda-forge pyqt==${CODE_SATURNE_PYQT_VERSION} python=${CODE_SATURNE_PYTHON_VERSION}

    echo "    generate setup..."
    cd ${CODE_SATURNE_INSTALL_DIR}
    CODE_SATURNE_INSTALL_PREFIX=$(pwd)
    ./../${CODE_SATURNE_SOURCE_DIR}/install_saturne.py
    cd -

    echo "    patch setup..."
    CODE_SATURNE_GCC=$(which gcc)
    sed -i "s;compC\s\+/usr/bin/cc;compC     ${CODE_SATURNE_GCC};g" ${CODE_SATURNE_INSTALL_DIR}/setup
    CODE_SATURNE_F95=$(which gfortran
    sed -i "s;compF\s\+/usr/bin/f95;compF    ${CODE_SATURNE_F95};g" ${CODE_SATURNE_INSTALL_DIR}/setup)
    #sed -i 's/download\s\+yes/download  no/g' ${CODE_SATURNE_INSTALL_DIR}/setup
    sed -i "s;prefix\s\+\/.*;prefix    ${CODE_SATURNE_INSTALL_PREFIX};g" ${CODE_SATURNE_INSTALL_DIR}/setup
    #/home/es/es_es/es_pkoester/code_saturne/8.2.0
    sed -i "s/hdf5\s\+no\s\+no\s\+None/hdf5       yes   yes      None/g" ${CODE_SATURNE_INSTALL_DIR}/setup
    sed -i "s/med\s\+no\s\+no\s\+None/med        yes   yes      None/g" ${CODE_SATURNE_INSTALL_DIR}/setup

    echo "    build from setup..."
    cd ${CODE_SATURNE_INSTALL_DIR}
    ./../${CODE_SATURNE_SOURCE_DIR}/install_saturne.py
    cd -
fi

if command -v module &> /dev/null; then
    module purge
fi
