//
//  configops.h
//  Phase1
//
//  Created by Qumber Zaidi on 2/22/22.
//

#ifndef CONFIGOPS_H
#define CONFIGOPS_H

#include "StringUtils.h"

typedef enum
{
    CFG_FILE_ACCESS_ERR = 3,
    CFG_CORRUPT_DESCRIPTOR_ERR,
    CFG_DATA_OUT_OF_RANGE_ERR,
    CFG_CORRUPT_PROMPT_ERR,
    CFG_VERSION_CODE,
    CFG_MD_FILE_NAME_CODE,
    CFG_CPU_SCHED_CODE,
    CFG_QUANT_CYCLES_CODE,
    CFG_MEM_AVAILABLE_CODE,
    CFG_PROC_CYCLES_CODE,
    CFG_IO_CYCLES_CODE,
    CFG_LOG_TO_CODE,
    CFG_LOG_FILE_NAME_CODE } ConfigCodeMesssages;

typedef enum
{
    CPU_SCHED_SJF_N_CODE,
    CPU_SCHED_STRF_P_CODE,
    CPU_SCHED_FCFS_P_CODE,
    CPU_SCHED_RR_P_CODE,
    CPU_SCHED_FCFS_N_CODE,
    LOGTO_MONITOR_CODE,
    LOGTO_FILE_CODE,
    LOGTO_BOTH_CODE } ConfigDataCodes;


ConfigDataType *clearConfigData(ConfigDataType *configData);
void configCodeToString(int code, char *outString);
void displayConfigData(ConfigDataType *configData);
Boolean getConfigData( char *fileName, ConfigDataType **configData,char *endStateMsg);
ConfigDataCodes getCpuSchedCode( char *codeStr);
ConfigDataCodes getLogToCode(char *logToStr);
Boolean valueInRange(int lineCode, int intVal, double doubleVal, char *stringVal);
int getDataLineCode(char *dataBuffer);

#endif /* configops_h */
