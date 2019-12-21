#ifndef _OUR_F_
#define _OUR_F_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AM.h"
#include "bf.h"
#include "defn.h"

void *getDBlockData(FilesInfo fileInfo, char * data, int i);
void *getIBlockData(FilesInfo fileInfo, char * data, int i);
void writeDBlockData(FilesInfo fileInfo, char *data, int i, void *value1, void *value2);
void writeIBlockData(FilesInfo fileInfo, char *data, int i, void *value1, void *value2);

#endif