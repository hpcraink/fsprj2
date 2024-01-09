//
// Created by andreas on 11.12.23.
//

#ifndef POWERMEASUREMENT_DEFINES_H
#define POWERMEASUREMENT_DEFINES_H

#define MAX_STR_LEN 100
#define CPU_INTEL 6
#define CPU_AMD 23


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
    unsigned int *cpuIds;
    unsigned int numberCPUCount;
} CpuPackage;

typedef struct CpuInfo {
    CpuPackage *cpuPackages;
    unsigned int cpuCount;
    unsigned int packageCount;
} CpuInfo;

typedef struct FileState {
    int fileDescriptor;
    int open;
} FileState;

typedef struct CPUMeasurementTask {
    char name[MAX_STR_LEN];
    char description[MAX_STR_LEN];
    unsigned int offsetInFile;
    unsigned int type;

    unsigned int cpuPackage;
    unsigned int cpuId;

    long long lastValue;
} CPUMeasurementTask;
