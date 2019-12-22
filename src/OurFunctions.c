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