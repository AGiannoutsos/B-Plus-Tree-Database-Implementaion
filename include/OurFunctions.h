#ifndef _OUR_F_
#define _OUR_F_

void *getDBlockData(FilesInfo fileInfo, char * data, int i);
void *getIBlockData(FilesInfo fileInfo, char * data, int i);
void writeDBlockData(FilesInfo fileInfo, char *data, int i, void *value1, void *value2);
void writeIBlockData(FilesInfo fileInfo, char *data, int i, void *value1, void *value2);

char *scanForNextNode(FilesInfo fileInfo, char *data, void *value1, int *nextNode);
char *writeToIBuffer(FilesInfo fileInfo,  char *buffer, char *data, void *value, int blockNum);
char *writeToDBuffer(FilesInfo fileInfo,  char *buffer, char *data, void *value1, void *value2);

int insertEntry(FilesInfo fileInfo, int treeNode,void *value1,void *value2, InsertEntry_Return *returnPair);

#endif