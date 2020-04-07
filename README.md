# libiotrace and IOTrace_Analyze – Tools for analyzing program File-I/O 

## libiotrace

libiotrace is a tool for monitoring a running dynamically linked program without the need for changing it. During a monitored run detailed data for many File-I/O related function calls is collected. The collected data is written to log files.

## Build libiotrace

Steps to build libiotrace

1. create a new folder <libiotrace-folder> for the source
2. get the source
	using git with terminal:
		change dir to <libiotrace-folder>
		use command `git clone https://github.com/hpcraink/fsprj2.git`
	using “Clone or download”-Button on https://github.com/hpcraink/fsprj2
		download zip
		extract zip to <libiotrace-folder>
3. build it
	open terminal
	go to <libiotrace-folder>
	`cd fsprj2/libiotrace/`
	`git rm --cached ext/cunit`
	`rm -rf ext`
	`git submodule add https://gitlab.com/cunity/cunit.git ext/cunit`
	`mkdir build` (for out of source build)
	`cd build/`
	`ccmake ..`
	press “c” and wait until configuration is done
	press “c” again (this brings up the option “g” to generate)
	press “g” and wait until ccmake exits
	`make` (wait until build is done)
	libiotrace is now available in folder <libiotrace-folder>/fsprj2/libiotrace/build/src
		libiotrace_shared.so (for dynamically linked programs)
		libiotrace_static.a (for linking against static linked programs)

## Use libiotrace

dynamically linked program
    to monitor the program <monitor-program> use the command
    `LD_PRELOAD=<path-to-libiotrace>/libiotrace_shared.so IOTRACE_LOG_NAME=<prefix-for-log-names> <monitor-program>`
static linked program
    link your program against libiotrace_static.a with ld linker option `-wrap` for each function you want to monitor (complete list of possible functions is available in <libiotrace-folder>/fsprj2/libiotrace/test/CMakeLists.txt)

## IOTrace_Analyze

OTrace_Analyze is used to prepare and analyze the output of libiotrace. This tool reconstructs the sequence of function calls for every thread and every file. It also evaluates the connection between a thread and a file, the time used and the amount of data transported for each function call. The results of this processing are stored in an optimized data model. On the basis of this data model, various graphics are generated. With these graphics, the efficiency of the constellations in the program can be determined. Furthermore, an animation is generated. This animation shows the function calls in a graph over time. The collected and processed data is provided as output files for further analysis. The graphics, the animation and the output files enable improvements of the File-I/O.

