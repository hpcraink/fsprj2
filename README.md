# libiotrace and IOTrace_Analyze – Tools for analyzing program File-I/O 

## libiotrace

libiotrace is a tool for monitoring a running dynamically linked program without the need for changing it. During a monitored run detailed data for many File-I/O related function calls is collected. The collected data is written to log files.

### Build libiotrace

Steps to build libiotrace

1. create a new folder &lt;libiotrace-folder&gt; for the source
2. get the source
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
	6. `git submodule add https://gitlab.com/cunity/cunit.git ext/cunit`
	7. `mkdir build` (for out of source build)
	8. `cd build/`
	9. `ccmake ..`
	10. press “c” and wait until configuration is done
	11. press “c” again (this brings up the option “g” to generate)
	12. press “g” and wait until ccmake exits
	13. `make` (wait until build is done)
	14. libiotrace is now available in folder &lt;libiotrace-folder&gt;/fsprj2/libiotrace/build/src
		* libiotrace_shared.so (for dynamically linked programs)
		* libiotrace_static.a (for linking against static linked programs)

### Use libiotrace

* dynamically linked program
    to monitor the program &lt;monitor-program&gt; use the command
    `LD_PRELOAD=&lt;path-to-libiotrace&gt;/libiotrace_shared.so IOTRACE_LOG_NAME=&lt;prefix-for-log-names&gt; &lt;monitor-program&gt;`
* static linked program
    link your program against libiotrace_static.a with ld linker option `-wrap` for each function you want to monitor (complete list of possible functions is available in &lt;libiotrace-folder&gt;/fsprj2/libiotrace/test/CMakeLists.txt)

## IOTrace_Analyze

OTrace_Analyze is used to prepare and analyze the output of libiotrace. This tool reconstructs the sequence of function calls for every thread and every file. It also evaluates the connection between a thread and a file, the time used and the amount of data transported for each function call. The results of this processing are stored in an optimized data model. On the basis of this data model, various graphics are generated. With these graphics, the efficiency of the constellations in the program can be determined. Furthermore, an animation is generated. This animation shows the function calls in a graph over time. The collected and processed data is provided as output files for further analysis. The graphics, the animation and the output files enable improvements of the File-I/O.

