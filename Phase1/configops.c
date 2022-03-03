//
//  configops.c
//  Phase1
//
//  Created by Qumber Zaidi on 2/22/22.
//

#include "configops.h"

ConfigDataType *clearConfigData(ConfigDataType *configData)
{
    if(configData != NULL)
    {
        free(configData);
        
        configData = NULL;
    }
    return NULL;
}

void configCodeToString(int code, char *outString)
{
    char displayStrings[8][10] = { "SJF-N", "SRTF-P", "FCFS-P",
                                   "RR-P", "FCFS-N", "Monitor",
        "File", "Both"};
    
    copyString(outString, displayStrings[code]);
}


void displayConfigData(ConfigDataType *configData)
{
    char displayString[STD_STR_LEN];
    
    printf("Config File Display\n");
    printf("-------------------\n");
    printf("Version               : %3.2f\n", configData-> version);
    printf("Program file name     : %s\n", configData-> metaDataFileName);
    configCodeToString(configData-> cpuSchedCode, displayString);
    printf("CPU schedule selection: %s\n", displayString);
    printf("Quantum time          : %d\n", configData-> quantumCycles);
    printf("Memory Available      : %d\n", configData-> memAvailable );
    printf("Process cycle rate    : %d\n", configData-> procCycleRate);
    printf("I/O cycle rate        : %d\n", configData-> ioCycleRate);
    configCodeToString(configData-> logToCode, displayString);
    printf("Log to selection      : %s\n", displayString);
    printf("Log file name         : %s\n\n", configData-> logToFileName);
}

Boolean getConfigData( char *fileName, ConfigDataType **configData,
                                     char *endStateMsg)
{
    const int NUM_DATA_LINES = 9;
    
    const char READ_ONLY_FLAG[] = "r";
    
    ConfigDataType *tempData;
    
    FILE *fileAccessPtr;
    char dataBuffer[MAX_STR_LEN];
    int intData = 0, dataLineCode, lineCtr = 0;
    double doubleData = 0.0;
    Boolean dontStopAtNonPrintable = False;
    
    copyString(endStateMsg, "Configuration file upload successful");
    
    *configData = NULL;
    
    fileAccessPtr = fopen(fileName, READ_ONLY_FLAG);
    
    if(fileAccessPtr == NULL)
    {
        copyString(endStateMsg, "Configuration file access error");
        
        return False;
        
    }
    
    if(getLineTo(fileAccessPtr, MAX_STR_LEN, COLON,
                 dataBuffer, IGNORE_LEADING_WS, dontStopAtNonPrintable) != NO_ERR
       || compareString(dataBuffer, "Start Simulator Configuration File") != STR_EQ)
    {
        fclose(fileAccessPtr);
        
        copyString(endStateMsg, "Corrupt configuration leader liner error");
        return False;
    }
    
    tempData = (ConfigDataType *) malloc(sizeof(ConfigDataType));
    
    while(lineCtr < NUM_DATA_LINES)
    {
        
        if(getLineTo(fileAccessPtr, MAX_STR_LEN, COLON ,
                     dataBuffer, IGNORE_LEADING_WS, dontStopAtNonPrintable) != NO_ERR)
        {
            free(tempData);
            
            fclose(fileAccessPtr);
            
            copyString(endStateMsg, "Configuration start line capture error");
            
            return False;
        }
        
        dataLineCode = getDataLineCode(dataBuffer);
        
        if(dataLineCode != CFG_CORRUPT_PROMPT_ERR)
        {
            
            if(dataLineCode == CFG_VERSION_CODE)
            {
                
                fscanf(fileAccessPtr, "%lf", &doubleData);
            }
            
            else if (dataLineCode == CFG_MD_FILE_NAME_CODE
                     || dataLineCode == CFG_LOG_FILE_NAME_CODE
                     || dataLineCode == CFG_CPU_SCHED_CODE
                     || dataLineCode == CFG_LOG_TO_CODE)
            {
                fscanf(fileAccessPtr, "%s", dataBuffer);
            }
            
            else{
                fscanf(fileAccessPtr, "%d", &intData);
            }
            
            if(valueInRange(dataLineCode, intData, doubleData, dataBuffer)
                                                                    == True)
            {
                switch(dataLineCode)
                {
                    case CFG_VERSION_CODE:
                        tempData-> version = doubleData;
                        break;
                        
                    case CFG_MD_FILE_NAME_CODE:
                        copyString(tempData-> metaDataFileName, dataBuffer);
                        break;
                        
                    case CFG_CPU_SCHED_CODE:
                        tempData-> cpuSchedCode= getCpuSchedCode(dataBuffer);
                        break;
                        
                    case CFG_QUANT_CYCLES_CODE:
                        tempData-> quantumCycles = intData;
                        break;
                        
                    case CFG_MEM_AVAILABLE_CODE:
                        tempData-> memAvailable = intData;
                        break;
                        
                    case CFG_IO_CYCLES_CODE:
                        tempData-> ioCycleRate = intData;
                        break;
                        
                    case CFG_LOG_TO_CODE:
                        tempData-> logToCode = getLogToCode(dataBuffer);
                        break;
                        
                    case CFG_LOG_FILE_NAME_CODE:
                        copyString(tempData -> logToFileName, dataBuffer);
                        break;
                }
            }
            
            else
            {
                free(tempData);
                
                fclose(fileAccessPtr);
                
                copyString(endStateMsg, "Configuration item out of range");
                
                return False;
            }
        }
        
        else
        {
            free(tempData);
            
            fclose(fileAccessPtr);
            
            copyString(endStateMsg, "Corrupted configuration prompt");
            
            return False;
        }
        
        lineCtr++;
    }
    
    if (getLineTo(fileAccessPtr , MAX_STR_LEN, PERIOD,
                  dataBuffer, IGNORE_LEADING_WS, dontStopAtNonPrintable) != NO_ERR
                   ||compareString(dataBuffer, "End Simulator Configuration File")
                                                                        != STR_EQ)
         {
             free(tempData);
             
             fclose(fileAccessPtr);
             
             copyString(endStateMsg, "Configuration end line capture error");
             
             return False;
    }
    
    *configData = tempData;
    
    fclose(fileAccessPtr);
    
    return True;
}

ConfigDataCodes getCpuSchedCode( char *codeStr)
{
    
    int strLen = getStringLength(codeStr);
    char *tempStr = (char *) malloc(strLen + 1);
    
    int returnVal = CPU_SCHED_FCFS_N_CODE;
    
    setStrToLowerCase(tempStr, codeStr);
    
    if(compareString (tempStr, "sjf-n") == STR_EQ)
    {
        returnVal = CPU_SCHED_SJF_N_CODE;
    }
    
    if(compareString(tempStr, "srtf-p") == STR_EQ)
    {
        returnVal = CPU_SCHED_STRF_P_CODE;
    }
    
    if(compareString(tempStr, "fcfs-p") == STR_EQ)
    {
        returnVal = CPU_SCHED_FCFS_P_CODE;
    }
    
    if(compareString(tempStr, "rr-p") == STR_EQ)
    {
        returnVal = CPU_SCHED_RR_P_CODE;
    }
    
    free(tempStr);
    
    return returnVal;
    
}


int getDataLineCode(char *dataBuffer)
{
    if (compareString(dataBuffer, "Version/Phase") == STR_EQ)
    {
        return CFG_VERSION_CODE;
    }
    
    if (compareString(dataBuffer, "File Path") == STR_EQ)
    {
        return CFG_MD_FILE_NAME_CODE;
    }
    
    if (compareString(dataBuffer, "CPU Scheduling Code") == STR_EQ)
    {
        return CFG_CPU_SCHED_CODE;
    }
    
    if (compareString(dataBuffer, "Quatum Time (cycles)") == STR_EQ)
    {
        return CFG_QUANT_CYCLES_CODE;
    }
    
    if (compareString(dataBuffer, "Memory Available (KB)") == STR_EQ)
    {
        return CFG_MEM_AVAILABLE_CODE;
    }
    
    if (compareString(dataBuffer, "Processor Cycle Time (msec)") == STR_EQ)
    {
        return CFG_PROC_CYCLES_CODE;
    }
    
    if (compareString(dataBuffer, "I/O Cycle Time (msec)") == STR_EQ)
    {
        return CFG_IO_CYCLES_CODE;
    }
    
    if (compareString(dataBuffer, "Log To") == STR_EQ)
    {
        return CFG_LOG_TO_CODE;
    }
    
    if (compareString(dataBuffer, "Log File Path") == STR_EQ)
    {
        return CFG_LOG_FILE_NAME_CODE;
    }
    return CFG_CORRUPT_PROMPT_ERR;
}

ConfigDataCodes getLogToCode(char *logToStr)
{
    int strLen = getStringLength(logToStr);
    char *tempStr = (char *)malloc(strLen + 1);
    
    int returnVal = LOGTO_MONITOR_CODE;
    
    setStrToLowerCase (tempStr, logToStr);
    
    if (compareString(tempStr, "both") == STR_EQ)
    {
        returnVal = LOGTO_BOTH_CODE;
    }
    
    if (compareString(tempStr, "file") == STR_EQ)
    {
        returnVal = LOGTO_FILE_CODE;
    }
    
    free(tempStr);
    
    return returnVal;
}

Boolean valueInRange(int lineCode, int intVal, double doubleVal, char *stringVal)
{
    
    Boolean result = True;
    char *tempStr;
    int strLen;
    
    switch(lineCode)
    {
            
        case CFG_VERSION_CODE:
            
            if(doubleVal < 0.00 || doubleVal > 10.0)
            {
                result = False;
            }
            break;
            
        case CFG_CPU_SCHED_CODE:
            
            strLen = getStringLength(stringVal);
            tempStr = (char *)malloc(strLen + 1);
            setStrToLowerCase(tempStr, stringVal);
            
            if( compareString(tempStr, "fcfs-n") != STR_EQ
               &&compareString(tempStr, "sjf-n") != STR_EQ
               &&compareString(tempStr, "srtf-p") != STR_EQ
               &&compareString(tempStr, "fcfs-p") != STR_EQ
               &&compareString(tempStr, "rr-p") != STR_EQ)
            {
                result = False;
            }
            
            free(tempStr);
            break;
            
        case CFG_MEM_AVAILABLE_CODE:
            
            if(intVal < 1024 || intVal > 102400)
            {
                result = False;
            }
            break;
            
        case CFG_PROC_CYCLES_CODE:
            
            if(intVal < 1 || intVal > 100)
            {
                
                result = False;
            }
            break;
            
        case CFG_IO_CYCLES_CODE:
            
            if(intVal < 1 || intVal > 1000)
            {
                
                result = False;
            }
            break;
            
        case CFG_LOG_TO_CODE:
            
            strLen = getStringLength(stringVal);
            tempStr = (char *)malloc(strLen + 1);
            setStrToLowerCase(tempStr, stringVal);
            
            if ( compareString(tempStr, "both") != STR_EQ
                &&compareString(tempStr, "monitor") != STR_EQ
                &&compareString(tempStr, "file") != STR_EQ)
            {
                result = False;
            }
            
            free(tempStr);
            
            break;

    }
    
    return result;
}
