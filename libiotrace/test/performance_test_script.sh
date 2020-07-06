#!/bin/bash

limit=5000
tests_per_buffer_per_thread=100000
file=performance
format="\t%E\t%U\t%S\t%D\t%K\t%M"

rm $file
echo real h:m:s, user CPU-seconds, sys CPU-seconds, avg unshared data kb, avg data+stack+text kb, max kb>>$file

echo malloc>>$file
for ((i = 10; i <= limit; i += 10)); do
OMP_NUM_THREADS=4 /usr/bin/time -o $file -a -f "$i$format" ./OpenMP_atomic_buffer_write_once_static_malloc "$i" "$((tests_per_buffer_per_thread * limit / i))";
done

echo write once buffer>>$file
for ((i = 10; i <= limit; i += 10)); do
OMP_NUM_THREADS=4 /usr/bin/time -o $file -a -f "$i$format" ./OpenMP_atomic_buffer_write_once_static_measure "$i" "$((tests_per_buffer_per_thread * limit / i))";
done
