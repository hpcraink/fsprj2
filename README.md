# libiotrace and IOTrace_Analyze – Tools for analyzing program File-I/O 

Tools to monitor, analyze and visualize File-I/O.

## Table of contents

* [libiotrace](#libiotrace)
    * [Prerequisites](#Prerequisites)
    * [License](#License)
    * [Build libiotrace](#Build-libiotrace)
    * [Use libiotrace](#Use-libiotrace)
* [IOTrace_Analyze](#IOTrace_Analyze)
    * [Prerequisites](#Prerequisites-1)
    * [License](#License-1)
    * [Build IOTrace_Analyze](#Build-IOTrace_Analyze)
    * [Use IOTrace_Analyze](#Use-IOTrace_Analyze)
    * [Generated files](#Generated-files)

## libiotrace

_libiotrace_ is a tool for monitoring a running dynamically linked program without the need for changing it.
During a monitored run detailed data for many File-I/O related function calls is collected.
The collected data is written to log files.

### Prerequisites

_libiotrace_ is currently only available for Linux-Systems.
It's tested with _Red Hat Enterprise Linux Server release 7.7_, _KDE neon User Edition 5.18 release 18.04_ and _Ubuntu 18.04.3 LTS_.

To build _libiotrace_ on your system you need a C/C++-Compiler, _make_, _CMake_ and optional _ccmake_.

### License

BSD 3-Clause

llhttp source https://github.com/nodejs/llhttp: MIT License

#### Tools needed to build _libiotrace_

CMake and ccmake: OSI-approved BSD 3-Clause License (see [CMake](https://cmake.org/))

CUnit: GNU LIBRARY GENERAL PUBLIC LICENSE Version 2 (see [CUnit](https://gitlab.com/cunity/cunit))

### Build libiotrace

Steps to build _libiotrace_:

1. create a new folder &lt;libiotrace-folder&gt; for the source
2. to get the source you have two options
    * using git with terminal:
        1. change dir to &lt;libiotrace-folder&gt;
        2. use command `git clone https://github.com/hpcraink/fsprj2.git`
    * using “Clone or download”-Button on https://github.com/hpcraink/fsprj2
        1. download zip
        2. extract zip to &lt;libiotrace-folder&gt;
3. build it
    1. open terminal
    2. go to &lt;libiotrace-folder&gt;
    3. `cd fsprj2/libiotrace/`
    4. `git rm --cached ext/cunit`
    5. `rm -rf ext`
    6. `cd ..`
    7. `git submodule add https://gitlab.com/cunity/cunit.git libiotrace/ext/cunit`
    8. `cd libiotrace/`
    9. `mkdir build` (for out of source build)
    10. `cd build/`
    11. `ccmake ..` (if you want to use cmake instead of ccmake type `cmake ..` instead of `ccmake ..`, set options with -D\<option\> and continue with step `make`)
    12. press “c” and wait until configuration is done
    13. optional: customize libiotrace (set/change _cmake_ options)
    
        * _BUFFER_SIZE_:
        
          libiotrace buffers output in one buffer per monitored process.
          This option sets the buffer size in bytes.
          A bigger buffer reduces the overhead of libiotrace.
          But a bigger buffer means also reduced available memory for the monitored program and the system.
          
        * _LOGGING_:
        
          If set to _ON_ detailed data is collected and written to output files.
          If set to _OFF_ nothing is collected.
          Setting this option to _OFF_ is only useful together with using the _libiotrace.h_ (see [Use libiotrace](#Use-libiotrace)).
          
        * _LOG_WRAPPER_TIME_:
        
          If set to _ON_ the time needed for collecting and writing the data is written to the output.
          In that case the overhead of _libiotrace_ can be included in further analysis.
          
        * _MAX_ERROR_TEXT_:
        
          For functions which use the lvalue errno to return error values libiotrace collects the error value and a corresponding error text.
          This option sets the maximum length of this text.
          Longer values are truncated.
          
        * _MAX_EXEC_ARRAY_LENGTH_:
        
          If the monitored program is dynamically linked and calls a exec-function the environment variable _LD_PRELOAD_ must be set for the new process (see [Use libiotrace](#Use-libiotrace) for _LD_PRELOAD_).
          To ensure this all in the parameters of the exec function given environment variables have to be inspected and in some cases changed.
          The maximum number of inspected variables is set with this option.
          
        * _MAX_FUNCTION_NAME_:
        
          Sets the maximum length of collected function names.
          Longer names get truncated.
        
        * _MAX_INFLUX_TOKEN_:
          Sets the maximum length for the InfluxDB token.

        * _MAX_MMSG_MESSAGES_:
        
          If the monitored program sends or receives multiple messages via call of function sendmmsg or recvmmsg this option sets the maximum count of collected messages in a single function call. 
        
        * _MAX_MSG_FILE_DESCRIPTORS_:
        
          If the monitored program uses _Unix Domain Sockets_ to send _File Descriptors_ from one process to another process this option sets the maximum count of collected _File Descriptors_ in a single send or receive message.
          
        * _MAX_STACKTRACE_DEPTH_:
        
          This option sets the maximum depth of an collected stack trace (the maximum number of inspected stack frames).
          A stack trace for each monitored function call is only collected if the option _STACKTRACE_DEPTH_ is set to a number greater than 0 and at least one of the options _STACKTRACE_PTR_ and _STACKTRACE_SYMBOL_ is set to _ON_ or a corresponding function from _libiotrace.h_ (see [Use libiotrace](#Use-libiotrace)) is used.
          
        * _MAX_STACKTRACE_ENTRY_LENGTH_:
        
          Sets maximum length of a single entry in a collected stack trace.
          Longer values are truncated.

        * _PORT_RANGE_MAX_:
          When Live-Tracing is enabled wrappers can be enabled and disabled remotely at runtime per process. To receive the control information libiotrace has to use a port per process. This is the maximum port value for that libiotrace tries to get a port. 

        * _PORT_RANGE_MIN_:
          When Live-Tracing is enabled wrappers can be enabled and disabled remotely at runtime per process. To receive the control information libiotrace has to use a port per process. This is the minimum port value for that libiotrace tries to get a port. 


        * _SENDING_:
          When set to _ON_ each wrapper sends its data live to InfluxDB. Please look in section _Live-Tracing_ which parameteres are required to send data to InfluxDB.

        * _STACKTRACE_DEPTH_:
        
          Sets the maximum number of currently collected stack trace entries.
          If the current stack trace is deeper than _STACKTRACE_DEPTH_ entries will be omitted.
          The value of _STACKTRACE_DEPTH_ has to be less than or equal to the value of _MAX_STACKTRACE_DEPTH_.
          _STACKTRACE_DEPTH_ can be changed during the run of the monitored program if _libiotrace.h_ is used see [Use libiotrace](#Use-libiotrace).
          
        * _STACKTRACE_PTR_:
        
          If set to _ON_ and _STACKTRACE_DEPTH_ is greater than 0 the memory address of stack trace entries is collected.
          
        * _STACKTRACE_SYMBOL_:
        
          If set to _ON_ and _STACKTRACE_DEPTH_ is greater than 0 the symbol name of stack trace entries is collected.
          
        * _WITH_DL_IO_:
        
          If set to _ON_ functions from _dlfcn.h_ are monitored (namely _dlopen_ and _dlmopen_).
          
        * _WITH_MPI_IO_:
        
          If set to _ON_ functions from _mpi_io.c_ are monitored
          
        * _WITH_POSIX_AIO_:
        
          If set to _ON_ functions from _aio.h_ (POSIX Asynchronous Input and Output) are monitored (namely _aio_read_, _aio_read64_, _aio_write_, _aio_write64_, _lio_listio_, _lio_listio64_, _aio_error_, _aio_error64_, _aio_return_, _aio_return64_, _aio_fsync_, _aio_fsync64_, _aio_suspend_, _aio_suspend64_, _aio_cancel_, _aio_cancel64_, _aio_init_ and _shm_open_).
          IOTrace_Analyze (see [IOTrace_Analyze](#IOTrace_Analyze)) doesn't analyze these functions. This will be implemented in the future.
          
        * _WITH_POSIX_IO_:
        
          If set to _ON_ functions from _dirent.h_, _fcntl.h_, _stdio.h_, _stdio_ext.h_, _stdlib.h_, _sys/epoll.h_, _sys/eventfd.h_, _sys/inotify.h_, _sys/memfd.h_, _sys/mman.h_, _sys/select.h_, _sys/socket.h_, _sys/uio.h_, _unistd.h_ and _wchar.h_ are monitored (for a complete list of functions see &lt;libiotrace-folder&gt;/fsprj2/libiotrace/src/posix_io.h).
          
        * _WITH_STD_IO_:
        
          If set to _ON_ functions calls which work with a _File Descriptor_ equal to _STDIN_FILENO_, _STDOUT_FILENO_ or _STDERR_FILENO_ and functions which work with a _file stream_ equal to _stdin_, _stdout_ or _stderr_ will be monitored.
          So if set to _OFF_, for such function calls no data will be collected.
          This can be a problem in _IOTrace_Analyze_.
          If for example the _File Descriptor_ _STDOUT_FILENO_ is duplicated with an call to the _dup2_ function and the resulting duplicate is not equal to _STDIN_FILENO_ or _STDERR_FILENO_ the output analysis in _IOTrace_Analyze_ will be wrong.
          Thats the case because the original _File Descriptor_ and the call of the _dup2_ function are not collected but the new _File Descriptor_ and function calls with this new _File Descriptor_ are collected.
          With this data the _IOTrace_Analyze_ is not able to get the correct file for the monitored function calls. The new _File Descriptor_ could be in use with an other file before the call to _dup2_.
          So during analysis the following calls to the new _File Descriptor_ will be connected to the file in use before the call of _dup2_.
          Which is probably wrong.
          So if you are not sure if the monitored program manipulates the standard (std) _file streams_ or _File Descriptors_ (e.g. with an redirect of standard _file streams_ during start of an new process) set this option to _ON_.
          In any other case you can omit a lot of overhead by setting it to _OFF_.
          
    14. press “c” again (this brings up the option “g” to generate)
    15. press “g” and wait until _ccmake_ exits
    16. `make` (wait until build is done)
    17. libiotrace is now available in folder &lt;libiotrace-folder&gt;/fsprj2/libiotrace/build/src
        * _libiotrace_shared.so_ (for dynamically linked programs)
        * _libiotrace_static.a_ (for linking against static linked programs)

### Use libiotrace

* dynamically linked program

    to monitor the program &lt;monitor-program&gt; use the command
    `LD_PRELOAD=<libiotrace-folder>/fsprj2/libiotrace/build/src/libiotrace_shared.so IOTRACE_LOG_NAME=<prefix-for-log-names> <monitor-program>`
    
* static linked program

    link your program against _libiotrace_static.a_ with _ld_ linker option `-wrap` for each function you want to monitor (complete list of possible functions is available in &lt;libiotrace-folder&gt;/fsprj2/libiotrace/test/CMakeLists.txt)

* using _libiotrace.h_

    to control and manipulate the behavior of _libiotrace_ during a run of the monitored program use the _libiotrace.h_ header.
    For that you have to change the monitored program.
    Build against the &lt;libiotrace-folder&gt;/fsprj2/libiotrace/include/libiotrace.h and link against &lt;libiotrace-folder&gt;/fsprj2/libiotrace/build/src/libiotrace.so.
    Use the functions out of _libiotrace.h_ directly in the source of the monitored program.
    If the changed monitored program is started with _LD_PRELOAD_ set to a path pointing to _libiotrace_shared.so_ the functions out of _libiotrace.h_ will manipulate the behavior.
    Otherwise the functions have no effect.
    
    Functions in _libiotrace.h_:
    
    * ```c
      void libiotrace_start_log();
      ```
      Start logging in actual thread. Useful in combination with the cmake option _LOGGING_ (see [Build libiotrace](#Build-libiotrace) and _ccmake_).
      With this function and the option it is possible to monitor only part of a program.
      
    * ```c
      void libiotrace_end_log();
      ```
      End logging in actual thread.
      
    * ```c
      void libiotrace_start_stacktrace_ptr();
      ```
      Start logging of stacktrace pointer in actual thread (if logging is active and stacktrace depth is greater than 0).
    
    * ```c
      void libiotrace_end_stacktrace_ptr();
      ```
      End logging of stacktrace pointer in actual thread.
      
    * ```c
      void libiotrace_start_stacktrace_symbol();
      ```
      Start logging of stacktrace symbols in actual thread (if logging is active and stacktrace depth is greater than 0).
      
    * ```c
      void libiotrace_end_stacktrace_symbol();
      ```
      End logging of stacktrace symbols in actual thread.
      
    * ```c
      void libiotrace_set_stacktrace_depth(int depth);
      ```
      Set stacktrace depth for logging in actual thread to _depth_.
      If _depth_ is 0 no stacktrace is logged.
      _depth_ must be less than or equal to _MAX_STACKTRACE_DEPTH_ (see [Build libiotrace](#Build-libiotrace) and _ccmake_).
      
    * ```c
      int libiotrace_get_stacktrace_depth();
      ```
      Get current stacktrace depth.

The output will be placed in the working direrctory of &lt;monitor-program&gt;.
Every generated file has a name beginning with &lt;prefix-for-log-names&gt;.

## IOTrace_Analyze

_IOTrace_Analyze_ is used to prepare and analyze the output of _libiotrace_.
This tool reconstructs the sequence of function calls for every thread and every file.
It also evaluates the connection between a thread and a file, the time used and the amount of data transported for each function call.
The results of this processing are stored in an optimized data model. On the basis of this data model, various graphics are generated.
With these graphics, the efficiency of the constellations in the program can be determined. Furthermore, an animation is generated.
This animation shows the function calls in a graph over time.
The collected and processed data is provided as output files for further analysis.
The graphics, the animation and the output files enable improvements of the File-I/O.

### Prerequisites

To use _IOTrace_Analyze_ a Java Runtime has to be installed.
It's tested with _java-11-openjdk-amd64_ on Ubuntu and _jre1.8.0_102_ on windows.

Generating the animation is a problem on a headless system.
The [_Gephi Toolkit_](https://github.com/gephi/gephi-toolkit) is used to animate the Graph.
Using this toolkit on a headless system will throw a HeadlessException.
If you want to run IOTrace_Analyze on a headless system omit the animation.
To do this set the entry writeAnimations in the _IOTrace_Analyze.properties_ to false.
Alternatively you can do X11 forwarding (use option -X with ssh command) to generate the animations on a headless system.

### License

BSD 3-Clause

#### Tools needed to build _IOTrace_Analyze_

Maven: Apache License, Version 2.0 (see [Maven](http://maven.apache.org/))

#### _IOTrace_Analyze_ dependencies

Gephi Toolkit: CDDL 1.0 and GNU General Public License v3 (see [Gephi Toolkit](https://github.com/gephi/gephi-toolkit))

Iceberg Charts: Apache License, Version 2.0 (see [MVNrepository](https://mvnrepository.com/artifact/com.frontangle/iceberg-charts/1.2.0))

JCodec: FreeBSD License (see [JCodec](http://jcodec.org/))

JFreeChart: GNU Lesser General Public Licence (see [JFreeChart](http://www.jfree.org/jfreechart/))

JUnit: Eclipse Public License 1.0 (see [MVNrepository](https://mvnrepository.com/artifact/junit/junit/4.12))

Log4j: Apache License, Version 2.0 (see [CMake](https://logging.apache.org/log4j/2.x/))

### Build IOTrace_Analyze

1. get the source like described in [Build libiotrace](#Build-libiotrace) step 1 and 2.
2. to get the jar you have two options
    * build a new jar with maven out of the directory &lt;libiotrace-folder&gt;/fsprj2/IOTrace_Analyze with command `mvn clean install`
    * use the provided snapshot _IOTrace_Analyze-0.0.1-SNAPSHOT-jar-with-dependencies.jar_ in &lt;libiotrace-folder&gt;/fsprj2/IOTrace_Analyze/test/

### Use IOTrace_Analyze

1. put the libiotrace output, the _IOTrace_Analyze-jar_, some _log4j2.properties_ and the _IOTrace_Analyze.properties_ in the same directory. Examples for the properties can be found in &lt;libiotrace-folder&gt;/fsprj2/IOTrace_Analyze/test/.
    * it's possible to alternate between different properties by using the command line parameters `-analyzeprop=<path/filename to IOTrace_Analyze.properties>` and `-log4jprop=<path/filename to log4j2.properties>`.
      If one of these parameters is given the apropriate file is not searched in the same directory.
      Instead it's loaded with the given filename from the given path. 
2. edit the _IOTrace_Analyze.properties_. At least the entry _inputFile_ has to be changed to the value of &lt;prefix-for-log-names&gt; to find the libiotrace output. The other entrys define which output will be generated.
    * each value loaded from _IOTrace_Analyze.properties_ can be overwritten by using a command line parameter.
      So e.g. instead of changing the entry _inputFile_ the parameter `-inputFile=<prefix-for-log-names>` could be used.
3. run the jar with the command `java -jar <IOTrace_Analyze-jar>`
   (or with more parameters e.g.: `java -jar <IOTrace_Analyze-jar> -inputFile=<prefix-for-log-names>`;
   for big log files it's necessary to increase the maximum memory allocation pool for the JVM with an additional parameter like `-Xmx16g`: `java -Xmx16g -jar <IOTrace_Analyze-jar> -inputFile=<prefix-for-log-names>`)

If the given properties are used, two new directorys are generated.
One with the name _logs_ which includes the file _IOTrace_Analyze.log_.
And one with the name _output_ which includes all generated diagrams, output files and animations.

### Generated files

#### &lt;prefix-for-log-names&gt;_function_summary.png

*&lt;prefix-for-log-names&gt;_function_summary.png* shows a bar chart with one entry for each monitored function.
Multiple calls of the same function are summarized.
For each function two to three bars are shown.
One bar shows the read or written bytes.
A other bar shows the time this function has needed in nano seconds.
The third bar is optional and only present if the option _LOG_WRAPPER_TIME_ during build of libiotrace was set (see [Build libiotrace](#Build-libiotrace) and _ccmake_).
If the option was given the bar shows the time needed for the wrapper functionality used to monitor the function (this shows the overhead of libiotrace).

The bar chart shows the efficiency of the used functions.
A function that uses a lot of time to read or write a few bytes is less efficient than a function that reads or writes more bytes in the same or even less time.
So this char shows some optimization potential.

![alt text](https://raw.githubusercontent.com/hpcraink/fsprj2/master/IOTrace_Analyze/test/output/firefox_test221_function_summary.png "&lt;prefix-for-log-names&gt;_function_summary.png")

#### &lt;prefix-for-log-names&gt;_time_pie.png

*&lt;prefix-for-log-names&gt;_time_pie.png*

![alt text](https://raw.githubusercontent.com/hpcraink/fsprj2/master/IOTrace_Analyze/test/output/firefox_test221_time_pie.png "&lt;prefix-for-log-names&gt;_time_pie.png")

#### &lt;prefix-for-log-names&gt;_bytes_pie.png

*&lt;prefix-for-log-names&gt;_bytes_pie.png*

![alt text](https://raw.githubusercontent.com/hpcraink/fsprj2/master/IOTrace_Analyze/test/output/firefox_test221_bytes_pie.png "&lt;prefix-for-log-names&gt;_bytes_pie.png")

#### &lt;prefix-for-log-names&gt;_1.mp4

*&lt;prefix-for-log-names&gt;_1.mp4*

![alt text](https://raw.githubusercontent.com/hpcraink/fsprj2/master/IOTrace_Analyze/test/output/firefox_test221_1.gif "&lt;prefix-for-log-names&gt;_1.mp4")



## Live-Tracing

To use Grafana and InfluxDB to trace every function call in real-time and write its values in the databse you have to do the following:

1. Go to the fsprj2 root directory
2. `cd libiotrace/build`
3. `ccmake ..`
4. Turn the option "SENDING" on
5. `cd ../../Grafana`
6. `sudo docker-compose up -d`

InfluxDB is now available under http://localhost:8086 (username: admin password: test12345678) and grafana under http://localhost:3000 (username: admin password: admin). 

Now you can use libiotrace like in the following example to send live data to InfluxDB. The Token is preconfigured with docker-compose and doesn't have to be changed. If you change token, organization name or bucket name you have to reconfigure the data sources in Grafana.

When _WITH_POSIX_IO_ is activated in cmake you can only use IPv4 addresses for _IOTRACE_DATABASE_IP_. When it is disabled name resolution is possible.

You can use a whitelist to specify which wrappers should be traced when "ALL_WRAPPERS_ACTIVE" in cmake is turned off. You have to create a new file called _whitelist_ in the directory of the program that should be traced. The whitelist file contains in each line the name of exactly one function. This could look like:

`MPI_File_open`<br>
`MPI_File_write`<br>
`MPI_File_read` <br>
`fopen` <br>
`fclose` <br>


To trace MPI File-I/O wrappers you have to turn on "WITH_MPI_IO" in ccmake.

### Example to use Live-Tracing for MPI with libiotrace
`mpirun -np 4 -x IOTRACE_LOG_NAME=MPI_read_test2 -x IOTRACE_DATABASE_IP=127.0.0.1 -x IOTRACE_DATABASE_PORT=8086 -x IOTRACE_INFLUX_ORGANIZATION=hse -x IOTRACE_INFLUX_BUCKET=hsebucket -x IOTRACE_INFLUX_TOKEN=OXBWllU1poZotgyBlLlo2XQ_u4AYGYKQmdxvJJeotKRyvdn5mwjEhCXyOjyldpMmNt_9YY4k3CK-f5Eh1bN0Ng== -x IOTRACE_WHITELIST=./whitelist -x LD_PRELOAD=/home/julian/Projects/fsprj2/libiotrace/build/src/libiotrace_shared.so mpi_program_to_be_observed`

### Activate and deactivate specific wrappers at runtime
When libiotrace is running in Live Tracing mode it is possible to activate and deactivate wrappers at runtime with HTTP. Each process writes its IP addresses and ports in the "MPI_read_test2_control.log" logfile. 

With this information HTTP-POST requests can be send to each process. E.g. "172.16.244.1:50003/MPI_Waitall/0" will deactivate the Live-Tracing of "MPI_Waitall" at one process. Sending ""172.16.244.1:50003/MPI_Waitall/1" will reactivate this.

Each process can also send a json list with the current status (active or incactive) of each wrapper. To get this information you have to send a HTTP-GET request to each process like "172.16.244.1:50003".
