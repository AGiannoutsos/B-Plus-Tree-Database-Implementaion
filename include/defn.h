#ifndef DEFN_H_
#define DEFN_H_

#define INTEGER 'i'
#define FLOAT 'f'
#define STRING 'c'

#define INDEX_BLOCK_TYPE 'i'
#define DATA_BLOCK_TYPE 'd'

int typeCheck(char type);
int lengthCheck(char type, int length);

#endif /* DEFN_H_ */
