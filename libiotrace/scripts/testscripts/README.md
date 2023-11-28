# Testscases

## Quick Start Guide

1. ssh to bwUniCluster

```shell
ssh es_<user id>@bwunicluster.scc.kit.edu
```

2. clone libiotrace

```shell
mkdir <libiotrace dir>
cd <libiotrace dir>
git clone https://github.com/hpcraink/fsprj2.git
```

3. create test directory

```shell
mkdir <test dir>
cd <test dir>
./<libiotrace dir>/libiotrace/scripts/testscripts/install/build_and_configure_all.sh
```
Installs InfluxDB and OpenFOAM.
Configures `<libiotrace dir>/libiotrace/scripts/testscripts/config/config`, so it points to `<test dir>`.
Preferable use tmux or screen for this command (command will run for more than one hour).

4. choose a test to run

All files in `<libiotrace dir>/libiotrace/scripts/testscripts/config/`
starting with `test_` are usable test configurations.
Choose one and change the value of the variable `test_config`
in the file `<libiotrace dir>/libiotrace/scripts/testscripts/config/config`
to the name of the choosen test configuration file.

5. run the test

```shell
cd <test dir>
./<libiotrace dir>/libiotrace/scripts/testscripts/start.sh
```
Configures and builds libiotrace.
Starts the test case with the build libiotrace as sbatch.

6. connect to InfluxDB

`squeue` and `squeue --start` show if the sbatch job already runs.
If the job has started a slurm log is written to `<test dir>`.

The `ib0 address of influxDB` from the slurm log can be used to connect to the InfluxDB.
A new ssh connection with port forwarding is needed:
```shell
ssh -L8086:<ib0 address of influxDB>:8086 es_<user id>@bwunicluster.scc.kit.edu
```
Now the InfluxDB is available at `http://localhost:8086/`.

## Overview Testcases
## test_openFOAM_motorBike
tbd.
### test_mpi_file_io
tbd.
### test_mpi_file_io_2
tbd.
### test_posix_file_io_random
tbd.

## Local Setup

### Restore InfluxDB Dump

tbd.
