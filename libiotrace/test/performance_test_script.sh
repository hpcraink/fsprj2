#!/bin/bash

min_threads=1
max_threads=8
steps_threads=1
min_block_size=10
max_block_size=5000
steps_block_size=10
#reuses=10
reuses=1
max_allocs_per_thread=10000
file=performance
format="\t%E\t%U\t%S\t%D\t%K\t%M"

rm $file
echo "allocation of $((max_allocs_per_thread * max_block_size)) bytes per thread in smaller blocks">>$file
echo -e "test\tthreads\tblock size (bytes)\treal h:m:s\tuser CPU-seconds\tsys CPU-seconds\tavg unshared data kb\tavg data+stack+text kb\tmax kb">>$file

for ((t = min_threads; t <= max_threads; t += steps_threads)); do

for ((i = min_block_size; i <= max_block_size; i += steps_block_size)); do
OMP_NUM_THREADS=$t /usr/bin/time -o $file -a -f "malloc\t$t\t$i$format" ./OpenMP_atomic_buffer_write_once_static_malloc "$i" "$((max_allocs_per_thread * max_block_size / i))" "$reuses";
done

for ((i = min_block_size; i <= max_block_size; i += steps_block_size)); do
OMP_NUM_THREADS=$t /usr/bin/time -o $file -a -f "write once buffer\t$t\t$i$format" ./OpenMP_atomic_buffer_write_once_static_measure "$i" "$((max_allocs_per_thread * max_block_size / i))" "$reuses";
done

done
