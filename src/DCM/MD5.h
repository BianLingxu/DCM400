#include <IOSTREAM>
typedef struct {
    unsigned int state[4];     
    unsigned int count[2];     
    unsigned char buffer[64];     
} MD5Context;

void MD5_Init(MD5Context * context);
void MD5_Update(MD5Context * context, const unsigned char * buf, int len);
void MD5_Final(MD5Context * context, unsigned char digest[16]);
void MD5_File (char * filename,unsigned char digest[16]);