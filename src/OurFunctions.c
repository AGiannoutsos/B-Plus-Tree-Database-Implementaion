#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AM.h"
#include "bf.h"
#include "OurFunctions.h"

void *getDBlockData(FilesInfo fileInfo, char * data, int i){
  int keyLength     = fileInfo.attrLength1;
  int recordLength  = fileInfo.recordLength;
  int offset        = sizeof(char)+2*sizeof(int);
  int attrType1     = fileInfo.attrType1;
  // void *key;
  offset += i*recordLength;
  //memcpy(key, data+offset, keyLength);

  if (attrType1 == 'c'){
        return (void*)data+offset;
    } else if (attrType1 == 'f') {
        return (void*)data+offset;
    } else{
        return (void*)data+offset;
    }
}

void *getIBlockData(FilesInfo fileInfo, char * data, int i){
  int keyLength     = fileInfo.attrLength1;
  int recordLength  = fileInfo.recordLength;
  int offset        = sizeof(char)+2*sizeof(int);
  int attrType1     = fileInfo.attrType1;

  offset += i*(keyLength+sizeof(int));

  if (attrType1 == 'c'){
        return (void*)data+offset;
    } else if (attrType1 == 'f') {
        return (void*)data+offset;
    } else{
        return (void*)data+offset;
    }
}

void writeDBlockData(FilesInfo fileInfo, char *data, int i, void *value1, void *value2){

  int va1Length     = fileInfo.attrLength1;
  int va2Length     = fileInfo.attrLength2;
  int recordLength  = fileInfo.recordLength;
  int offset        = sizeof(char)+2*sizeof(int);
  int numOfRecords;
  int strlenAttr1   = va1Length;
  int strlenAttr2   = va2Length;
  offset += i*recordLength;
  memcpy(&numOfRecords, data+sizeof(char), sizeof(int));
  if(fileInfo.attrType1 == STRING)
    strlenAttr1 = strlen(value1)+1;
  if(fileInfo.attrType2 == STRING)
    strlenAttr2 = strlen(value2)+1;
  
  memmove(data+offset+recordLength, data+offset, (numOfRecords-i)*recordLength);  //move data to write in that position
  memset(data+offset, 0, recordLength); //flush the moved data space
  //write the new data
  memcpy(data+offset, value1, strlenAttr1);
  memcpy(data+offset+va1Length, value2, strlenAttr2);

  numOfRecords++;     //update the counter
  memcpy(data+sizeof(char), &numOfRecords, sizeof(int));
}

void writeIBlockData(FilesInfo fileInfo, char *data, int i, void *value1, void *value2){

  int va1Length     = fileInfo.attrLength1;
  int va2Length     = fileInfo.attrLength2;
  int recordLength  = fileInfo.recordLength;
  int offset        = sizeof(char)+2*sizeof(int);
  int numOfRecords;
  int strlenAttr1   = va1Length;
  int strlenAttr2   = va2Length;
  offset += i*recordLength;
  memcpy(&numOfRecords, data+sizeof(char), sizeof(int));
  if(fileInfo.attrType1 == STRING)
    strlenAttr1 = strlen(value1)+1;
  if(fileInfo.attrType2 == STRING)
    strlenAttr2 = strlen(value2)+1;
  
  memmove(data+offset+recordLength, data+offset, (numOfRecords-i)*recordLength);  //move data to write in that position
  memset(data+offset, 0, recordLength); //flush the moved data space
  //write the new data
  memcpy(data+offset, value1, strlenAttr1);
  memcpy(data+offset+va1Length, value2, strlenAttr2);

  numOfRecords++;     //update the counter
  memcpy(data+sizeof(char), &numOfRecords, sizeof(int));
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
      memcpy(nextNode, data+offset-sizeof(int), sizeof(int)); // works with -keyLength
  }
  free(key);
  free(prevkey);
  return data+offset;
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
      memcpy(buffer+offset+keyLength, &blockNum, sizeof(int));
  }
  free(key);
  return buffer;
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
      memcpy(buffer+offset+keyLength, value2, strlenAttr2);
  }
  free(key);
  return buffer;
}

