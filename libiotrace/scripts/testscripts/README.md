# Testscases

## Setup BWUniCluster

### Install OpenFoam

1. Run `build_OpenFoam.sh` from target directory

### Install InfluxDB

1. Run `build_InfluxDB.sh` from target directory

or

1. Download `influxdb2-2.0.6-linux-amd64.tar.gz`

```shell
wget https://dl.influxdata.com/influxdb/releases/influxdb2-2.0.6-linux-amd64.tar.gz
```
2. Unpack `influxdb2-2.0.6-linux-amd64.tar.gz`

```shell
tar –xvzf influxdb2-2.0.6-linux-amd64.tar.gz
```

### Setup Test Variables 

1. Copy `config.sample` to `config`

```shell
cp config.sample config
```

2. Change config to own values


## Run Tests

Run `start.sh`


## Overview 
### openFOAM_motorBike
tbd.
### mpi_file_io
tbd.
### mpi_file_io_2
tbd.
### posix_file_io_random
tbd.


## Local Setup

### Restore InfluxDB Dump

tbd.

### Connect to InfluxDB

Connet to Login Node, with Portforwording to DBNODE

```shell
ssh -L8086:DBNODE-IP:8086 MY_LOGIN@bwunicluster.scc.kit.edu
```
=> DBNODE-IP from Slurm-Log



