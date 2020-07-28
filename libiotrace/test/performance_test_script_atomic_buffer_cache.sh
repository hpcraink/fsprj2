#!/bin/bash

min_threads=1
max_threads=8
steps_threads=1
min_iterations=250000000
max_iterations=1000000000
steps_iterations=250000000

file=performance_cache
format="\t%E\t%U\t%S\t%D\t%K\t%M"

rm $file
echo -e "test\tthreads\titerations\treal h:m:s\tuser CPU-seconds\tsys CPU-seconds\tavg unshared data kb\tavg data+stack+text kb\tmax kb">>$file

for ((t = min_threads; t <= max_threads; t += steps_threads)); do

for ((i = min_iterations; i <= max_iterations; i += steps_iterations)); do
OMP_NUM_THREADS=$t /usr/bin/time -o $file -a -f "malloc\t$t\t$i$format" ./OpenMP_atomic_memory_cache_malloc_static "$i";
OMP_NUM_THREADS=$t /usr/bin/time -o $file -a -f "cache\t$t\t$i$format" ./OpenMP_atomic_memory_cache_measure_static "$i";
done

done
