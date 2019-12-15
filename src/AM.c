#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AM.h"
#include "bf.h"

int AM_errno = AME_OK;
int BF_errno = BF_OK;
FilesMap filesMap;
ScansMap scansMap;

#define CALL_BF(call)       \
{                           \
  BF_errno = call;          \
  if (BF_errno != BF_OK) {  \
    AM_errno = AME_BF_ERROR;\
    return AME_BF_ERROR;    \
  }                         \
}

void AM_Init() {
printf("+AM_Init: just got called.\n");
  AM_errno = AME_OK;
  BF_Init(LRU);
  // Initialize FileMap
  filesMap.filesCounter = 0;
  for (int i=0;i<MAX_OPEN_FILES;i++){
    //filesMap.filesInfo[i].fileId == -1: means empty file position
    filesMap.filesInfo[i].fileId = -1;
  }
  // Initialize ScansMap
  scansMap.scansCounter = 0;
  for (int i=0;i<MAX_OPEN_SCANS;i++){
    //scansMap.scansInfo[i].fileIndex == -1: means empty scan position
    scansMap.scansInfo[i].fileIndex = -1;
  }

  // Test me daddy! ahhhh yeah test me hard!!
// Get sure that we don't have to put anything in close
// Use print_Errors for the next function calls
  printf("++Starting test\n");
// Test Opens(over maximum and the same file multiple times)
// Test Closes (close already closed file, or not existed file, close multiple files)
// Open Scan, test scans (over maximum scans, the same file scan)
// Test Scan Closes (close already closed scan, or not existed scan, close multiple scans)
// Combine the above and destroy or close files that have opened scans, or have terminated at least one scan

   AM_CreateIndex("mytest.db", 'f', 4, 'c', 12);
   AM_PrintError(NULL);
   AM_CreateIndex("mytest.db", 'f', 4, 'c', 12);
   AM_PrintError(NULL);
   AM_DestroyIndex("mytest.db");
   AM_PrintError(NULL);
   AM_DestroyIndex("mytest.db");
   AM_PrintError(NULL);
   AM_CreateIndex("mytest.db", 'f', 4, 'c', 12);
   AM_PrintError(NULL);
   int index = -324;
   index = AM_OpenIndex("mytest.db");
   AM_PrintError(NULL);
   printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
   index = AM_OpenIndex("mytest.db");
   AM_PrintError(NULL);
   printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
   index = AM_OpenIndex("mytest.db");
   AM_PrintError(NULL);
   printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
   AM_CloseIndex(index);
   AM_PrintError(NULL);
   printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
   printf("++Test Finished\n---------------------------\n");
}


int AM_CreateIndex(char *fileName, char attrType1, int attrLength1, char attrType2, int attrLength2) {
printf("+AM_CreateIndex: just got called.\n");
  AM_errno = AME_OK;
  // Creating the BF file
  CALL_BF(BF_CreateFile(fileName))
  // Initialize the BF file by allocating the first block where the information will be stored
  int fd;
  CALL_BF(BF_OpenFile(fileName, &fd))
  // Allocate first Block
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_AllocateBlock(fd,block))
  // Initialize first Block of the B-Plus file
  B_PLUS_FILE_INDICATOR_TYPE BplusFileIndicator = B_PLUS_FILE_INDICATOR;
  char *data = BF_Block_GetData(block);
  memcpy(data, &BplusFileIndicator, B_PLUS_FILE_INDICATOR_SIZE);
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE, &attrType1, sizeof(char));
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+sizeof(char), &attrLength1, sizeof(int));
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+sizeof(char)+sizeof(int), &attrType2, sizeof(char));
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+sizeof(int), &attrLength2, sizeof(int));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block))
  BF_CloseFile(fd);
  BF_Block_Destroy(&block);
  return AME_OK;
}


int AM_DestroyIndex(char *fileName) {
printf("+AM_DestroyIndex: just got called.\n");
  AM_errno = AME_OK;
  // Search the file in the filesMap
  int i = 0;
  while (i < filesMap.filesCounter && strcmp(filesMap.filesInfo[i].fileName, fileName) != 0) i++;
  if(i != filesMap.filesCounter){
    AM_errno = AME_OPENED_FILE;
    return AME_OPENED_FILE;
  }
  // Delete the file and check if that deleted successfully
  if (remove(fileName) != 0) {
    AM_errno = AME_UNABLE_TO_DELETE_FILE;
    return AME_UNABLE_TO_DELETE_FILE;
  }
  return AME_OK;
}


int AM_OpenIndex (char *fileName) {
printf("+AM_OpenIndex: just got called.\n");
  AM_errno = AME_OK;
  // Check if there is free space in filesMap
  if (filesMap.filesCounter >= MAX_OPEN_FILES){
    AM_errno = AME_OPEN_FILES_LIMIT_ERROR;
    return AME_OPEN_FILES_LIMIT_ERROR;
  }
  // Open the file in the filesMap
  int fileIndex;
  // Search an empty position
  for (fileIndex=0; filesMap.filesInfo[fileIndex].fileId != -1; fileIndex++);
  CALL_BF(BF_OpenFile(fileName, &(filesMap.filesInfo[fileIndex].fileId)))

  // Check if the file is B-Plus-File
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(filesMap.filesInfo[fileIndex].fileId, 0, block))
  B_PLUS_FILE_INDICATOR_TYPE indicator;
  char* data = BF_Block_GetData(block);
  memcpy(&indicator, data, B_PLUS_FILE_INDICATOR_SIZE);
  // Check if the file is B-Plus-File
  if (!B_PLUS_FILE_INDICATOR_COMPARE(indicator)){
    filesMap.filesInfo[fileIndex].fileId = -1;
    AM_errno = AME_TYPE_ERROR;
    return AME_TYPE_ERROR;
  }
  // Initialize the index of the filesMap where the file will be saved, with the file information
  filesMap.filesInfo[fileIndex].fileName = fileName;
  filesMap.filesInfo[fileIndex].openedScans = 0;
  // Increase the filesCounter
  filesMap.filesCounter++;

  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  return fileIndex;
}


int AM_CloseIndex (int fileDesc) {
printf("+AM_CloseIndex: just got called.\n");
  AM_errno = AME_OK;
  // Check if the file is closed
  if(filesMap.filesInfo[fileDesc].fileId == -1){
    AM_errno = AME_INVALID_FILE_ERROR;
    return AME_INVALID_FILE_ERROR;
  }
  // Check if there are opened scans for the file in scansMap
  if (filesMap.filesInfo[fileDesc].openedScans > 0) {
    AM_errno = AME_OPENED_SCANS;
    return AME_OPENED_SCANS;
  }
  // Close BF File
  CALL_BF(BF_CloseFile(filesMap.filesInfo[fileDesc].fileId))
  // Clear the filesMap
  filesMap.filesInfo[fileDesc].fileId = -1;
  // Decrease the filesCounter
  filesMap.filesCounter--;
  return AME_OK;
}


int AM_InsertEntry(int fileDesc, void *value1, void *value2) {
printf("+AM_InsertEntry: just got called.\n");
  AM_errno = AME_OK;
  //?...
  //? EDW ISWS PETHANOYME
  //?
  return AME_OK;
}


int AM_OpenIndexScan(int fileDesc, int op, void *value) {
printf("+AM_OpenIndexScan: just got called.\n");
  AM_errno = AME_OK;
  // Check if there is free space in ScansMap
  if (scansMap.scansCounter >= MAX_OPEN_SCANS){
    AM_errno = AME_OPEN_SCANS_LIMIT_ERROR;
    return AME_OPEN_SCANS_LIMIT_ERROR;
  }
  // Check if the file is closed
  if(filesMap.filesInfo[fileDesc].fileId == -1){
    AM_errno = AME_INVALID_FILE_ERROR;
    return AME_INVALID_FILE_ERROR;
  }
  // Open the scan in the scansMap
  int scanIndex;
  // Search an empty position
  for (scanIndex=0; scansMap.scansInfo[scanIndex].fileIndex != -1; scanIndex++);
  // Initialize the index of the scansMap where the scan will be saved, with the scan information
  scansMap.scansInfo[scanIndex].fileIndex = fileDesc;
  scansMap.scansInfo[scanIndex].foundCounter = 0;
  scansMap.scansInfo[scanIndex].Operator = op;
  // Increase opened scans counter of the file in filesMap
  filesMap.filesInfo[fileDesc].openedScans++;
  // Increase the scansCounter
  scansMap.scansCounter++;
  //?...
  //? Do the first search until tha leaf where the value may be
  //?
  scansMap.scansInfo[scanIndex].blockId = 411563;
  return scanIndex;
}


void *AM_FindNextEntry(int scanDesc) {
printf("+AM_FindNextEntry: just got called.\n");
  AM_errno = AME_OK;
  //?...
  //? Search the next entry that is good, based on our standards boy
  //?  
//   if (there is not other entry which we wanna make love) {
//     AM_errno = AME_EOF;
//     return NULL;
//   }
}


int AM_CloseIndexScan(int scanDesc) {
printf("+AM_CloseIndexScan: just got called.\n");
  AM_errno = AME_OK;
  // Check if the scan is closed
  if (scansMap.scansInfo[scanDesc].fileIndex == -1) {
    AM_errno = AME_INVALID_SCAN_ERROR;
    return AME_INVALID_SCAN_ERROR;
  }
  int fileDesc = scansMap.scansInfo[scanDesc].fileIndex;
  // Check if the file is closed
  if (filesMap.filesInfo[fileDesc].fileId == -1) {
    AM_errno = AME_INVALID_FILE_ERROR;
    return AME_INVALID_FILE_ERROR;
  }  
  // Decrease opened scans counter of the file in filesMap
  filesMap.filesInfo[fileDesc].openedScans--;
  // Clear the scansMap
  scansMap.scansInfo[scanDesc].fileIndex = -1;
  // Decrease the scansCounter
  scansMap.scansCounter--;
  return AME_OK;
}


void AM_PrintError(char *errString) {
  if (errString != NULL)
    printf("%s: ", errString);
  switch(AM_errno) {  
      case AME_OK:
        printf("All good Baby!\n");
        break;
      case AME_EOF:
        printf("AME_EOF: .\n");
        break;
      case AME_OPEN_FILES_LIMIT_ERROR:
        printf("AME_OPEN_FILES_LIMIT_ERROR: .\n");
        //    printf("Error: Has reached the maximum number of files that can be opened\n");
        break;
      case AME_TYPE_ERROR:
        printf("AME_TYPE_ERROR: .\n");
        //    printf("Error: Not a B-Plus-file\n");
        break;
      case AME_INVALID_FILE_ERROR:
        printf("AME_INVALID_FILE_ERROR: .\n");
        break;
      case AME_OPEN_SCANS_LIMIT_ERROR:
        printf("AME_OPEN_SCANS_LIMIT_ERROR: .\n");
        //    printf("Error: Has reached the maximum number of scans that can be opened\n");
        break;
      case AME_OPENED_SCANS:
        printf("AME_OPENED_SCANS: .\n");
        break;
      case AME_OPENED_FILE:
        printf("AME_OPENED_FILE: .\n");
        //    printf("Error: file is opened in the filesMap\n");
        break;
      case AME_INVALID_SCAN_ERROR:
        printf("AME_INVALID_SCAN_ERROR: .\n");
        break;
      case AME_BF_ERROR:
        BF_PrintError(BF_errno);
        break;
      case AME_UNABLE_TO_DELETE_FILE:
        printf("AME_UNABLE_TO_DELETE_FILE: Unable to destroy the file.\n");
        break;
      default:
        printf("Undefined Error!\n");
        break;
  }
}

void AM_Close() {
  // We had not allocated any new memory so AM_Close has no use on our implementation of the exercise.
  // thanx!
  printf("+AM_Close: just got called.\n");
}
