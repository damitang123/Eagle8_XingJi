#ifndef _MD5_H_
#define _MD5_H_

/* Data structure for MD5 (Message Digest) computation */
typedef struct
{
	unsigned long			i[2];                   /* Number of _bits_ handled mod 2^64 */
	unsigned long			buf[4];                                    /* Scratch buffer */
	unsigned char			in[64];                              /* Input buffer */
	unsigned char			digest[16];     /* Actual digest after MD5Final call */
} MD5_CTX;

void MD5_Transform (unsigned long *buf, unsigned long *in);
void MD5Init(MD5_CTX *mdContext, unsigned long pseudoRandomNumber);
void MD5Update(MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
void MD5Final(MD5_CTX *mdContext);


char *GetSafePassword(char *strDeviceID, unsigned long dwSeed, unsigned char *aSystemTime);

#endif

