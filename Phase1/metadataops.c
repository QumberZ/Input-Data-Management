//
//  metadataops.c
//  Phase1
//
//  Created by Qumber Zaidi on 2/23/22.
//

#include "metadataops.h"


const int BAD_ARG_VAL = -1;

OpCodeType *addNode(OpCodeType * localPtr, OpCodeType *newNode)
{
    if (localPtr == NULL)
    {
        localPtr = (OpCodeType *) malloc(sizeof(OpCodeType));
        
        localPtr-> pid = newNode-> pid;
        copyString(localPtr-> command, newNode-> command);
        copyString(localPtr-> inOutArg, newNode-> inOutArg);
        copyString(localPtr-> strArg1, newNode-> strArg1);
        localPtr-> intArg2 = newNode-> intArg2;
        localPtr-> intArg3 = newNode-> intArg3;
        localPtr-> opEndTime = newNode-> opEndTime;
        localPtr-> nextNode = NULL;
        
        return localPtr;
    }
    localPtr-> nextNode = addNode(localPtr-> nextNode, newNode);
    
    return localPtr;
}



OpCodeType *clearMetaDataList(OpCodeType *localPtr)
{
    if (localPtr != NULL)
    {
        clearMetaDataList(localPtr-> nextNode);
        
        free(localPtr);
        
        localPtr = NULL;
    }
    
    return NULL;
}

void displayMetaData(OpCodeType *localPtr)
{
    
    printf("Meta-Data File Display\n");
    printf("---------------------\n\n");
    
    while (localPtr != NULL)
    {
        printf("Op Code: ");
        printf("/pid: %d", localPtr->pid);
        printf("/cmd: %s", localPtr->command);
        
        if(compareString(localPtr-> command, "dev") == STR_EQ)
        {
            printf("/io: %s", localPtr ->inOutArg);
       }
        
        else
        {
            printf("/io: NA");
        }
        
        printf("\n\t /arg1: %s", localPtr->strArg1);
        
        printf("/arg 2: %d", localPtr-> intArg2);
        
        printf("/arg 3: %d", localPtr-> intArg3);
        
        printf("/op end time: %8.6f" , localPtr-> opEndTime);
        
        printf("\n\n");
        
        localPtr = localPtr-> nextNode;
    }
}

int getCommand(char *cmd, char *inputStr, int index)
{
    int lengthOfCommand = 3;
    
    while (index < lengthOfCommand)
    {
        cmd[index] = inputStr[index];
        
        index++;
        
        cmd[index] = NULL_CHAR;
    }
    return index;
    
}




Boolean getMetaData(char *fileName, OpCodeType **opCodeDataHead, char *endStateMsg)
{
    const char READ_ONLY_FLAG[] = "r";
    
    int accessResult, startCount = 0, endCount = 0;
    char dataBuffer[MAX_STR_LEN];
    Boolean ignoreLeadingWhiteSpace = True;
    Boolean stopAtNonPrintable = True;
    Boolean returnState = True;
    OpCodeType *newNodePtr;
    OpCodeType *localHeadPtr = NULL;
    FILE *fileAccessPtr;
    
    *opCodeDataHead = NULL;
    
    copyString(endStateMsg, "MetaData file upload successful");
    
    fileAccessPtr = fopen(fileName, READ_ONLY_FLAG);
    
    if (fileAccessPtr == NULL)
    {
        copyString(endStateMsg, "Metadata file access error");
        
        return False;
    }
    
    if (getLineTo(fileAccessPtr, MAX_STR_LEN, COLON,
                  dataBuffer, ignoreLeadingWhiteSpace, stopAtNonPrintable) != NO_ERR
                  ||compareString(dataBuffer, "Start Program Meta-Data Code") != STR_EQ)
    {
        fclose(fileAccessPtr);
        
        copyString(endStateMsg, "Corrupt metadata leader line error");
        
        return False;
    }
    
    newNodePtr = (OpCodeType *) malloc(sizeof(OpCodeType));
    
    accessResult = getOpCommand(fileAccessPtr, newNodePtr);
    
    startCount = updateStartCount(startCount, newNodePtr-> strArg1);
    endCount = updateEndCount(endCount, newNodePtr->strArg1);
    
    
    if (accessResult != COMPLETE_OPCMD_FOUND_MSG)
    {
        printf("Theres a failure in first op command\n");
        
        fclose(fileAccessPtr);
        
        *opCodeDataHead = clearMetaDataList(localHeadPtr);
        
        free(newNodePtr);
        
        copyString(endStateMsg, "Metadata incomplete first op command found");
        
        return False;
    }
    
    while (accessResult == COMPLETE_OPCMD_FOUND_MSG)
    {
        localHeadPtr = addNode(localHeadPtr, newNodePtr);
        
        accessResult = getOpCommand(fileAccessPtr, newNodePtr);
        
        startCount = updateStartCount(startCount, newNodePtr-> strArg1);
        endCount = updateEndCount(endCount, newNodePtr-> strArg1);
    }

    if (accessResult == LAST_OPCMD_FOUND_MSG)
    {
        if (startCount == endCount)
        {
            localHeadPtr = addNode(localHeadPtr, newNodePtr);
            
            accessResult = NO_ERR;
            
            if (getLineTo(fileAccessPtr, MAX_STR_LEN, PERIOD,
                          dataBuffer, ignoreLeadingWhiteSpace, stopAtNonPrintable) != NO_ERR
                ||compareString(dataBuffer, "End Program Meta-Data Code") != STR_EQ)
            {
                accessResult = MD_CORRUPT_DESCRIPTOR_ERR;
                
                
                copyString(endStateMsg, "Metadata corrupted descriptor error");
            }
        }
    }
    
    else
    {
        copyString(endStateMsg, "Corrupted metadata op code");
        
        returnState = False;
    }
    
    if (accessResult != NO_ERR)
    {
        localHeadPtr = clearMetaDataList(localHeadPtr);
    }
    
    fclose(fileAccessPtr);
    
    free(newNodePtr);
    
    *opCodeDataHead = localHeadPtr;
    
    return returnState;
    
    
}


int getOpCommand(FILE *filePtr, OpCodeType *inData)
{
    
    
    const int MAX_CMD_LENGTH = 5;
    const int MAX_ARG_STR_LENGTH = 15;
    
    
    int accessResult, numBuffer = 0;
    char strBuffer[STD_STR_LEN];
    char cmdBuffer[MAX_CMD_LENGTH];
    char argStrBuffer[MAX_ARG_STR_LENGTH];
    int runningStringIndex = 0;
    Boolean stopAtNonPrintable = True;
    Boolean arg2FailureFlag = False;
    Boolean arg3FailureFlag = False;
    
    
    accessResult = getLineTo(filePtr, STD_STR_LEN, SEMICOLON,
                             strBuffer, IGNORE_LEADING_WS, stopAtNonPrintable);
    
    
    if (accessResult == NO_ERR)
    {
        runningStringIndex = getCommand(cmdBuffer,
                                        strBuffer, runningStringIndex);
        
        copyString(inData-> command, cmdBuffer);
    }
    else
    {
        inData = NULL;
        
        return OPCMD_ACCESS_ERR;
    }
    
    
    if (verifyValidCommand(cmdBuffer) == False)
    {
        return CORRUPT_OPCMD_ERR;
    }
    
    
    inData->pid = 0;
    inData->inOutArg[0] = NULL_CHAR;
    inData-> intArg2 = 0;
    inData-> intArg3 = 0;
    inData-> opEndTime = 0.0;
    inData-> nextNode = NULL;
    
    if (compareString(cmdBuffer, "dev") == STR_EQ)
    {
        runningStringIndex = getStringArg(argStrBuffer,
                                          strBuffer, runningStringIndex);
        
        
        copyString(inData->inOutArg, argStrBuffer);
        
        if (compareString(argStrBuffer, "in") != STR_EQ
            && compareString(argStrBuffer, "out") != STR_EQ)
        {
            return CORRUPT_OPCMD_ARG_ERR;
        }
    }
    
    runningStringIndex = getStringArg(argStrBuffer,
                                      strBuffer, runningStringIndex);
    
    
    copyString(inData-> strArg1, argStrBuffer);
    
    if (verifyFirstStringArg(argStrBuffer) == False)
    {
        return CORRUPT_OPCMD_ARG_ERR;
    }
    
    if (compareString(inData-> command, "sys") == STR_EQ
        && compareString(inData-> strArg1, "end") == STR_EQ)
    {
        return LAST_OPCMD_FOUND_MSG;
    }
    
    if (compareString(inData-> command, "app") == STR_EQ
        && compareString(inData-> strArg1, "start") == STR_EQ)
    {
        runningStringIndex = getNumberArg(&numBuffer,
                                          strBuffer, runningStringIndex);
        
        if (numBuffer <= BAD_ARG_VAL)
        {
            arg2FailureFlag = True;
        }
        
        inData-> intArg2 = numBuffer;
    }
    
    else if (compareString(inData->command, "cpu") == STR_EQ)
    {
        
        runningStringIndex = getNumberArg(&numBuffer,
                                          strBuffer, runningStringIndex);
        
        if (numBuffer <= BAD_ARG_VAL)
        {
            arg2FailureFlag = True;
        }
        
        inData-> intArg2 = numBuffer;
        
    }
    
    
    else if (compareString(inData->command, "dev") == STR_EQ)
    {
        
        runningStringIndex = getNumberArg(&numBuffer, strBuffer, runningStringIndex);
        
        if (numBuffer <= BAD_ARG_VAL)
        {
            arg2FailureFlag = True;
        }
        
        inData-> intArg2 = numBuffer;
        
    }
    
    else if (compareString(inData->command, "mem") == STR_EQ)
    {
        
        runningStringIndex = getNumberArg(&numBuffer,
                                          strBuffer, runningStringIndex);
        
        if (numBuffer <= BAD_ARG_VAL)
        {
            arg2FailureFlag = True;
        }
        
        inData-> intArg2 = numBuffer;
        
        runningStringIndex = getNumberArg(&numBuffer,
                                          strBuffer, runningStringIndex);
        
        if (numBuffer <= BAD_ARG_VAL)
        {
            arg3FailureFlag = True;
        }
        
        inData-> intArg3 = numBuffer;
        
    }
    
    
    if (arg2FailureFlag == True || arg3FailureFlag == True)
    {
        return CORRUPT_OPCMD_ARG_ERR;
    }
    
    return COMPLETE_OPCMD_FOUND_MSG;
        
    }



    
int getNumberArg(int *number, char *inputStr, int index)
{
    Boolean foundDigit = False;
    *number = 0;
    int multiplier = 1;
    
    while (inputStr[index] <= SPACE || inputStr[index] == COMMA)
    {
        index++;
    }
    
    while (isDigit(inputStr[index]) == True
                                 &&inputStr[index] != NULL_CHAR)
    {
        foundDigit = True;
        
        (*number) = (*number) * multiplier + inputStr[index] - '0';
        
        index++; multiplier = 10;
    }
    
    if (foundDigit == False)
    {
        *number = BAD_ARG_VAL;
    }
    return index;
}




int getStringArg(char *strArg, char *inputStr, int index)
{
    int localIndex = 0;
    
    while (inputStr[index] <= SPACE || inputStr[index] == COMMA)
    {
        index++;
    }
    
    while (inputStr[index] != COMMA && inputStr[index] != NULL_CHAR)
    {
        strArg[localIndex] = inputStr[index];
        
        index++; localIndex++;
        
        strArg[localIndex] = NULL_CHAR;
    }
    return index;
}

Boolean isDigit(char testChar)
{
    if (testChar >= '0' && testChar <= '9')
    {
        return True;
    }
    
    return False;
}

int updateEndCount(int count, char *opString)
{
    if (compareString(opString, "end") == STR_EQ)
    {
        return count + 1;
    }
    return count;
}


int updateStartCount(int count, char *opString)
{
    if (compareString(opString, "start") == STR_EQ)
    {
        return count + 1;
    }
    return count;
}



Boolean verifyFirstStringArg(char *strArg)
{
    if (compareString(strArg, "access") == STR_EQ
        || compareString(strArg, "allocate") == STR_EQ
        || compareString(strArg, "end") == STR_EQ
        || compareString(strArg, "ethernet") == STR_EQ
        || compareString(strArg, "hard drive") == STR_EQ
        || compareString(strArg, "keyboard") == STR_EQ
        || compareString(strArg, "monitor") == STR_EQ
        || compareString(strArg, "printer") == STR_EQ
        || compareString(strArg, "process") == STR_EQ
        || compareString(strArg, "serial") == STR_EQ
        || compareString(strArg, "sound signal") == STR_EQ
        || compareString(strArg, "start") == STR_EQ
        || compareString(strArg, "usb") == STR_EQ
        || compareString(strArg, "video signal") == STR_EQ)
    {
        return True;
    }
    return False;
}


Boolean verifyValidCommand(char *testCmd)
{
    if (compareString(testCmd, "sys") == STR_EQ
        || compareString(testCmd, "app") == STR_EQ
        || compareString(testCmd, "cpu") == STR_EQ
        || compareString(testCmd, "mem") == STR_EQ
        || compareString(testCmd, "dev") == STR_EQ)
    {
        return True;
    }
    return False;
}
