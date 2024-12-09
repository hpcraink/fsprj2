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
Installs InfluxDB, OpenFOAM, Code Saturne and Salome.
Configures `<<test dir>/config/config`, so it points to `<test dir>`.
Preferable use tmux or screen for this command (command will run for more than one hour).

4. choose a test to run

All files in `<libiotrace dir>/libiotrace/scripts/testscripts/config/`
starting with `test_` are usable test configurations.
Choose one and change the value of the variable `test_config`
in the file `<test dir>/config/config`
to the name of the choosen test configuration file.

5. run the test

```shell
cd <test dir>
./<libiotrace dir>/libiotrace/scripts/testscripts/start.sh
```
Configures and builds libiotrace.
Starts the test case with the build libiotrace as sbatch.

6. connect to InfluxDB and/or get a InfluxDB backup

6.1. connect to InfluxDB

`squeue` and `squeue --start` show if the sbatch job already runs.
If the job has started a slurm log is written to `<test dir>`.

The `ib0 address of influxDB` from the slurm log can be used to connect to the InfluxDB.
A new ssh connection with port forwarding is needed:
```shell
ssh -L8086:<ib0 address of influxDB>:8086 es_<user id>@bwunicluster.scc.kit.edu
```
Now the InfluxDB is available at `http://localhost:8086/`.

6.2 get a InfluxDB backup

The test script (see 5. run the test) makes a backup at the end of the test.
This backup ist saved in `<test dir>/scripts/influxdb_backup/<timestamp>`.
Zip the backup on the cluster:
```shell
zip -r <influxdb backup name>.zip <test dir>/scripts/influxdb_backup/<timestamp>/
```
Switch to a shell on your system and copy/extract the backup:
```shell
cd <fsprj2 dir>/Live-Tracing/testbackup
scp es_<user id>@bwunicluster.scc.kit.edu:/home/es/es_es/es_<user id>/<path to test dir>/<test dir>/<influxdb backup name>.zip .
unzip <influxdb backup name>.zip
```
_Replace_ all content of an existing InfluxDB on your system (all existing content is _deleted_):
```shell
cd <fsprj2 dir>/Live-Tracing
sudo docker-compose up
sudo docker exec -it libiotrace_influxdb bash
cd backup/
influx restore --full --token OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== <timestamp>/
```
Alternatively you can add the restored data to a new bucket
```shell
cd <fsprj2 dir>/Live-Tracing
sudo docker-compose up
sudo docker exec -it libiotrace_influxdb bash
cd backup/
influx restore --full --bucket <name of a new bucket> --token OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== <timestamp>/
```

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
