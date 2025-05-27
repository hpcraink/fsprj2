#!/bin/bash

code_saturne_config="$(dirname "$0")/../config/code_saturne"
if [ -f ${code_saturne_config} ]; then
    source ${code_saturne_config}
else
    echo "file ${code_saturne_config} does not exist"
    exit 1
fi

gcc_mpi_config="$(dirname "$0")/../config/gcc_mpi"
if [ -f ${gcc_mpi_config} ]; then
    source ${gcc_mpi_config}
else
    echo "file ${gcc_mpi_config} does not exist"
    exit 2
fi

echo "load modules..."
if command -v module &> /dev/null; then
    module purge
    module load compiler/gnu/${test_gcc_version}
    module load mpi/openmpi/${test_mpi_version}
    module load devel/miniforge/${CODE_SATURNE_MINIFORGE_VERSION}
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
    ./sbin/bootstrap
    #autoreconf -iv
    #aclocal
    #autoheader
    #automake --force-missing --add-missing
    #autoconf
    
    echo "    patch install script for med download..."
    sed -i "s;url=\"https://files.salome-platform.org/Salome/medfile/%s\");url=\"${CODE_SATURNE_MEDFILE_URL}%s\");g" ./install_saturne.py
    sed -i "s;url=\"https://files.salome-platform.org/Salome/other/%s\");url=\"${CODE_SATURNE_MEDFILE_URL}%s\");g" ./install_saturne.py
    sed -i "s;url=\"http://files.salome-platform.org/Salome/other/%s\");url=\"${CODE_SATURNE_MEDFILE_URL}%s\");g" ./install_saturne.py
    sed -i "s;version=\"4.0.0\",;version=\"4.1.1\",;g" ./install_saturne.py
    sed -i "s;archive=\"med-4.0.0.tar.gz\",;archive=\"med-4.1.1.tar.gz\",;g" ./install_saturne.py

    echo "    patch hdf5 version"
    sed -i "s;version=\"1.10.6\",;version=\"1.10.7\",;g" ./install_saturne.py
    sed -i "s;archive=\"hdf5-1.10.6.tar.gz\",;archive=\"hdf5-1.10.7.tar.gz\",;g" ./install_saturne.py
    sed -i "s;url=\"https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.6/src/%s\";url=\"https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.7/src/%s\";g" ./install_saturne.py
    
    echo "    patch med version check"
    sed -i "s;\([[:space:]]*\)if lib == 'scotch':;\1if lib == 'med':\n\1    print(p.source_dir + \"/config/med_check_hdf5.m4\")\n\1    subprocess.run([\"sed\", \"-i\", \"s|\\\\[\\\\[ \\\\\\\\\\\\\\\t\\\\]\\\\]|[[:space:]]|g\", p.source_dir + \"/configure\"])\n\1    p.install()\n\1if lib == 'scotch':;g" ./install_saturne.py
    
    echo "    patch no-error"
    sed -i "s;self.packages\['code_saturne'\].config_opts = config_opts;self.packages\['code_saturne'\].config_opts = config_opts + ' CXXFLAGS=\"-Wno-error=incompatible-pointer-types -Wno-error=implicit-function-declaration -Wno-error=int-conversion\" CFLAGS=\"-Wno-error=incompatible-pointer-types -Wno-error=implicit-function-declaration -Wno-error=int-conversion\" --enable-mpi-io';g" ./install_saturne.py
    cd -
    #cd ${CODE_SATURNE_SOURCE_DIR}/libple/
    #autoreconf -iv
    #mkdir build-aux
    #aclocal
    #autoheader
    #automake --force-missing --add-missing
    #autoconf
    #cd -
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
    #sed -i "s/hdf5\s\+no\s\+no\s\+None/hdf5       yes   yes      None/g" ${CODE_SATURNE_INSTALL_DIR}/setup
    sed -i "s/hdf5\s\+auto\s\+no\s\+None/hdf5       yes   yes      None/g" ${CODE_SATURNE_INSTALL_DIR}/setup
    sed -i "s/med\s\+no\s\+no\s\+None/med        yes   yes      None/g" ${CODE_SATURNE_INSTALL_DIR}/setup

    echo "    build from setup..."
    cd ${CODE_SATURNE_INSTALL_DIR}
    ./../${CODE_SATURNE_SOURCE_DIR}/install_saturne.py
    cd -
fi

if command -v module &> /dev/null; then
    module purge
fi
