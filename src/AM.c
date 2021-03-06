#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "AM.h"
#include "bf.h"
#include "OurFunctions.h"



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
    } else if (dataType == INTEGER){
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
  BF_Block_SetDirty(block);                                      
  CALL_BF_BLOCK_DESTROY(block)
  numOfBlock--;
  return  numOfBlock;                        
}

void AM_Init() {
// printf("+AM_Init: just got called.\n");
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
}


int AM_CreateIndex(char *fileName, char attrType1, int attrLength1, char attrType2, int attrLength2) {
// printf("+AM_CreateIndex: just got called.\n");
  AM_errno = AME_OK;
  // Check if the types are accepted
  if (typeCheck(attrType1) != 0) {
    AM_errno = AME_ATTRTYPE1_ERROR;
    return AME_ATTRTYPE1_ERROR;
  }
  if (typeCheck(attrType2) != 0) {
    AM_errno = AME_ATTRTYPE2_ERROR;
    return AME_ATTRTYPE2_ERROR;
  }
  // Check if the length of types are accepted
  if (lengthCheck(attrType1, attrLength1) != 0) {
    AM_errno = AME_ATTRLENGTH1_ERROR;
    return AME_ATTRLENGTH1_ERROR;
  }
  if (lengthCheck(attrType2, attrLength2) != 0) {
    AM_errno = AME_ATTRLENGTH2_ERROR;
    return AME_ATTRLENGTH2_ERROR;
  }
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
// printf("+AM_DestroyIndex: just got called.\n");
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
//   printf("+AM_OpenIndex: just got called.\n");
  AM_errno = AME_OK;
  // Check if there is free space in filesMap
  if (filesMap.filesCounter >= MAX_OPEN_FILES){
    AM_errno = AME_OPEN_FILES_LIMIT_ERROR;
    return AME_OPEN_FILES_LIMIT_ERROR;
  }
  // Check if there is the file
  if( access(fileName, F_OK) == -1 ) {
    AM_errno = AME_NOT_EXISTING_FILE;
    return AME_NOT_EXISTING_FILE;
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


  // Increase the filesCounter
  filesMap.filesCounter++;

  CALL_BF_BLOCK_DESTROY(block)
  return fileIndex;
}


int AM_CloseIndex (int fileDesc) {
// printf("+AM_CloseIndex: just got called.\n");
  AM_errno = AME_OK;
  // Check if the fileDesc is accepted
  if (fileDesc < 0 || fileDesc > MAX_OPEN_FILES) {
    AM_errno = AME_INVALID_FILEDESC_ERROR;
    return AME_INVALID_FILEDESC_ERROR;
  }
  // Check if the file is closed
  if(filesMap.filesInfo[fileDesc].fileId == -1) {
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
// printf("+AM_InsertEntry: just got called.\n");
  AM_errno = AME_OK;
  // Check if the fileDesc is accepted
  if (fileDesc < 0 || fileDesc > MAX_OPEN_FILES) {
    AM_errno = AME_INVALID_FILEDESC_ERROR;
    return AME_INVALID_FILEDESC_ERROR;
  }
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

    filesMap.filesInfo[fileDesc].firstBlock = leftBlockNum;

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

    insertEntry(filesMap.filesInfo[fileDesc], rootNum, value1, value2, &returnPair);

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
    }
    free(returnPair.key);
  //?...
  //? EDW ISWS PETHANOYME
  //?
  //! telika EPIZHSAMEE
  return AME_OK;
}


int AM_OpenIndexScan(int fileDesc, int op, void *value) {
// printf("+AM_OpenIndexScan: just got called.\n");
  AM_errno = AME_OK;
  // Check if the fileDesc is accepted
  if (fileDesc < 0 || fileDesc > MAX_OPEN_FILES) {
    AM_errno = AME_INVALID_FILEDESC_ERROR;
    return AME_INVALID_FILEDESC_ERROR;
  }
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
  scansMap.scansInfo[scanIndex].fileIndex     = fileDesc;
  scansMap.scansInfo[scanIndex].foundIndex    = 0;
  scansMap.scansInfo[scanIndex].recordIndex   = 0;
  scansMap.scansInfo[scanIndex].Operator      = op;
  // Increase opened scans counter of the file in filesMap
  filesMap.filesInfo[fileDesc].openedScans++;
  // Increase the scansCounter
  scansMap.scansCounter++;

  // find the right block that has value inside
  CALL_BF_BLOCK_INIT(block)
  int fileId    = filesMap.filesInfo[fileDesc].fileId;
  int currBlock = filesMap.filesInfo[fileDesc].root;
  CALL_BF(BF_GetBlock(fileId, currBlock, block))
  char *data = BF_Block_GetData(block);
  char indicator;
  int nextBlock;
  memcpy(&indicator, data, sizeof(char));

  //find the next block with scan for next node
  while(indicator != 'd'){
    scanForNextNode(filesMap.filesInfo[fileDesc], data, value, &nextBlock);
    CALL_BF(BF_UnpinBlock(block))
    CALL_BF(BF_GetBlock(fileId, nextBlock, block))
    data = BF_Block_GetData(block);
    memcpy(&indicator, data, sizeof(char));
  }

  //Scan inside the block
  int keyLength = filesMap.filesInfo[fileDesc].attrLength1;
  int firstBlock = filesMap.filesInfo[fileDesc].firstBlock;
  int numOfRecords;
  int i;
  int j;
  char *key = malloc(keyLength);
  memcpy(&numOfRecords, data+sizeof(char), sizeof(int));


  // set defferent pointer for each operator
  if( op == EQUAL || op == GREATER_THAN_OR_EQUAL || op == NOT_EQUAL || op == LESS_THAN_OR_EQUAL){
    //find the last first record in block
    for (i = 0; i < numOfRecords; i++){
      memcpy(key, getDBlockData(filesMap.filesInfo[fileDesc], data, i), keyLength);
      if (compare(filesMap.filesInfo[fileDesc], key, value) == 0)
        break;
    }
    scansMap.scansInfo[scanIndex].foundBlock    = nextBlock;
    scansMap.scansInfo[scanIndex].foundIndex    = i;
    
    // Find the last equal key in the same block
    for (j = i; j < numOfRecords; j++){
      memcpy(key, getDBlockData(filesMap.filesInfo[fileDesc], data, j), keyLength);
      if (compare(filesMap.filesInfo[fileDesc], key, value) > 0)
        break;
    }

    //for the start and ending point
    if(op == EQUAL){
      
      scansMap.scansInfo[scanIndex].recordBlock   = nextBlock;
      scansMap.scansInfo[scanIndex].recordIndex   = i;
      scansMap.scansInfo[scanIndex].endBlock      = nextBlock;
      scansMap.scansInfo[scanIndex].endIndex      = j;
    }
    else if (op == GREATER_THAN_OR_EQUAL){

      scansMap.scansInfo[scanIndex].recordBlock   = nextBlock;
      scansMap.scansInfo[scanIndex].recordIndex   = i;
      scansMap.scansInfo[scanIndex].endBlock      = nextBlock;
      scansMap.scansInfo[scanIndex].endIndex      = BF_BLOCK_SIZE;
    }
    else if (op = NOT_EQUAL){
      
      scansMap.scansInfo[scanIndex].recordBlock   = firstBlock;
      scansMap.scansInfo[scanIndex].recordIndex   = 0;
      scansMap.scansInfo[scanIndex].endBlock      = nextBlock;
      scansMap.scansInfo[scanIndex].endIndex      = j-1;
    }
    else if(op = LESS_THAN_OR_EQUAL){
      
      scansMap.scansInfo[scanIndex].recordBlock   = firstBlock;
      scansMap.scansInfo[scanIndex].recordIndex   = 0;
      scansMap.scansInfo[scanIndex].endBlock      = nextBlock;
      scansMap.scansInfo[scanIndex].endIndex      = j;
    }
    
    

  } else if ( op == GREATER_THAN ){
    for (i = 0; i < numOfRecords; i++){
      memcpy(key, getDBlockData(filesMap.filesInfo[fileDesc], data, i), keyLength);
      if (compare(filesMap.filesInfo[fileDesc], key, value) > 0)
        break;
    }
    // Greater wastn found in this block
    if(i == numOfRecords){
      //Get next block
      memcpy(&nextBlock, data+sizeof(char)+sizeof(int), sizeof(int));
      if(nextBlock==-1){
        AM_errno = AME_EOF; // If no other block is avaible it is end of file                    
      }
      scansMap.scansInfo[scanIndex].foundBlock    = nextBlock;
      scansMap.scansInfo[scanIndex].foundIndex    = 0;
      scansMap.scansInfo[scanIndex].recordBlock   = nextBlock; 
      scansMap.scansInfo[scanIndex].recordIndex   = 0;
    }
    else{
      scansMap.scansInfo[scanIndex].foundBlock    = nextBlock;
      scansMap.scansInfo[scanIndex].foundIndex    = i;
      scansMap.scansInfo[scanIndex].recordBlock   = nextBlock; 
      scansMap.scansInfo[scanIndex].recordIndex   = i;
    }
    scansMap.scansInfo[scanIndex].endBlock = nextBlock;
    scansMap.scansInfo[scanIndex].endIndex = BF_BLOCK_SIZE;
    
  } else if ( op == LESS_THAN ){

    for (i = 0; i < numOfRecords; i++){
      memcpy(key, getDBlockData(filesMap.filesInfo[fileDesc], data, i), keyLength);
      if (compare(filesMap.filesInfo[fileDesc], key, value) == 0)
        break;
    }

    scansMap.scansInfo[scanIndex].foundBlock    = nextBlock;
    scansMap.scansInfo[scanIndex].foundIndex    = i;
    scansMap.scansInfo[scanIndex].recordBlock   = firstBlock; 
    scansMap.scansInfo[scanIndex].recordIndex   = 0;
    scansMap.scansInfo[scanIndex].endBlock      = nextBlock;
    scansMap.scansInfo[scanIndex].endIndex      = i;
  }
  
  CALL_BF(BF_UnpinBlock(block))
  CALL_BF_BLOCK_DESTROY(block)
  free(key);

  if(AM_errno != AME_OK)
    return AME_EOF;
  return scanIndex;
}


void *AM_FindNextEntry(int scanDesc) {
// printf("+AM_FindNextEntry: just got called.\n");
  AM_errno = AME_OK;
  // Check if the fileDesc is accepted
  if (scanDesc < 0 || scanDesc > MAX_OPEN_SCANS) {
    AM_errno = AME_INVALID_SCANDESC_ERROR;
    return NULL;
  }

  int op          = scansMap.scansInfo[scanDesc].Operator;
  int foundBlock  = scansMap.scansInfo[scanDesc].foundBlock;
  int foundIndex  = scansMap.scansInfo[scanDesc].foundIndex;
  int recordBlock = scansMap.scansInfo[scanDesc].recordBlock;
  int recordIndex = scansMap.scansInfo[scanDesc].recordIndex;
  int endBlock    = scansMap.scansInfo[scanDesc].endBlock;
  int endIndex    = scansMap.scansInfo[scanDesc].endIndex;
  int fileDesc    = scansMap.scansInfo[scanDesc].fileIndex;
  int fileId      = filesMap.filesInfo[fileDesc].fileId;
  int keyLength   = filesMap.filesInfo[fileDesc].attrLength1; 
  int valLength   = filesMap.filesInfo[fileDesc].attrLength2;
  int numOfRecords;
  int nextBlock;
  char *data;

  CALL_BF_BLOCK_INIT(block)
  char *key = malloc(keyLength);

  if( op == EQUAL || op == GREATER_THAN || op == GREATER_THAN_OR_EQUAL || op == LESS_THAN_OR_EQUAL){

    if( endIndex == recordIndex && endBlock == recordBlock){
      AM_errno = AME_EOF;
      printf("EOF\n");
      return NULL;
    }

    CALL_BF(BF_GetBlock(fileId, recordBlock, block))
    data = BF_Block_GetData(block);
    memcpy(&numOfRecords, data+sizeof(char), sizeof(int));

    if(numOfRecords < recordIndex+1){
      memcpy(&nextBlock, data+sizeof(char)+sizeof(int), sizeof(int));
      if(nextBlock == -1){
        AM_errno = AME_EOF;
        free(key);
        CALL_BF_BLOCK_DESTROY(block)
        printf("EOF\n");
        return NULL;
      }

      CALL_BF(BF_UnpinBlock(block))
      CALL_BF(BF_GetBlock(fileId, nextBlock, block))
      data = BF_Block_GetData(block);
      memcpy(&numOfRecords, data+sizeof(char), sizeof(int));
      scansMap.scansInfo[scanDesc].recordBlock = nextBlock;
      scansMap.scansInfo[scanDesc].recordIndex = 0;
      recordIndex = 0;
      recordBlock = nextBlock;
    }

    memcpy(key, getDBlockData(filesMap.filesInfo[fileDesc], data, recordIndex), keyLength);

    scansMap.scansInfo[scanDesc].recordIndex++;
    free(key);
    CALL_BF_BLOCK_DESTROY(block)
    return getDBlockData(filesMap.filesInfo[fileDesc], data, recordIndex)+keyLength;
  }
  else if ( op == NOT_EQUAL ){
    CALL_BF(BF_GetBlock(fileId, recordBlock, block))
    data = BF_Block_GetData(block);
    memcpy(&numOfRecords, data+sizeof(char), sizeof(int));

    while(recordBlock == foundBlock && (recordIndex >= foundIndex && recordIndex <= endIndex)){
      recordIndex++;
      scansMap.scansInfo[scanDesc].recordIndex++;
    }

    if(numOfRecords < recordIndex+1){
      memcpy(&nextBlock, data+sizeof(char)+sizeof(int), sizeof(int));
      if(nextBlock == -1){
        AM_errno = AME_EOF;
        free(key);
        CALL_BF_BLOCK_DESTROY(block)
        return NULL;
      }

      CALL_BF(BF_UnpinBlock(block))
      CALL_BF(BF_GetBlock(fileId, nextBlock, block))
      data = BF_Block_GetData(block);
      memcpy(&numOfRecords, data+sizeof(char), sizeof(int));
      scansMap.scansInfo[scanDesc].recordBlock = nextBlock;
      scansMap.scansInfo[scanDesc].recordIndex = 0;
      recordIndex = 0;
      recordBlock = nextBlock;
    }

    while(recordBlock == foundBlock && (recordIndex >= foundIndex && recordIndex <= endIndex)){
      recordIndex++;
      scansMap.scansInfo[scanDesc].recordIndex++;
      if(numOfRecords < recordIndex+1){
        AM_errno = AME_EOF;
        free(key);
        CALL_BF_BLOCK_DESTROY(block)
        printf("EOF\n");
        return NULL;
      }
    }

    memcpy(key, getDBlockData(filesMap.filesInfo[fileDesc], data, recordIndex), keyLength);

    scansMap.scansInfo[scanDesc].recordIndex++;
    free(key);
    CALL_BF_BLOCK_DESTROY(block)
    return getDBlockData(filesMap.filesInfo[fileDesc], data, recordIndex)+keyLength;
  }
  else if (op == LESS_THAN){

    if( endIndex == recordIndex && endBlock == recordBlock){
      AM_errno = AME_EOF;
      printf("EOF\n");
      return NULL;
    }

    CALL_BF(BF_GetBlock(fileId, recordBlock, block))
    data = BF_Block_GetData(block);
    memcpy(&numOfRecords, data+sizeof(char), sizeof(int));

    if(numOfRecords < recordIndex+1){
      memcpy(&nextBlock, data+sizeof(char)+sizeof(int), sizeof(int));
      if(nextBlock == -1){
        AM_errno = AME_EOF;
        free(key);
        CALL_BF_BLOCK_DESTROY(block)
        printf("EOF\n");
        return NULL;
      }

      CALL_BF(BF_UnpinBlock(block))
      CALL_BF(BF_GetBlock(fileId, nextBlock, block))
      data = BF_Block_GetData(block);
      memcpy(&numOfRecords, data+sizeof(char), sizeof(int));
      scansMap.scansInfo[scanDesc].recordBlock = nextBlock;
      scansMap.scansInfo[scanDesc].recordIndex = 0;
      recordIndex = 0;
      recordBlock = nextBlock;
    }
    if( endIndex == recordIndex && endBlock == recordBlock){
      AM_errno = AME_EOF;
      printf("EOF\n");
      return NULL;
    }

    memcpy(key, getDBlockData(filesMap.filesInfo[fileDesc], data, recordIndex), keyLength);

    scansMap.scansInfo[scanDesc].recordIndex++;
    free(key);
    CALL_BF_BLOCK_DESTROY(block)
    return getDBlockData(filesMap.filesInfo[fileDesc], data, recordIndex)+keyLength;
  }

}


int AM_CloseIndexScan(int scanDesc) {
// printf("+AM_CloseIndexScan: just got called.\n");
  AM_errno = AME_OK;
  // Check if the fileDesc is accepted
  if (scanDesc < 0 || scanDesc > MAX_OPEN_SCANS) {
    AM_errno = AME_INVALID_SCANDESC_ERROR;
    return AME_INVALID_SCANDESC_ERROR;
  }
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
  if (AM_errno == AME_OK)
    return;
  if (errString != NULL)
    printf("%s\t: ", errString);
  switch(AM_errno) {  
      case AME_EOF:
        printf("AME_EOF.\n");
        break;
      case AME_OPEN_FILES_LIMIT_ERROR:
        printf("AME_OPEN_FILES_LIMIT_ERROR.\n");
            printf("Error: Has reached the maximum number of files that can be opened\n");
        break;
      case AME_TYPE_ERROR:
        printf("AME_TYPE_ERROR.\n");
            printf("Error: Not a B-Plus-file\n");
        break;
      case AME_INVALID_FILE_ERROR:
        printf("AME_INVALID_FILE_ERROR.\n");
        break;
      case AME_OPEN_SCANS_LIMIT_ERROR:
        printf("AME_OPEN_SCANS_LIMIT_ERROR.\n");
            printf("Error: Has reached the maximum number of scans that can be opened\n");
        break;
      case AME_OPENED_SCANS:
        printf("AME_OPENED_SCANS.\n");
        break;
      case AME_OPENED_FILE:
        printf("AME_OPENED_FILE.\n");
            printf("Error: file is opened in the filesMap\n");
        break;
      case AME_INVALID_SCAN_ERROR:
        printf("AME_INVALID_SCAN_ERROR.\n");
        break;
      case AME_BF_ERROR:
        BF_PrintError(BF_errno);
        break;
      case AME_UNABLE_TO_DELETE_FILE:
        printf("AME_UNABLE_TO_DELETE_FILE: Unable to destroy the file.\n");
        break;
      case AME_ATTRLENGTH1_ERROR:
        printf("AME_ATTRLENGTH1_ERROR.\n");
        printf("Error: wrong length of attribute 1");
        break;
      case AME_ATTRLENGTH2_ERROR:
        printf("AME_ATTRLENGTH2_ERROR.\n");
        printf("Error: wrong length of attribute 2");
        break;
      case AME_ATTRTYPE1_ERROR:
        printf("AME_ATTRTYPE1_ERROR.\n");
        break;
      case AME_ATTRTYPE2_ERROR:
        printf("AME_ATTRTYPE2_ERROR.\n");
        break;
      case UNDEFINED_BLOCK_TYPE:
        printf("UNDEFINED_BLOCK_TYPE.\n");
        break;
      case AME_NOT_EXISTING_FILE:
        printf("AME_NOT_EXISTING_FILE.\n");
        break;
      case AME_INVALID_FILEDESC_ERROR:
        printf("AME_INVALID_FILEDESC_ERROR.\n");
        break;
      case AME_INVALID_SCANDESC_ERROR:
        printf("AME_INVALID_SCANDESC_ERROR.\n");
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


int insertEntry(FilesInfo fileInfo, int treeNode,void *value1,void *value2, InsertEntry_Return *returnPair) {
    // Recursive function to insert entry in the treeNode
    // Get the block of the treeNode
    CALL_BF_BLOCK_INIT(block)
    int test;
    BF_GetBlockCounter(fileInfo.fileId,&test);
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
    if (indicator == INDEX_BLOCK_TYPE) {
        // Block is index block
        int numOfKeys;
        memcpy(&numOfKeys, data+sizeof(char), sizeof(int));
        int nextNode;
        char *nextNodeKeyPosition = scanForNextNode(fileInfo, data, value1, &nextNode);
        int sizeOfPair = keyLength+sizeof(int);

        InsertEntry_Return newReturnPair;
        newReturnPair.blockPointer = treeNode;
        newReturnPair.key = malloc(keyLength);
        
        insertEntry(fileInfo, nextNode, value1, value2, &newReturnPair);

        if (newReturnPair.blockPointer != treeNode) {

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

                // Find the cutting point
                int cutPoint = (numOfKeys+1)/2;

                numOfKeys = cutPoint-1;
                int newNumOfKeys = fileInfo.maxKeysPerBlock +1 - cutPoint;

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
                free(buffer);
                BF_Block_SetDirty(newblock);
                CALL_BF_BLOCK_DESTROY(newblock)

            } else {
                memmove(nextNodeKeyPosition+keyLength+sizeof(int), nextNodeKeyPosition,
                        data+2*sizeof(int)+sizeof(char)+numOfKeys*(keyLength+sizeof(int)) - nextNodeKeyPosition);  //move data to write in that position
                memset(nextNodeKeyPosition, 0, keyLength+sizeof(int)); //flush the moved data space
                //write the new data
                int strlenAttr1      = fileInfo.attrLength1;
                if(fileInfo.attrType1 == STRING && strlen(newReturnPair.key) < strlenAttr1)
                    strlenAttr1 = strlen(newReturnPair.key)+1;
                memcpy(nextNodeKeyPosition, newReturnPair.key, strlenAttr1);
                memcpy(nextNodeKeyPosition+keyLength, &(newReturnPair.blockPointer), sizeof(int));

                numOfKeys++;
                memcpy(data+sizeof(char),&numOfKeys,sizeof(int));
            }
            free(newReturnPair.key);

        }
    } else if (indicator == DATA_BLOCK_TYPE) {
        key = malloc(keyLength);
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
            while ( compare(fileInfo, key, prevkey) == 0){
              cutPoint--;
              memset(key,0,keyLength);  // Flush the key memory
              memset(prevkey,0,keyLength);
              memcpy(key, getDBlockData(fileInfo, buffer-(sizeof(char)+2*sizeof(int)), cutPoint), keyLength);
              memcpy(prevkey, getDBlockData(fileInfo, buffer-(sizeof(char)+2*sizeof(int)), cutPoint-1), keyLength);
            }
            newNumOfRecords = fileInfo.maxRecordsPerBlock + 1 - cutPoint;
            numOfRecords = cutPoint;
            // Allocate block
            CALL_BF_BLOCK_INIT(newblock)
            int newBlockNum = Create_Data_Block(fileInfo.fileId, newNumOfRecords, nextBlock);
            CALL_BF(BF_GetBlock(fileInfo.fileId, newBlockNum, newblock))
            char *newdata = BF_Block_GetData(newblock);


            memcpy(newdata+sizeof(char)+2*sizeof(int), buffer+cutPoint*recordLength, newNumOfRecords*recordLength);
            // Update the old block
            memcpy(data+sizeof(char)+2*sizeof(int),  buffer, numOfRecords*recordLength);

            memcpy(data+sizeof(char), &numOfRecords, sizeof(int));
            memcpy(data+sizeof(char)+sizeof(int), &newBlockNum, sizeof(int));

            free(buffer);
            free(prevkey);
            memcpy(returnPair->key, getDBlockData(fileInfo, newdata, 0), keyLength);
            returnPair->blockPointer = newBlockNum;
            BF_Block_SetDirty(newblock);
            CALL_BF_BLOCK_DESTROY(newblock)
        } else {
            int flag = 0;
            i = 0;
            for(i=0; i<numOfRecords; i++) {
                memcpy(key, getDBlockData(fileInfo, data, i), keyLength);
                // If value on record less that value to insert memset the following records
                if (compare(fileInfo, key, value1) > 0) {
                    writeDBlockData(fileInfo, data, i, value1, value2);
                    flag = 1;
                    break;
                }
            }
            if(flag == 0){
                writeDBlockData(fileInfo, data, i, value1, value2);
            }
        }
        free(key);

    } else {
        return UNDEFINED_BLOCK_TYPE;
    }
    BF_Block_SetDirty(block);
    CALL_BF_BLOCK_DESTROY(block)
    return AME_OK;
}

