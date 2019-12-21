#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AM.h"
#include "bf.h"
#include "defn.h"
#include "OurFunctions.h"

int insertEntry(FilesInfo fileInfo, int treeNode,void *value1,void *value2, InsertEntry_Return *returnPair);
char *scanForNextNode(FilesInfo fileInfo, char *data, void *value1, int *nextNode);

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

#define CALL_BF_BLOCK_INIT(variable)       \
  BF_Block *variable;                      \
  BF_Block_Init(&variable);                \

#define CALL_BF_BLOCK_DESTROY(variable)    \
  CALL_BF(BF_UnpinBlock(variable))         \
  BF_Block_Destroy(&variable);             \


int compare(FilesInfo fileInfo, void *value1, void *value2) {
    // Check data type to compare
    char dataType = fileInfo.attrType1;

    if (dataType == STRING){
        return strcmp((char*) value1, (char*) value2);
    } else if (dataType == FLOAT) {
        if (*((float*)value1) > *((float*)value2)) return 1;
        else if (*((float*)value1) == *((float*)value2)) return 0;
        else return -1;
    } else{
        return (*(int*)value1 - *(int*)value2);
    }
}

int Create_Data_Block(int fd, int numOfRecords, int nextBlock) {

  char indicator = 'd';                                         
  int numOfBlock = 0;                                            
  CALL_BF_BLOCK_INIT(block)                                        
  CALL_BF(BF_AllocateBlock(fd, block))                           
  char *data = BF_Block_GetData(block);                      
  memcpy(data, &indicator, sizeof(char));                   
  memcpy(data+sizeof(char), &numOfRecords, sizeof(int));  
  memcpy(data+sizeof(char)+sizeof(int), &nextBlock, sizeof(int)); 

  CALL_BF(BF_GetBlockCounter(fd, &numOfBlock))     
  BF_Block_SetDirty(block);                                      
  CALL_BF_BLOCK_DESTROY(block)
  numOfBlock--;
    printf("\n\n***CREATEDD NEWWW DATA BLOKK**%d****\n\n",numOfBlock);
  return  numOfBlock;                        
}

int Create_Index_Block(int fd) {

  char indicator = 'i';                                         
  int numOfKeys = 0;
  int numOfBlock = 0;                                            
  CALL_BF_BLOCK_INIT(block)                                        
  CALL_BF(BF_AllocateBlock(fd, block))                           
  char *data = BF_Block_GetData(block);                      
  memcpy(data, &indicator, sizeof(char));                   
  memcpy(data+sizeof(char), &numOfKeys, sizeof(int));  

  CALL_BF(BF_GetBlockCounter(fd, &numOfBlock))     
    printf("\n\n***CREATEDD NEWWW INDEX BLOKK**%d****\n\n",numOfBlock);
  BF_Block_SetDirty(block);                                      
  CALL_BF_BLOCK_DESTROY(block)
  numOfBlock--;
  return  numOfBlock;                        
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
// // Test Opens(over maximum and the same file multiple times)
// // Test Closes (close already closed file, or not existed file, close multiple files)
// // Open Scan, test scans (over maximum scans, the same file scan)
// // Test Scan Closes (close already closed scan, or not existed scan, close multiple scans)
// // Combine the above and destroy or close files that have opened scans, or have terminated at least one scan

//    AM_CreateIndex("mytest.db", 'i', 4, 'c', 88);
//    AM_PrintError(NULL);
//    AM_CreateIndex("mytest.db", 'i', 4, 'c', 44);
//    AM_PrintError(NULL);
//    AM_DestroyIndex("mytest.db");
//    AM_PrintError(NULL);
//    AM_DestroyIndex("mytest.db");
//    AM_PrintError(NULL);
//    AM_CreateIndex("mytest.db", 'c', 88, 'c', 4);
//    AM_PrintError(NULL);
//    int index = -324;
//    index = AM_OpenIndex("mytest.db");
//    AM_PrintError(NULL);
//    printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
//    index = AM_OpenIndex("mytest.db");
//    AM_PrintError(NULL);
//    printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
//    index = AM_OpenIndex("mytest.db");
//    AM_PrintError(NULL);
//    printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
//    AM_CloseIndex(index);
//    AM_PrintError(NULL);
//    printf("index in filesMap is:%d (Counter = %d).\n", index, filesMap.filesCounter);
//    char* key = "I";
// //    AM_InsertEntry(0, key, "");
// //    AM_PrintError(NULL);
// //    key = "F";
// //    AM_InsertEntry(0, key, "");
// //    key = "G";
// //    AM_InsertEntry(0, key, "");
// //    key = "I";
// //    AM_InsertEntry(0, key, "");
// //    key = "I";
// //    AM_InsertEntry(0, key, "");
// //    key = "JJ";
// //    AM_InsertEntry(0, key, "");
// //    key = "W";
// //    AM_InsertEntry(0, key, "");
// //    key = "Z";
// //    AM_InsertEntry(0, key, "");
// //     key = "ZZ";
// //    AM_InsertEntry(0, key, "");
   
// //    key = malloc(2);
// //    strcpy(key, "a");
// //    for (int i = 0; i < 20; i++){
// //       AM_InsertEntry(0, key, "");
// //         // strcpy(key, &((++key[0])) );
// //         key[0] = 'a' + i;
// //    }
//    key = malloc(2);
//    strcpy(key, "Z");
//    for (int i = 0; i < 11; i++){
//       AM_InsertEntry(0, key, "");
//         // strcpy(key, &((++key[0])) );
//         key[0] = 'Z' - (i+1);
//    }

//    AM_PrintError(NULL);
//   //test the 3 first blocks printing
//   BF_Block *block;
//   BF_Block_Init(&block);
//   int fd = filesMap.filesInfo[0].fileId;
//   char *data;
//   int *intdata;
//   int blocks;
//   char myID;
//   BF_GetBlockCounter(fd,&blocks);
//   for (int i = 1; i < blocks; i++){
//     BF_GetBlock(fd,i,block);
//     printf("i=%d\n",i);
//     data = BF_Block_GetData(block);
//     memcpy(&myID, data, sizeof(char));
//     if(myID == 'i') {
//         printf("block-> %d dataa-> %c \n",i,data[0]);
//         intdata = data;
//         for (int j = 1; j < 512; j+=2){
//         printf("kcolb-> %d data[%d]-> %c||%d      kcolb-> %d data[%d]-> %c||%d\n",i,j,(int)data[j],data[j],i,j+1,(int)data[j+1],data[j+1]);
//         }
//     }
//         printf("-----\n");
//     CALL_BF(BF_UnpinBlock(block))
//   }
//   BF_Block_Destroy(&block);
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
  CALL_BF_BLOCK_INIT(block)
  CALL_BF(BF_AllocateBlock(fd, block))
  // Initialize first Block of the B-Plus file
  B_PLUS_FILE_INDICATOR_TYPE BplusFileIndicator = B_PLUS_FILE_INDICATOR;
  char *data = BF_Block_GetData(block);
  memcpy(data, &BplusFileIndicator, B_PLUS_FILE_INDICATOR_SIZE);
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE,                             &attrType1, sizeof(char));                        /* size of first attribute */
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+sizeof(char),                &attrLength1, sizeof(int));                       /* Length of first attribute */
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+sizeof(char)+sizeof(int),    &attrType2, sizeof(char));                        /* size of second attribute */
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+sizeof(int),  &attrLength2, sizeof(int));                       /* Length of second attribute */
  int rootBlockNum = Create_Index_Block(fd);
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+2*sizeof(int), &rootBlockNum, sizeof(int));                     /* Root of B+ Tree */
  int maxKeysPerBlock = (BF_BLOCK_SIZE - sizeof(char) - sizeof(int)) / (attrLength1 + sizeof(int));
  if (BF_BLOCK_SIZE - sizeof(char) - sizeof(int) - ((attrLength1 + sizeof(int)) * maxKeysPerBlock) < sizeof(int))
    maxKeysPerBlock--;
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+3*sizeof(int), &maxKeysPerBlock, sizeof(int));                  /* maximum keys per index block */
  int maxRecordsPerBlock = (BF_BLOCK_SIZE - sizeof(char) - 2*sizeof(int)) / (attrLength1 + attrLength2);
  memcpy(data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+4*sizeof(int), &maxRecordsPerBlock, sizeof(int));               /* maximum records per data block */  
  BF_Block_SetDirty(block);
  CALL_BF_BLOCK_DESTROY(block)
  BF_CloseFile(fd);
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
  CALL_BF_BLOCK_INIT(block)
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
  memcpy(&(filesMap.filesInfo[fileIndex].attrType1),    data+B_PLUS_FILE_INDICATOR_SIZE,                            sizeof(char));
  memcpy(&(filesMap.filesInfo[fileIndex].attrLength1),  data+B_PLUS_FILE_INDICATOR_SIZE+sizeof(char),               sizeof(int));
  memcpy(&(filesMap.filesInfo[fileIndex].attrType2),    data+B_PLUS_FILE_INDICATOR_SIZE+sizeof(char)+sizeof(int),   sizeof(char));
  memcpy(&filesMap.filesInfo[fileIndex].attrLength2,    data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+sizeof(int), sizeof(int)); 
  filesMap.filesInfo[fileIndex].recordLength = filesMap.filesInfo[fileIndex].attrLength1 + filesMap.filesInfo[fileIndex].attrLength2;
  memcpy(&(filesMap.filesInfo[fileIndex].root), data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+2*sizeof(int), sizeof(int));
  memcpy(&(filesMap.filesInfo[fileIndex].maxKeysPerBlock), data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+3*sizeof(int), sizeof(int));
  memcpy(&(filesMap.filesInfo[fileIndex].maxRecordsPerBlock), data+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+4*sizeof(int), sizeof(int));


  printf("root %d kes per block %d recirds per block %d ",filesMap.filesInfo[fileIndex].root,  
        filesMap.filesInfo[fileIndex].maxKeysPerBlock, filesMap.filesInfo[fileIndex].maxRecordsPerBlock);
  // Increase the filesCounter
  filesMap.filesCounter++;

  CALL_BF_BLOCK_DESTROY(block)
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
  // Check if the file is closed
  if(filesMap.filesInfo[fileDesc].fileId == -1){
    AM_errno = AME_INVALID_FILE_ERROR;
    return AME_INVALID_FILE_ERROR;
  }
  CALL_BF_BLOCK_INIT(root)
  CALL_BF(BF_GetBlock(filesMap.filesInfo[fileDesc].fileId, filesMap.filesInfo[fileDesc].root, root))
  char *data = BF_Block_GetData(root);
  int numOfKeys;
  memcpy(&numOfKeys, data+sizeof(char), sizeof(int));
  if (numOfKeys == 0) { // first record in file
    int rightBlockNum, leftBlockNum;
    rightBlockNum = Create_Data_Block(filesMap.filesInfo[fileDesc].fileId, 0, -1);
    leftBlockNum  = Create_Data_Block(filesMap.filesInfo[fileDesc].fileId, 0, rightBlockNum);

    memcpy(data+sizeof(char)+sizeof(int), &leftBlockNum, sizeof(int));
    memcpy(data+sizeof(char)+2*sizeof(int), value1, filesMap.filesInfo[fileDesc].attrLength1);
    memcpy(data+sizeof(char)+2*sizeof(int)+filesMap.filesInfo[fileDesc].attrLength1, &rightBlockNum, sizeof(int));
    numOfKeys = 1;
    memcpy(data+sizeof(char), &numOfKeys, sizeof(int));
    BF_Block_SetDirty(root);
 
  }
  CALL_BF_BLOCK_DESTROY(root)
    int rootNum = filesMap.filesInfo[fileDesc].root;
    int maxKeysPerBlock = filesMap.filesInfo[fileDesc].maxKeysPerBlock;
    InsertEntry_Return returnPair;
    returnPair.blockPointer = rootNum;
    returnPair.key = malloc(filesMap.filesInfo[fileDesc].attrLength1);

    // printf("1-RETURN_PAIR: block=%d &&& kes = %s (but key = %s)\n", returnPair.blockPointer, (char*)returnPair.key, value1);
    insertEntry(filesMap.filesInfo[fileDesc], rootNum, value1, value2, &returnPair);
    // printf("2-RETURN_PAIR: block=%d &&& kes = %s\n\n", returnPair.blockPointer, (char*)returnPair.key);
        printf("ROOT!!-TIME TO BREAK SOME ASS - key = %c\n",returnPair.key);
    if (returnPair.blockPointer != rootNum) {
        
        int keyLength = filesMap.filesInfo[fileDesc].attrLength1;

        // Create new block for root
        int newRootBlockNum = Create_Index_Block(filesMap.filesInfo[fileDesc].fileId);
        CALL_BF_BLOCK_INIT(newroot)
        CALL_BF(BF_GetBlock(filesMap.filesInfo[fileDesc].fileId, newRootBlockNum, newroot))
        char *newdata = BF_Block_GetData(newroot);

        numOfKeys = 1;
        memcpy(newdata+sizeof(char), &numOfKeys, sizeof(int));
        memcpy(newdata+sizeof(char)+sizeof(int), &rootNum, sizeof(int));
        memcpy(newdata+sizeof(char)+2*sizeof(int), returnPair.key, keyLength);
        memcpy(newdata+sizeof(char)+2*sizeof(int)+keyLength, &returnPair.blockPointer, sizeof(int));

        // This is the new root
        filesMap.filesInfo[fileDesc].root = newRootBlockNum;

        BF_Block_SetDirty(newroot);
        CALL_BF_BLOCK_DESTROY(newroot);
        // Update METADATA Block
        CALL_BF_BLOCK_INIT(block)
        CALL_BF(BF_GetBlock(filesMap.filesInfo[fileDesc].fileId, 0, block))
        char *rootdata = BF_Block_GetData(block);
        memcpy(rootdata+B_PLUS_FILE_INDICATOR_SIZE+2*sizeof(char)+2*sizeof(int), &newRootBlockNum, sizeof(int));
        BF_Block_SetDirty(block);
        CALL_BF_BLOCK_DESTROY(block);
        printf("ROOT!!-I BROKE SOME ASS OUR NEW ROOT IS: %d\n", newRootBlockNum);
    }
    free(returnPair.key);
  //?...
  //? EDW ISWS PETHANOYME
  //?
  //! telika EPIZHSAMEE
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

char *writeToDBuffer(FilesInfo fileInfo,  char *buffer, char *data, void *value1, void *value2){
  // write record to buffer
  int maxNumOfRecords = fileInfo.maxRecordsPerBlock;
  int keyLength       = fileInfo.attrLength1;
  int recordLength    = fileInfo.recordLength;
  int numOfRecords    = maxNumOfRecords;
  int strlenAttr1     = fileInfo.attrLength1;
  int strlenAttr2     = fileInfo.attrLength2;
  if(fileInfo.attrType1 == STRING && strlen(value1) < strlenAttr1)
    strlenAttr1 = strlen(value1)+1;
  if(fileInfo.attrType2 == STRING && strlen(value2) < strlenAttr2)
    strlenAttr2 = strlen(value2)+1;

  memcpy(buffer, data+sizeof(char)+2*sizeof(int), maxNumOfRecords*recordLength);

  //write value1 in the buffer
  int flag = 0;
  int i = 0;
  void *key = malloc(strlenAttr1);
  int offset = 0;

  for(i=0; i<numOfRecords; i++) {
      memcpy(key, buffer+offset, strlenAttr1);
      // If value on record less that value to insert memset the following records
      if (compare(fileInfo, key, value1) > 0) {
        printf("TRIS LALOUN KAI TRIS XOREVOUN move %d\n",(numOfRecords-i)*recordLength);
        memmove(buffer+offset+recordLength, buffer+offset, (numOfRecords-i)*recordLength);  //move data to write in that position
        memset(buffer+offset, 0, recordLength); //flush the moved data space
        //write the new data
        memcpy(buffer+offset, value1, strlenAttr1);
        memcpy(buffer+offset+keyLength, value2, strlenAttr2);
        flag = 1;
        break;
      }
      offset += recordLength;
  }
  if(flag == 0){
      memmove(buffer+offset+recordLength, buffer+offset, (numOfRecords-i)*recordLength);  //move data to write in that position
      memset(buffer+offset, 0, recordLength); //flush the moved data space
      //write the new data
      memcpy(buffer+offset, value1, strlenAttr1);
    //   printf("BUFDFER+OFFST= %c\n",*(buffer+offset));
      memcpy(buffer+offset+keyLength, value2, strlenAttr2);
  }
  free(key);
  return buffer;
}

char *writeToIBuffer(FilesInfo fileInfo,  char *buffer, char *data, void *value, int blockNum){
  // write record to buffer
  int maxNumOfKeys     = fileInfo.maxKeysPerBlock;
  int keyLength        = fileInfo.attrLength1;
  int numOfKeys        = maxNumOfKeys;
  int pairLength       = keyLength + sizeof(int);
  int strlenAttr1      = fileInfo.attrLength1;
  if(fileInfo.attrType1 == STRING && strlen(value) < strlenAttr1)
    strlenAttr1 = strlen(value)+1;


  memcpy(buffer, data+sizeof(char)+sizeof(int), maxNumOfKeys*pairLength + sizeof(int));

  //write value1 in the buffer
  int flag = 0;
  int i = 0;
  void *key = malloc(strlenAttr1);
  int offset = 0;

  for(i=0; i<numOfKeys; i++) {
      memcpy(key, buffer+offset+sizeof(int), strlenAttr1);
      // If value on record less that value to insert memset the following records
      if (compare(fileInfo, key, value) > 0) {
        memmove(buffer+offset+pairLength, data+offset+sizeof(int), (numOfKeys-i)*pairLength);  //move data to write in that position
        memset(buffer+offset, 0, pairLength); //flush the moved data space
        //write the new data
        memcpy(buffer+offset, value, strlenAttr1);
        memcpy(buffer+offset+keyLength, &blockNum, sizeof(int));
        flag = 1;
        break;
      }
      offset += pairLength;
  }
  if(flag == 0){
      offset += sizeof(int);
      memmove(buffer+offset+pairLength, data+offset, (numOfKeys-i)*pairLength);  //move data to write in that position
      memset(buffer+offset, 0, pairLength); //flush the moved data space
      //write the new data
      memcpy(buffer+offset, value, strlenAttr1);
    //   printf("BUFDFER+OFFST= %c\n",*(buffer+offset));
      memcpy(buffer+offset+keyLength, &blockNum, sizeof(int));
  }
  free(key);
  return buffer;
}

int insertEntry(FilesInfo fileInfo, int treeNode,void *value1,void *value2, InsertEntry_Return *returnPair) {
    // Recursive function to insert entry in the treeNode
    // Get the block of the treeNode
    CALL_BF_BLOCK_INIT(block)
    int test;
    BF_GetBlockCounter(fileInfo.fileId,&test);
        printf("datablock %d nums-> %d\n\n",treeNode, test);
    CALL_BF(BF_GetBlock(fileInfo.fileId, treeNode, block))
    char *data = BF_Block_GetData(block);
    // Unpin the block to minimize the count of the opened block(usefull for deep recursive insertings)

    int keyLength     = fileInfo.attrLength1;
    int recordLength  = fileInfo.recordLength;
    int offset;
    int i;
    void *key;
    void *prevkey;
    char indicator;
    memcpy(&indicator, data, sizeof(char));
    // Check the type of block
    if (indicator == 'i') {
        // Block is index block
        int numOfKeys;
        memcpy(&numOfKeys, data+sizeof(char), sizeof(int));
        int nextNode;
        char *nextNodeKeyPosition = scanForNextNode(fileInfo, data, value1, &nextNode);
        int sizeOfPair = keyLength+sizeof(int);
        // printf("Hello to everyOne: %d, %s(numOfByte = %d)\n",nextNode, nextNodeKeyPosition, nextNodeKeyPosition-data);

        InsertEntry_Return newReturnPair;
        newReturnPair.blockPointer = treeNode;
        newReturnPair.key = malloc(keyLength);
        
        
        
//        BF_Block_SetDirty(block);
//        CALL_BF_BLOCK_DESTROY(block)
        
        
        printf("1-RETURN_PAIR: block=%d &&& kes = %s (but key = %s)\n", newReturnPair.blockPointer, (char*)newReturnPair.key, value1);
        insertEntry(fileInfo, nextNode, value1, value2, &newReturnPair);
        printf("2-RETURN_PAIR: block=%d &&& kes = %s\n\n", newReturnPair.blockPointer, (char*)newReturnPair.key);
        if (newReturnPair.blockPointer != treeNode) {
            // CALL_BF_BLOCK_INIT(block)
            // CALL_BF(BF_GetBlock(fileInfo.fileId, treeNode, block))
            // char *data = BF_Block_GetData(block);
            if (numOfKeys >= fileInfo.maxKeysPerBlock) {
                /*
                -Create Buffer
                -Coppy everything in it
                -Put new key+Pointer in it
                -find the middle
                    -right from the middle is a pointer and left a key
                -Change Num of keys in first Block(the keys until middle -1)
                -Create New Block
                -Put everything right from the middle in it (after descripitio part)
                -Change returnPair(remaining key and newBlockNum)
                */
                //Allocate buffer for index block
                char *buffer = malloc((fileInfo.maxKeysPerBlock+1)*(keyLength+sizeof(int)+1*sizeof(int)));
                // Write data to buffer
                buffer = writeToIBuffer(fileInfo, buffer, data, newReturnPair.key, newReturnPair.blockPointer);

                // for (int j = 0; j < (fileInfo.maxKeysPerBlock+1)*(keyLength+sizeof(int)) + sizeof(int); j+=2)
                //     printf("AAAA-> %d data[%d]-> %c||%d      AAAA-> %d data[%d]-> %c||%d\n",100,j,(int)buffer[j],buffer[j],100,j+1,(int)buffer[j+1],buffer[j+1]);
                

                // Find the cutting point
                int cutPoint = (numOfKeys+1)/2;

                numOfKeys = cutPoint-1;
                int newNumOfKeys = fileInfo.maxKeysPerBlock +1 - cutPoint;

                // printf("CUT= %d||%d\n\n",cutPoint,*(buffer+(cutPoint)*sizeOfPair));

                CALL_BF_BLOCK_INIT(newblock)
                int newBlockNum = Create_Index_Block(fileInfo.fileId);
                CALL_BF(BF_GetBlock(fileInfo.fileId, newBlockNum, newblock))
                char *newdata = BF_Block_GetData(newblock);

                memcpy(newdata+sizeof(char)+sizeof(int), buffer+cutPoint*sizeOfPair, newNumOfKeys*sizeOfPair+sizeof(int));

                //update key record number in both blocks
                memcpy(data+sizeof(char)+sizeof(int), buffer, numOfKeys*sizeOfPair+sizeof(int));

                memcpy(newdata+sizeof(char), &newNumOfKeys, sizeof(int));
                memcpy(data+sizeof(char), &numOfKeys, sizeof(int));

                memcpy(returnPair->key, buffer+cutPoint*sizeOfPair-keyLength, keyLength);
                returnPair->blockPointer = newBlockNum;
                printf("++++++++WE WILL RETURN: %s BUT WE RETURN: %s\n", (buffer+cutPoint*sizeOfPair-keyLength), returnPair->key);
                free(buffer);
                BF_Block_SetDirty(newblock);
                CALL_BF_BLOCK_DESTROY(newblock)
            printf("INCALL-RETURN_PAIR: block=%d &&& kes = %s\n", returnPair->blockPointer, (char*)returnPair->key);            returnPair->blockPointer = newBlockNum;

            } else {
                printf("TIME TO BREAK SOME ASS\n");
                memmove(nextNodeKeyPosition+keyLength+sizeof(int), nextNodeKeyPosition,
                        data+2*sizeof(int)+sizeof(char)+numOfKeys*(keyLength+sizeof(int)) - nextNodeKeyPosition);  //move data to write in that position
                memset(nextNodeKeyPosition, 0, keyLength+sizeof(int)); //flush the moved data space
                //write the new data
                int strlenAttr1      = fileInfo.attrLength1;
                if(fileInfo.attrType1 == STRING && strlen(newReturnPair.key) < strlenAttr1)
                    strlenAttr1 = strlen(newReturnPair.key)+1;
                    printf("NEW KEY: %s\n",newReturnPair.key);
                memcpy(nextNodeKeyPosition, newReturnPair.key, strlenAttr1);
                memcpy(nextNodeKeyPosition+keyLength, &(newReturnPair.blockPointer), sizeof(int));

                numOfKeys++;
                memcpy(data+sizeof(char),&numOfKeys,sizeof(int));
                printf("I BROKE SOME ASS\n");
            }
            free(newReturnPair.key);
            // BF_Block_SetDirty(block);
            // CALL_BF_BLOCK_DESTROY(block)
        }
    } else if (indicator == 'd') {
        key = malloc(keyLength);
        // CALL_BF_BLOCK_INIT(block)
        // CALL_BF(BF_GetBlock(fileInfo.fileId, treeNode, block))
        // char *data = BF_Block_GetData(block);
        // Block in data block
        int numOfRecords, newNumOfRecords;
        // Scan data to place with sorted order
        // Buffer to copy the records
        int nextBlock;
        memcpy(&nextBlock, data+sizeof(char)+sizeof(int), sizeof(int));
        memcpy(&numOfRecords, data+sizeof(char), sizeof(int));

        if ( numOfRecords >= fileInfo.maxRecordsPerBlock ) {
            //Allocate buffer
            char *buffer = malloc((fileInfo.maxRecordsPerBlock+1)*recordLength);
            // Write data to buffer
            buffer = writeToDBuffer(fileInfo, buffer, data, value1, value2);

            // Find the point to cut the block in half 2 3 3 3 3 
            int cutPoint = (numOfRecords+1)/2;
            prevkey = malloc(keyLength);
            memcpy(key, buffer+cutPoint*recordLength, keyLength);
            memcpy(prevkey, buffer+(cutPoint-1)*recordLength, keyLength);
            printf("KEY= %s || PREVKEY= %s +++++++++++++ THEY HAVE POSITIONS: %d\n", key, prevkey, (int)getDBlockData(fileInfo, buffer-(sizeof(char)+2*sizeof(int)), cutPoint) - (int)getDBlockData(fileInfo, buffer-(sizeof(char)+2*sizeof(int)), cutPoint-1));
            while ( compare(fileInfo, key, prevkey) == 0){
              cutPoint--;
              memset(key,0,keyLength);  // Flush the key memory
              memset(prevkey,0,keyLength);
              memcpy(key, getDBlockData(fileInfo, buffer-(sizeof(char)+2*sizeof(int)), cutPoint), keyLength);
              memcpy(prevkey, getDBlockData(fileInfo, buffer-(sizeof(char)+2*sizeof(int)), cutPoint-1), keyLength);
            }
            newNumOfRecords = fileInfo.maxRecordsPerBlock + 1 - cutPoint;
            printf("INFO HERE: <CUTPOINT: %d & NUMOFRECORDS: %d & MAXRECORDS: %d & NEWNUMOFRECORDS: %d>\n", cutPoint, numOfRecords, fileInfo.maxRecordsPerBlock, newNumOfRecords);
            numOfRecords = cutPoint;
            // Allocate block
            CALL_BF_BLOCK_INIT(newblock)
            int newBlockNum = Create_Data_Block(fileInfo.fileId, newNumOfRecords, nextBlock);
            CALL_BF(BF_GetBlock(fileInfo.fileId, newBlockNum, newblock))
            char *newdata = BF_Block_GetData(newblock);


            printf("NEW num of records: %d &&& buffer pointer : %d\n\n", newNumOfRecords, cutPoint*recordLength);
            memcpy(newdata+sizeof(char)+2*sizeof(int), buffer+cutPoint*recordLength, newNumOfRecords*recordLength);
            // Update the old block
            memcpy(data+sizeof(char)+2*sizeof(int),  buffer, numOfRecords*recordLength);

            memcpy(data+sizeof(char), &numOfRecords, sizeof(int));
            memcpy(data+sizeof(char)+sizeof(int), &newBlockNum, sizeof(int));

            free(buffer);
            free(prevkey);
            memcpy(returnPair->key, getDBlockData(fileInfo, newdata, 0), keyLength);
            printf("RETURN KEY: %s &&&  %d \n",returnPair->key, (int)getDBlockData(fileInfo, newdata, 0)-(int)newdata);
            returnPair->blockPointer = newBlockNum;
            BF_Block_SetDirty(newblock);
            CALL_BF_BLOCK_DESTROY(newblock)
        } else {
            int flag = 0;
            i = 0;
            // offset = sizeof(char)+2*sizeof(int);
            for(i=0; i<numOfRecords; i++) {
                memcpy(key, getDBlockData(fileInfo, data, i), keyLength);
                // If value on record less that value to insert memset the following records
                if (compare(fileInfo, key, value1) > 0) {
                    writeDBlockData(fileInfo, data, i, value1, value2);
                    flag = 1;
                    break;
                }
                // offset += recordLength;
            }
            if(flag == 0){
                writeDBlockData(fileInfo, data, i, value1, value2);
            }
        }
            // for (int j = 0; j < 500; j+=2){
            //     printf("kcolb-> %d data[%d]-> %c      kcolb-> %d data[%d]-> %c\n",100,j, data[j],100,j+1,data[j+1]);

            // }
        free(key);
//        BF_Block_SetDirty(block);
//        CALL_BF_BLOCK_DESTROY(block)
    } else {
        printf("gamh8hkameeee \n\n");
    }
    BF_Block_SetDirty(block);
    CALL_BF_BLOCK_DESTROY(block)
    return AME_OK;
}

char *scanForNextNode(FilesInfo fileInfo, char *data, void *value1, int *nextNode){
  // Block is index block
  // Get the block of the treeNode
  int numOfKeys;
  int flag = 0;
  int i = 0;
  int offset;
  int keyLength = fileInfo.attrLength1;
  void *key     = malloc(keyLength);
  void *prevkey = malloc(keyLength);

  memcpy(&numOfKeys, data+sizeof(char), sizeof(int));

  offset = sizeof(char)+2*sizeof(int);
  // Search the right position for the record is going to get inserted. And insert it
  for(i=0; i<numOfKeys; i++) {
      memcpy(key,  getIBlockData(fileInfo, data, i), keyLength);
      // Access all the keys until you find the correct position for the record or the end of the block
      if (compare(fileInfo, key, value1) > 0) {
          memcpy(nextNode, data+offset-sizeof(int), sizeof(int));
          // insertEntry(fileInfo, nextNode, value1, value2, returnPair);
          flag = 1;
          break;
      }
      offset += keyLength+sizeof(int);
  }
  if (flag == 0) {
    //   printf("trust me dady offset =%d\n", offset);
      memcpy(nextNode, data+offset-sizeof(int), sizeof(int)); // works with -keyLength
      // insertEntry(fileInfo, nextNode, value1, value2, returnPair);
  }
  free(key);
  free(prevkey);
//   printf("SCAN RETURN= value= %s, nextNode= %d\n",value1,*nextNode);
  return data+offset;
}