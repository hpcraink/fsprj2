#!/bin/bash

export IOTRACE_INFLUX_ORGANIZATION=hse
export IOTRACE_INFLUX_BUCKET=hsebucket
export IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng==
export IOTRACE_INFLUX_ADMIN_USER="admin"
export IOTRACE_INFLUX_PASSWORD="test12345678"


./influx backup /home/devDachs/MPI_power_rank_loop_influx/$(date '+%Y-%m-%d_%H-%M')_${1} -t ${IOTRACE_INFLUX_TOKEN}