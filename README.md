# libiotrace and IOTrace_Analyze – Tools for analyzing program File-I/O 

## libiotrace

libiotrace is a tool for monitoring a running dynamically linked program without the need for changing it.
During a monitored run detailed data for many File-I/O related function calls is collected.
The collected data is written to log files.

### Prerequisites

libiotrace is currently only available for Linux-Systems.
It's testet with Red Hat Enterprise Linux Server release 7.7, KDE neon User Edition 5.18 release 18.04 and Ubuntu 18.04.3 LTS.

To build libiotrace on your system you need a C-Compiler, make and ccmake (or cmake).

### Build libiotrace

Steps to build libiotrace

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
    `LD_PRELOAD=<path-to-libiotrace>/libiotrace_shared.so IOTRACE_LOG_NAME=<prefix-for-log-names> <monitor-program>`
* static linked program

    link your program against libiotrace_static.a with ld linker option `-wrap` for each function you want to monitor (complete list of possible functions is available in &lt;libiotrace-folder&gt;/fsprj2/libiotrace/test/CMakeLists.txt)

The output will be placed in the working direrctory of &lt;monitor-program&gt;.
Every generated file has a name beginning with &lt;prefix-for-log-names&gt;.

## IOTrace_Analyze

IOTrace_Analyze is used to prepare and analyze the output of libiotrace.
This tool reconstructs the sequence of function calls for every thread and every file.
It also evaluates the connection between a thread and a file, the time used and the amount of data transported for each function call.
The results of this processing are stored in an optimized data model. On the basis of this data model, various graphics are generated.
With these graphics, the efficiency of the constellations in the program can be determined. Furthermore, an animation is generated.
This animation shows the function calls in a graph over time.
The collected and processed data is provided as output files for further analysis.
The graphics, the animation and the output files enable improvements of the File-I/O.

### Prerequisites

To use IOTrace_Analyze a Java Runtime has to be installed.
It's tested with java-11-openjdk-amd64 on Ubuntu and jre1.8.0_102 on windows.

Generating the animation is not possible on a headless system.
The Gephi Toolkit is used to animate the Graph.
Using this toolkit on a headless system will throw a HeadlessException.
If you want to run IOTrace_Analyze on a headless system omit the animation.
To do this set in the IOTrace_Analyze.properties the entry writeAnimations to false.

### Build IOTrace_Analyze

1. get the source like described in [Build libiotrace](#Build-libiotrace) step 1 and 2.
2. to get the jar you have two options
    * build a new jar with maven out of the directory &lt;libiotrace-folder&gt;/fsprj2/IOTrace_Analyze
    * use the provided snapshot IOTrace_Analyze-0.0.1-SNAPSHOT-jar-with-dependencies.jar in &lt;libiotrace-folder&gt;/fsprj2/IOTrace_Analyze/test/

### Use IOTrace_Analyze

1. put the libiotrace output, the IOTrace_Analyze-jar, some log4j2.properties and the IOTrace_Analyze.properties in the same directory. Examples for the properties can be found in &lt;libiotrace-folder&gt;/fsprj2/IOTrace_Analyze/test/.
2. edit the IOTrace_Analyze.properties. At least the entry inputFile has to be changed to the value of &lt;prefix-for-log-names&gt; to find the libiotrace output. The other entrys define which output will be generated.
3. run the jar with the command `java -jar <IOTrace_Analyze-jar>` (for big log files it's necessary to increase the maximum memory allocation pool for the JVM with an additional parameter like `-Xmx16g`)

If the given properties are used, two new directorys are generated.
One with the name logs which includes the file IOTrace_Analyze.log.
And one with the name output which includes all generated diagrams, output files and animations.

### Generated files

#### &lt;prefix-for-log-names&gt;_function_summary.png

&lt;prefix-for-log-names&gt;_function_summary.png shows a bar chart with one entry for each monitored function.
Multiple calls of the same function are summarized.
For each function two to three bars are shown.
One bar shows the read or written bytes.
A other bar shows the time this function has needed in nano seconds.
The third bar is optional and only present if the option LOG_WRAPPER_TIME during build of libiotrace was set (see [Build libiotrace](#Build-libiotrace) and ccmake).
If the option was given the bar shows the time needed for the wrapper functionality used to monitor the function (this shows the overhead of libiotrace).

The bar chart shows the efficiency of the used functions.
A function that uses a lot of time to read or write a few bytes is less efficient than a function that reads or writes more bytes in the same or even less time.
So this char shows some optimization potential.

![alt text](https://raw.githubusercontent.com/hpcraink/fsprj2/master/IOTrace_Analyze/test/output/firefox_test221_function_summary.png "&lt;prefix-for-log-names&gt;_function_summary.png")

#### &lt;prefix-for-log-names&gt;firefox_test221_time_pie.png

&lt;prefix-for-log-names&gt;firefox_test221_time_pie.png 

![alt text](https://raw.githubusercontent.com/hpcraink/fsprj2/master/IOTrace_Analyze/test/output/firefox_test221_time_pie.png "&lt;prefix-for-log-names&gt;firefox_test221_time_pie.png")
