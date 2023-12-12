#!/bin/bash

config_dir="config"
config_file="config"

# can only be executed from libiotrace testscript subdir
echo "check if script resides in subdir of libiotrace"
libiotrace_CMakeLists_txt="$(dirname "$0")/../../CMakeLists.txt"
if [ -f ${libiotrace_CMakeLists_txt} ]; then
    if ! grep -q "project(libiotrace" "${libiotrace_CMakeLists_txt}"; then
        echo "${libiotrace_CMakeLists_txt} does not contain libiotrace project declaration"
    fi
else
    echo "${libiotrace_CMakeLists_txt} not found"
    exit
fi
echo "    passed"


# source base config file
echo "source base config file"
base_config="$(dirname "$0")/${config_dir}/${config_file}"
if [ -f ${base_config} ]; then
    source ${base_config}
else
    echo "base config file ${base_config} does not exist"
    exit
fi
echo "    done"

# source specific test case config file
echo "source specific test case config file"
test_case_config="${test_config_dir}/${test_config}"
if [ -f ${test_case_config} ]; then
    source ${test_case_config}
else
    echo "test config file ${test_case_config} does not exist"
    exit
fi
echo "    done"

# source additional config files
echo "source additional config files"
for source_config in "${test_source_configs[@]}"
do
    test_source_config="${test_config_dir}/${source_config}"
    if [ -f ${test_source_config} ]; then
        source ${test_source_config}
    else
        echo "config file ${test_source_config} does not exist"
        exit
    fi
done
echo "    done"

# source base config file again
# to correctly expand variables set by additional config files
echo "source base config file again"
source ${base_config}
echo "    done"

# source specific test case config file again
# to correctly expand variables set by additional config files
echo "source specific test case config file again"
source ${test_case_config}
echo "    done"

# create test dir (for tmp and output files)
echo "create test dir"
mkdir -p ${test_dir}
echo "    done"

# load modules
echo "load modules"
module purge
for module in "${test_modules[@]}"
do
    module load ${module}
done
echo "    done"

# source test environment (e.g. openFOAM)
echo "source test environment"
for source_file in "${test_source[@]}"
do
    source ${source_file}
done
echo "    done"

echo "reload modules"
if (( ${#test_source[@]} != 0 )); then
    # workaround: OpenFOAMs /etc/bashrc writes /usr/lib64 to
    # LD_LIBRARY_PATH prior to the openmpi directories; to
    # set the openmpi directories in front of the /usr/lib64
    # a reload of the openmpi module is necessary
    module purge
    for module in "${test_modules[@]}"
    do
        module load ${module}
    done
fi
echo "    done"

# create libiotrace build dir
echo "create libiotrace build dir"
libiotrace_build_dir=${test_libiotrace_build}
mkdir -p "${libiotrace_build_dir}"
echo "    done"

# build libiotrace in subshell (doesn't change current working dir of shell)
echo "build libiotrace"
(
    cmake_lists=$(realpath "$(dirname "$0")/../../")
    cd ${libiotrace_build_dir}
    cmake "${cmake_lists}" "${test_libiotrace_cmake_options[@]}"
    make
)
echo "    done"

echo "load and decompress data"
for file_url in "${test_file_url[@]}"
do
    file_url=(${file_url})
    echo ${file_url[0]}
    echo ${file_url[1]}
    wget -nc -O ${file_url[1]} ${file_url[0]}
done
for file_zip in "${test_unzip_files[@]}"
do
    echo ${file_zip}
    unzip -n ${file_zip}
done
echo "    done"

echo "start sbatch"
sbatch -p ${test_queue_name} -N ${test_nodes} --mem=${test_mem} -t ${test_time} $(dirname "$0")/manage_db_and_worker_nodes.sh ${test_script_dir}/${test_script} ${test_name} ${test_influx_dir} ${test_dir} ${test_processes_per_worker}
echo "    done"
