
#include <defn.h>

int typeCheck(char type) {
    if (type != INTEGER && type != FLOAT && type != STRING)
        return -1;
    return 0;
};

int lengthCheck(char type, int length) {
    switch(type){
        case INTEGER:
            if (length != 4)
                return -1;
            break;
        case FLOAT:
        if (length != 4)
                return -1;
            break;
        case STRING:
            if (length > 255 || length < 1)
                return -1;
            break;
        default:
            break;
    }
    return 0;
};