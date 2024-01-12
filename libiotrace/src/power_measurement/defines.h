//
// Created by andreas on 11.12.23.
//

#ifndef POWERMEASUREMENT_DEFINES_H
#define POWERMEASUREMENT_DEFINES_H

#define MAX_STR_LEN 100
#define CPU_INTEL 6
#define CPU_AMD 23

#define METHOD_RAPL 1
#define METHOD_POWERCAP 2

// RAPL

/***************/
/* AMD Support */
/***************/
#define MSR_AMD_RAPL_POWER_UNIT                 0xc0010299

#define MSR_AMD_PKG_ENERGY_STATUS               0xc001029B
#define MSR_AMD_PP0_ENERGY_STATUS               0xc001029A

/*****************/
/* Intel support */
/*****************/

/*
 * Platform specific RAPL Domains.
 * Note that PP1 RAPL Domain is supported on 062A only
 * And DRAM RAPL Domain is supported on 062D only
 */

/* RAPL defines */
#define MSR_INTEL_RAPL_POWER_UNIT    0x606

/* Package */
#define MSR_PKG_RAPL_POWER_LIMIT        0x610
#define MSR_INTEL_PKG_ENERGY_STATUS     0x611
#define MSR_PKG_PERF_STATUS             0x613
#define MSR_PKG_POWER_INFO              0x614

/* PP0 */
#define MSR_PP0_POWER_LIMIT             0x638
#define MSR_INTEL_PP0_ENERGY_STATUS     0x639
#define MSR_PP0_POLICY                  0x63A
#define MSR_PP0_PERF_STATUS             0x63B

/* PP1 */
#define MSR_PP1_POWER_LIMIT             0x640
#define MSR_PP1_ENERGY_STATUS           0x641
#define MSR_PP1_POLICY                  0x642

/* DRAM */
#define MSR_DRAM_POWER_LIMIT            0x618
#define MSR_DRAM_ENERGY_STATUS          0x619
#define MSR_DRAM_PERF_STATUS            0x61B
#define MSR_DRAM_POWER_INFO             0x61C

/* PSYS RAPL Domain */
#define MSR_PLATFORM_ENERGY_STATUS      0x64d

/* RAPL bitsmasks */
#define POWER_UNIT_OFFSET          0
#define POWER_UNIT_MASK         0x0f

#define ENERGY_UNIT_OFFSET      0x08
#define ENERGY_UNIT_MASK        0x1f

#define TIME_UNIT_OFFSET        0x10
#define TIME_UNIT_MASK          0x0f

/* RAPL POWER UNIT MASKS */
#define POWER_INFO_UNIT_MASK     0x7fff
#define THERMAL_SHIFT                 0
#define MINIMUM_POWER_SHIFT          16
#define MAXIMUM_POWER_SHIFT          32
#define MAXIMUM_TIME_WINDOW_SHIFT    48

#define PACKAGE_ENERGY        0
#define PACKAGE_THERMAL        1
#define PACKAGE_MINIMUM        2
#define PACKAGE_MAXIMUM        3
#define PACKAGE_TIME_WINDOW    4
#define PACKAGE_ENERGY_CNT      5
#define PACKAGE_THERMAL_CNT     6
#define PACKAGE_MINIMUM_CNT     7
#define PACKAGE_MAXIMUM_CNT     8
#define PACKAGE_TIME_WINDOW_CNT 9
#define DRAM_ENERGY        10
#define PLATFORM_ENERGY        11

#endif //POWERMEASUREMENT_DEFINES_H

typedef struct CpuPackage {
    int id;
    unsigned int *cpu_ids;
    unsigned int number_cpu_count;
} CpuPackage;

typedef struct CpuInfo {
    CpuPackage *cpu_packages;
    unsigned int cpu_count;
    unsigned int package_count;
} CpuInfo;

typedef struct FileState {
    int file_descriptor;
    int open;
} FileState;

typedef struct CPUMeasurementTask {
    char name[MAX_STR_LEN];
    unsigned int offset_in_file;
    unsigned int type;

    unsigned int cpu_package;
    unsigned int cpu_id;

    long long last_measurement_value;
} CPUMeasurementTask;


//Powercap


// package events
#define PKG_ENERGY                  0
#define PKG_MAX_ENERGY_RANGE        1
#define PKG_MAX_POWER_A             2
#define PKG_POWER_LIMIT_A           3
#define PKG_TIME_WINDOW_A           4
#define PKG_MAX_POWER_B             5
#define PKG_POWER_LIMIT_B           6
#define PKG_TIME_WINDOW_B           7
#define PKG_ENABLED                 8
#define PKG_NAME                    9

#define PKG_NUM_EVENTS              10
static int   pkg_events[PKG_NUM_EVENTS]        = {PKG_ENERGY, PKG_MAX_ENERGY_RANGE, PKG_MAX_POWER_A, PKG_POWER_LIMIT_A, PKG_TIME_WINDOW_A, PKG_MAX_POWER_B, PKG_POWER_LIMIT_B, PKG_TIME_WINDOW_B, PKG_ENABLED, PKG_NAME};
static char *pkg_event_names[PKG_NUM_EVENTS]   = {"ENERGY_UJ", "MAX_ENERGY_RANGE_UJ", "MAX_POWER_A_UW", "POWER_LIMIT_A_UW", "TIME_WINDOW_A_US", "MAX_POWER_B_UW", "POWER_LIMIT_B_UW", "TIME_WINDOW_B", "ENABLED", "NAME"};
static char *pkg_sys_names[PKG_NUM_EVENTS]     = {"energy_uj", "max_energy_range_uj", "constraint_0_max_power_uw", "constraint_0_power_limit_uw", "constraint_0_time_window_us", "constraint_1_max_power_uw", "constraint_1_power_limit_uw", "constraint_1_time_window_us", "enabled", "name"};
static mode_t   pkg_sys_flags[PKG_NUM_EVENTS]  = {O_RDONLY, O_RDONLY, O_RDONLY, O_RDWR, O_RDONLY, O_RDONLY, O_RDWR, O_RDONLY, O_RDONLY, O_RDONLY};


// POWERCAP

#define COMPONENT_ENERGY            10
#define COMPONENT_MAX_ENERGY_RANGE  11
#define COMPONENT_MAX_POWER_A       12
#define COMPONENT_POWER_LIMIT_A     13
#define COMPONENT_TIME_WINDOW_A     14
#define COMPONENT_ENABLED           15
#define COMPONENT_NAME              16

#define COMPONENT_NUM_EVENTS        7
static int   component_events[COMPONENT_NUM_EVENTS]      = {COMPONENT_ENERGY, COMPONENT_MAX_ENERGY_RANGE, COMPONENT_MAX_POWER_A, COMPONENT_POWER_LIMIT_A, COMPONENT_TIME_WINDOW_A, COMPONENT_ENABLED, COMPONENT_NAME};
static char *component_event_names[COMPONENT_NUM_EVENTS] = {"ENERGY_UJ", "MAX_ENERGY_RANGE_UJ", "MAX_POWER_A_UW", "POWER_LIMIT_A_UW", "TIME_WINDOW_A_US", "ENABLED", "NAME"};
static char *component_sys_names[COMPONENT_NUM_EVENTS]         = {"energy_uj", "max_energy_range_uj", "constraint_0_max_power_uw", "constraint_0_power_limit_uw", "constraint_0_time_window_us", "enabled", "name"};
static mode_t   component_sys_flags[COMPONENT_NUM_EVENTS]      = {O_RDONLY, O_RDONLY, O_RDONLY, O_RDWR, O_RDONLY, O_RDONLY, O_RDONLY};

#define POWERCAP_MAX_COUNTERS (2 * (PKG_NUM_EVENTS + (3 * COMPONENT_NUM_EVENTS)))


