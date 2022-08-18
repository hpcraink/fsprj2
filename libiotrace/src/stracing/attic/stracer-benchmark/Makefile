# --  Vars  --
# ---------------------------------------------------------------
LIBIOTRACE_DIR             := /mnt/hgfs/uni/fpj/fsprj2/libiotrace
STRACING_LINUX_SRC_DIR     := /usr/src/linux-5.15.0
BENCHMARK_NR_MEASUREMENTS  := 100000
# ---------------------------------------------------------------

CC                         := gcc
CFLAGS                     := -O3 -Wall -Werror
LDLIBS                     := -lbsd

BENCHMARK_EXEC             := stracing_benchmark


BENCHMARK_DIR_PATH         := $(LIBIOTRACE_DIR)/benchmark
BENCHMARK_RESULTS_DIR      := results

LIBIOTRACE_COMPILE_FLAGS   := DCMAKE_BUILD_TYPE=Release  -DENABLE_OUTPUT=LOGFILE -DWITH_MPI_IO=OFF -DENABLE_REMOTE_CONTROL=OFF  -DFILENAME_RESOLUTION_ENABLED=OFF
LIBIOTRACE_SO_FILE         := src/libiotrace_shared.so

LIBIOTRACE_W_STRACING_DIR  := $(BENCHMARK_DIR_PATH)/w_stracing
LIBIOTRACE_WO_STRACING_DIR := $(BENCHMARK_DIR_PATH)/wo_stracing



# --  Targets  --
.PHONY: benchmark
benchmark: $(BENCHMARK_EXEC) env_setup
	mkdir -p $(BENCHMARK_RESULTS_DIR) &&\
		./$(BENCHMARK_EXEC) $(BENCHMARK_NR_MEASUREMENTS)  > $(BENCHMARK_RESULTS_DIR)/0-wo_libiotrace.csv &&\
		IOTRACE_LOG_NAME=delete_me_0 LD_PRELOAD=$(LIBIOTRACE_WO_STRACING_DIR)/$(LIBIOTRACE_SO_FILE) ./$(BENCHMARK_EXEC) $(BENCHMARK_NR_MEASUREMENTS)  > $(BENCHMARK_RESULTS_DIR)/1-1-wo_stracing.csv &&\
		IOTRACE_LOG_NAME=delete_me_1 LD_PRELOAD=$(LIBIOTRACE_W_STRACING_DIR)/$(LIBIOTRACE_SO_FILE) ./$(BENCHMARK_EXEC) $(BENCHMARK_NR_MEASUREMENTS)  > $(BENCHMARK_RESULTS_DIR)/1-2-w_stracing.csv &&\
		rm delete_me* libiotrace_stracer.log


$(BENCHMARK_EXEC): stracing_benchmark.c


.PHONY: env_setup
env_setup: $(LIBIOTRACE_W_STRACING_DIR)/$(LIBIOTRACE_SO_FILE) $(LIBIOTRACE_WO_STRACING_DIR)/$(LIBIOTRACE_SO_FILE)

$(LIBIOTRACE_W_STRACING_DIR)/$(LIBIOTRACE_SO_FILE):
	mkdir -p $(LIBIOTRACE_W_STRACING_DIR) &&\
		cd $(LIBIOTRACE_W_STRACING_DIR) &&\
		cmake $(LIBIOTRACE_COMPILE_FLAGS) -DSTRACING_ENABLED=ON -DSTRACING_LINUX_SRC_DIR=$(STRACING_LINUX_SRC_DIR) -DSTRACING_UXD_SOCKET_FILEPATH=/tmp/libiotrace-tracer.sock -DSTRACING_STRACER_LOG_OUTPUT=LOGFILE ../.. &&\
		cmake --build .

$(LIBIOTRACE_WO_STRACING_DIR)/$(LIBIOTRACE_SO_FILE):
	mkdir -p $(LIBIOTRACE_WO_STRACING_DIR) &&\
		cd $(LIBIOTRACE_WO_STRACING_DIR) &&\
		cmake $(LIBIOTRACE_COMPILE_FLAGS) -DSTRACING_ENABLED=OFF ../.. &&\
		cmake --build .


.PHONY: env_cleanup
env_cleanup:
	rm -rf $(BENCHMARK_DIR_PATH)


.PHONY: clean
clean: env_cleanup
	rm $(BENCHMARK_EXEC)
