#ifndef AM_H_
#define AM_H_

/* Error codes */

extern int AM_errno;
extern int BF_errno;

#define EQUAL 1
#define NOT_EQUAL 2
#define LESS_THAN 3
#define GREATER_THAN 4
#define LESS_THAN_OR_EQUAL 5
#define GREATER_THAN_OR_EQUAL 6

// OUR CODE STARTS HERE

typedef struct InsertEntry_Return
{
    void *key;
    int blockPointer;
} InsertEntry_Return;

// typedef struct AM_Index_Information {
//     char indicator ='i';
//     int numOfKeys;
// } AM_Index_Information;
// #define AM_INDEX_BLOCK_INFORMATION_SIZE sizeof(AM_Index_Information)

// typedef struct AM_Data_Information {
//     char indicator = 'd';
//     int numOfRecords;
//     int nextBlock;
// } AM_Data_Information;
// #define AM_DATA_BLOCK_INFORMATION_SIZE sizeof(AM_Data_Information)

#define B_PLUS_FILE_INDICATOR_TYPE char *
#define B_PLUS_FILE_INDICATOR "B+"
#define B_PLUS_FILE_INDICATOR_SIZE sizeof(B_PLUS_FILE_INDICATOR)
#define B_PLUS_FILE_INDICATOR_COMPARE(indicator) \
    ( strcmp(B_PLUS_FILE_INDICATOR, indicator) == 0 )

#define AME_OK 0
#define AME_EOF -1                      /* There is not other value that satisfies the scan */
#define AME_OPEN_FILES_LIMIT_ERROR -2   /* There are already MAX_OPEN_FILES files opened */
#define AME_TYPE_ERROR -3               /* Not a B-Plus-File */
#define AME_INVALID_FILE_ERROR -4       /* File is not open */
#define AME_OPEN_SCANS_LIMIT_ERROR -5   /* There are already MAX_OPEN_SCANS scans opened */
#define AME_OPENED_SCANS -6             /* At least one scan is opened for a file */
#define AME_OPENED_FILE -7              /* File is opened */
#define AME_INVALID_SCAN_ERROR -8       /* Scan is not open */
#define AME_BF_ERROR -9                 /* Error in the bf part */
#define AME_UNABLE_TO_DELETE_FILE -10   /* Unable to delete file with remove() */

typedef struct FilesInfo
{
	char* fileName;
    char attrType1;
    char attrType2;
    int attrLength1;
    int attrLength2;
    int recordLength;
	int fileId;
    int openedScans;
    int root;
    int maxKeysPerBlock;
    int maxRecordsPerBlock;
} FilesInfo;

#define MAX_OPEN_FILES 20
typedef struct FilesMap
{
	FilesInfo filesInfo[MAX_OPEN_FILES];
	int filesCounter;

}FilesMap;

typedef struct ScansInfo
{
    int fileIndex;
    int blockId;
    int Operator;
    int foundCounter;
} ScansInfo;

#define MAX_OPEN_SCANS 20
typedef struct ScansMap
{
    ScansInfo scansInfo[MAX_OPEN_SCANS];
	int scansCounter;
}ScansMap;
// OUR CODE ENDS HERE

void AM_Init( void );


int AM_CreateIndex(
  char *fileName, /* όνομα αρχείου */
  char attrType1, /* τύπος πρώτου πεδίου: 'c' (συμβολοσειρά), 'i' (ακέραιος), 'f' (πραγματικός) */
  int attrLength1, /* μήκος πρώτου πεδίου: 4 γιά 'i' ή 'f', 1-255 γιά 'c' */
  char attrType2, /* τύπος πρώτου πεδίου: 'c' (συμβολοσειρά), 'i' (ακέραιος), 'f' (πραγματικός) */
  int attrLength2 /* μήκος δεύτερου πεδίου: 4 γιά 'i' ή 'f', 1-255 γιά 'c' */
);


int AM_DestroyIndex(
  char *fileName /* όνομα αρχείου */
);


int AM_OpenIndex (
  char *fileName /* όνομα αρχείου */
);


int AM_CloseIndex (
  int fileDesc /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
);


int AM_InsertEntry(
  int fileDesc, /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
  void *value1, /* τιμή του πεδίου-κλειδιού προς εισαγωγή */
  void *value2 /* τιμή του δεύτερου πεδίου της εγγραφής προς εισαγωγή */
);


int AM_OpenIndexScan(
  int fileDesc, /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
  int op, /* τελεστής σύγκρισης */
  void *value /* τιμή του πεδίου-κλειδιού προς σύγκριση */
);


void *AM_FindNextEntry(
  int scanDesc /* αριθμός που αντιστοιχεί στην ανοιχτή σάρωση */
);


int AM_CloseIndexScan(
  int scanDesc /* αριθμός που αντιστοιχεί στην ανοιχτή σάρωση */
);


void AM_PrintError(
  char *errString /* κείμενο για εκτύπωση */
);

void AM_Close();


#endif /* AM_H_ */
