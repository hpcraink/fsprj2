#!/bin/bash

salome_config="$(dirname "$0")/../config/salome"
if [ -f ${salome_config} ]; then
    source ${salome_config}
else
    echo "file ${salome_config} does not exist"
    exit 1
fi

#rm -rf ${SALOME_INSTALL_DIR}
if ! [ -d ${SALOME_INSTALL_DIR} ]; then
    mkdir ${SALOME_INSTALL_DIR}
fi
cd ${SALOME_INSTALL_DIR}

if ! [ -f ../${SALOME_TAR} ]; then
    echo "Please download Salome for CentOS ${SALOME_ARCH} in version ${SALOME_VERSION} from ${SALOME_URL} and rename it to ${SALOME_TAR}"
    exit 2
else
    if ! [ -d ${SALOME_DIR} ]; then
        tar -zxf ../${SALOME_TAR};
    fi
fi

echo "load modules..."
if command -v module &> /dev/null; then
    module purge
    module load devel/miniconda/${SALOME_MINICONDA_VERSION}
fi

if ! [ -d ${SALOME_CONDA_ENV} ]; then
    conda create -y --prefix ${SALOME_CONDA_ENV}
    conda activate ${PWD}/${SALOME_CONDA_ENV}
    conda install -y -c conda-forge python==${SALOME_PYTHON_VERISON} freeimage==${SALOME_FREEIMAGE_VERSION} psutil openmpi==${SALOME_OPENMPI_VERSION}

    # remove outdated unpatched libcrpto.so (use system lib instead)
    for library in "${SALOME_REMOVE_LIBS[@]}"
    do
        mv ${PWD}/${SALOME_CONDA_ENV}/lib/${library} ${PWD}/${SALOME_CONDA_ENV}/lib/${library}.orig
    done
else
    conda activate ${PWD}/${SALOME_CONDA_ENV}
fi

export PATHTOSALOME=${PWD}/${SALOME_DIR}
export PATH=$PATHTOSALOME:$PATH
export LD_LIBRARY_PATH="${PWD}/${SALOME_CONDA_ENV}/lib:${LD_LIBRARY_PATH}"
source ${PATHTOSALOME}/${SALOME_LAUNCH_SCRIPT}

for dimensions in "${SALOME_MESH_DIMENSIONS[@]}"
do
    x=$(echo ${dimensions} | cut -f1 -d:)
    y=$(echo ${dimensions} | cut -f2 -d:)
    z=$(echo ${dimensions} | cut -f3 -d:)
    s=$(echo ${dimensions} | cut -f4 -d:)
    if ! [ -f ${SALOME_OUTPUT_FILE_PREFIX}_${x}_${y}_${z}_${s}.med ]; then
        $(dirname "$0")/${SALOME_MESH_CUBE_SCRIPT} -o ${SALOME_OUTPUT_FILE_PREFIX} -d . -x ${x} -y ${y} -z ${z} -s ${s}
    fi
done

if command -v module &> /dev/null; then
    module purge
fi
