#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include "MD5.h"

/* Padding */
static const unsigned char MD5_PADDING[64] =
{
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* MD5_F, MD5_G and MD5_H are basic MD5 functions: selection, majority, parity */
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#ifndef ROTATE_LEFT
	#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#endif

/* MD5_FF, MD5_GG, MD5_HH, and MD5_II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define MD5_FF(a, b, c, d, x, s, ac) {(a) += MD5_F ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }
#define MD5_GG(a, b, c, d, x, s, ac) {(a) += MD5_G ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }
#define MD5_HH(a, b, c, d, x, s, ac) {(a) += MD5_H ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }
#define MD5_II(a, b, c, d, x, s, ac) {(a) += MD5_I ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b); }

/* Constants for transformation */
#define MD5_S11 7  /* Round 1 */
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21 5  /* Round 2 */
#define MD5_S22 9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31 4  /* Round 3 */
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41 6  /* Round 4 */
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21

/* Basic MD5 step. MD5_Transform buf based on in */
void MD5_Transform (unsigned long *buf, unsigned long *in)
{
	unsigned long a = buf[0], b = buf[1], c = buf[2], d = buf[3];

	/* Round 1 */
	MD5_FF ( a, b, c, d, in[ 0], MD5_S11, (unsigned long) 3614090360u); /* 1 */
	MD5_FF ( d, a, b, c, in[ 1], MD5_S12, (unsigned long) 3905402710u); /* 2 */
	MD5_FF ( c, d, a, b, in[ 2], MD5_S13, (unsigned long)  606105819u); /* 3 */
	MD5_FF ( b, c, d, a, in[ 3], MD5_S14, (unsigned long) 3250441966u); /* 4 */
	MD5_FF ( a, b, c, d, in[ 4], MD5_S11, (unsigned long) 4118548399u); /* 5 */
	MD5_FF ( d, a, b, c, in[ 5], MD5_S12, (unsigned long) 1200080426u); /* 6 */
	MD5_FF ( c, d, a, b, in[ 6], MD5_S13, (unsigned long) 2821735955u); /* 7 */
	MD5_FF ( b, c, d, a, in[ 7], MD5_S14, (unsigned long) 4249261313u); /* 8 */
	MD5_FF ( a, b, c, d, in[ 8], MD5_S11, (unsigned long) 1770035416u); /* 9 */
	MD5_FF ( d, a, b, c, in[ 9], MD5_S12, (unsigned long) 2336552879u); /* 10 */
	MD5_FF ( c, d, a, b, in[10], MD5_S13, (unsigned long) 4294925233u); /* 11 */
	MD5_FF ( b, c, d, a, in[11], MD5_S14, (unsigned long) 2304563134u); /* 12 */
	MD5_FF ( a, b, c, d, in[12], MD5_S11, (unsigned long) 1804603682u); /* 13 */
	MD5_FF ( d, a, b, c, in[13], MD5_S12, (unsigned long) 4254626195u); /* 14 */
	MD5_FF ( c, d, a, b, in[14], MD5_S13, (unsigned long) 2792965006u); /* 15 */
	MD5_FF ( b, c, d, a, in[15], MD5_S14, (unsigned long) 1236535329u); /* 16 */

	/* Round 2 */
	MD5_GG ( a, b, c, d, in[ 1], MD5_S21, (unsigned long) 4129170786u); /* 17 */
	MD5_GG ( d, a, b, c, in[ 6], MD5_S22, (unsigned long) 3225465664u); /* 18 */
	MD5_GG ( c, d, a, b, in[11], MD5_S23, (unsigned long)  643717713u); /* 19 */
	MD5_GG ( b, c, d, a, in[ 0], MD5_S24, (unsigned long) 3921069994u); /* 20 */
	MD5_GG ( a, b, c, d, in[ 5], MD5_S21, (unsigned long) 3593408605u); /* 21 */
	MD5_GG ( d, a, b, c, in[10], MD5_S22, (unsigned long)   38016083u); /* 22 */
	MD5_GG ( c, d, a, b, in[15], MD5_S23, (unsigned long) 3634488961u); /* 23 */
	MD5_GG ( b, c, d, a, in[ 4], MD5_S24, (unsigned long) 3889429448u); /* 24 */
	MD5_GG ( a, b, c, d, in[ 9], MD5_S21, (unsigned long)  568446438u); /* 25 */
	MD5_GG ( d, a, b, c, in[14], MD5_S22, (unsigned long) 3275163606u); /* 26 */
	MD5_GG ( c, d, a, b, in[ 3], MD5_S23, (unsigned long) 4107603335u); /* 27 */
	MD5_GG ( b, c, d, a, in[ 8], MD5_S24, (unsigned long) 1163531501u); /* 28 */
	MD5_GG ( a, b, c, d, in[13], MD5_S21, (unsigned long) 2850285829u); /* 29 */
	MD5_GG ( d, a, b, c, in[ 2], MD5_S22, (unsigned long) 4243563512u); /* 30 */
	MD5_GG ( c, d, a, b, in[ 7], MD5_S23, (unsigned long) 1735328473u); /* 31 */
	MD5_GG ( b, c, d, a, in[12], MD5_S24, (unsigned long) 2368359562u); /* 32 */

	/* Round 3 */
	MD5_HH ( a, b, c, d, in[ 5], MD5_S31, (unsigned long) 4294588738u); /* 33 */
	MD5_HH ( d, a, b, c, in[ 8], MD5_S32, (unsigned long) 2272392833u); /* 34 */
	MD5_HH ( c, d, a, b, in[11], MD5_S33, (unsigned long) 1839030562u); /* 35 */
	MD5_HH ( b, c, d, a, in[14], MD5_S34, (unsigned long) 4259657740u); /* 36 */
	MD5_HH ( a, b, c, d, in[ 1], MD5_S31, (unsigned long) 2763975236u); /* 37 */
	MD5_HH ( d, a, b, c, in[ 4], MD5_S32, (unsigned long) 1272893353u); /* 38 */
	MD5_HH ( c, d, a, b, in[ 7], MD5_S33, (unsigned long) 4139469664u); /* 39 */
	MD5_HH ( b, c, d, a, in[10], MD5_S34, (unsigned long) 3200236656u); /* 40 */
	MD5_HH ( a, b, c, d, in[13], MD5_S31, (unsigned long)  681279174u); /* 41 */
	MD5_HH ( d, a, b, c, in[ 0], MD5_S32, (unsigned long) 3936430074u); /* 42 */
	MD5_HH ( c, d, a, b, in[ 3], MD5_S33, (unsigned long) 3572445317u); /* 43 */
	MD5_HH ( b, c, d, a, in[ 6], MD5_S34, (unsigned long)   76029189u); /* 44 */
	MD5_HH ( a, b, c, d, in[ 9], MD5_S31, (unsigned long) 3654602809u); /* 45 */
	MD5_HH ( d, a, b, c, in[12], MD5_S32, (unsigned long) 3873151461u); /* 46 */
	MD5_HH ( c, d, a, b, in[15], MD5_S33, (unsigned long)  530742520u); /* 47 */
	MD5_HH ( b, c, d, a, in[ 2], MD5_S34, (unsigned long) 3299628645u); /* 48 */

	/* Round 4 */
	MD5_II ( a, b, c, d, in[ 0], MD5_S41, (unsigned long) 4096336452u); /* 49 */
	MD5_II ( d, a, b, c, in[ 7], MD5_S42, (unsigned long) 1126891415u); /* 50 */
	MD5_II ( c, d, a, b, in[14], MD5_S43, (unsigned long) 2878612391u); /* 51 */
	MD5_II ( b, c, d, a, in[ 5], MD5_S44, (unsigned long) 4237533241u); /* 52 */
	MD5_II ( a, b, c, d, in[12], MD5_S41, (unsigned long) 1700485571u); /* 53 */
	MD5_II ( d, a, b, c, in[ 3], MD5_S42, (unsigned long) 2399980690u); /* 54 */
	MD5_II ( c, d, a, b, in[10], MD5_S43, (unsigned long) 4293915773u); /* 55 */
	MD5_II ( b, c, d, a, in[ 1], MD5_S44, (unsigned long) 2240044497u); /* 56 */
	MD5_II ( a, b, c, d, in[ 8], MD5_S41, (unsigned long) 1873313359u); /* 57 */
	MD5_II ( d, a, b, c, in[15], MD5_S42, (unsigned long) 4264355552u); /* 58 */
	MD5_II ( c, d, a, b, in[ 6], MD5_S43, (unsigned long) 2734768916u); /* 59 */
	MD5_II ( b, c, d, a, in[13], MD5_S44, (unsigned long) 1309151649u); /* 60 */
	MD5_II ( a, b, c, d, in[ 4], MD5_S41, (unsigned long) 4149444226u); /* 61 */
	MD5_II ( d, a, b, c, in[11], MD5_S42, (unsigned long) 3174756917u); /* 62 */
	MD5_II ( c, d, a, b, in[ 2], MD5_S43, (unsigned long)  718787259u); /* 63 */
	MD5_II ( b, c, d, a, in[ 9], MD5_S44, (unsigned long) 3951481745u); /* 64 */

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

// Set pseudoRandomNumber to zero for RFC MD5 implementation
void MD5Init (MD5_CTX *mdContext, unsigned long pseudoRandomNumber)
{
	mdContext->i[0] = mdContext->i[1] = (unsigned long)0;

	/* Load magic initialization constants */
	mdContext->buf[0] = (unsigned long)0x67452301 + (pseudoRandomNumber * 11);
	mdContext->buf[1] = (unsigned long)0xefcdab89 + (pseudoRandomNumber * 71);
	mdContext->buf[2] = (unsigned long)0x98badcfe + (pseudoRandomNumber * 37);
	mdContext->buf[3] = (unsigned long)0x10325476 + (pseudoRandomNumber * 97);
}

void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen)
{
	unsigned long in[16];
	int mdi = 0;
	unsigned int i = 0, ii = 0;

	/* Compute number of bytes mod 64 */
	mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((mdContext->i[0] + ((unsigned long)inLen << 3)) < mdContext->i[0])
	{
		mdContext->i[1]++;
	}
	mdContext->i[0] += ((unsigned long)inLen << 3);
	mdContext->i[1] += ((unsigned long)inLen >> 29);

	while (inLen--)
	{
		/* Add new character to buffer, increment mdi */
		mdContext->in[mdi++] = *inBuf++;

		/* Transform if necessary */
		if (mdi == 0x40)
		{
			for (i = 0, ii = 0; i < 16; i++, ii += 4)
				in[i] = (((unsigned long)mdContext->in[ii + 3]) << 24) |
						(((unsigned long)mdContext->in[ii + 2]) << 16) |
						(((unsigned long)mdContext->in[ii + 1]) << 8) |
						((unsigned long)mdContext->in[ii]);

			MD5_Transform (mdContext->buf, in);
			mdi = 0;
		}
	}
}

void MD5Final (MD5_CTX *mdContext)
{
	unsigned long in[16];
	int mdi = 0;
	unsigned int i = 0, ii = 0, padLen = 0;

	/* Save number of bits */
	in[14] = mdContext->i[0];
	in[15] = mdContext->i[1];

	/* Compute number of bytes mod 64 */
	mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

	/* Pad out to 56 mod 64 */
	padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
	MD5Update (mdContext, (unsigned char *)MD5_PADDING, padLen);

	/* Append length in bits and transform */
	for (i = 0, ii = 0; i < 14; i++, ii += 4)
		in[i] = (((unsigned long)mdContext->in[ii + 3]) << 24) |
				(((unsigned long)mdContext->in[ii + 2]) << 16) |
				(((unsigned long)mdContext->in[ii + 1]) <<  8) |
				((unsigned long)mdContext->in[ii]);
	MD5_Transform (mdContext->buf, in);

	/* Store buffer in digest */
	for (i = 0, ii = 0; i < 4; i++, ii += 4)
	{
		mdContext->digest[ii]   = (unsigned char)( mdContext->buf[i]        & 0xFF);
		mdContext->digest[ii + 1] = (unsigned char)((mdContext->buf[i] >>  8) & 0xFF);
		mdContext->digest[ii + 2] = (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
		mdContext->digest[ii + 3] = (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
	}
}



char *GetSafePassword(char *strDeviceID, unsigned long dwSeed, unsigned char *aSystemTime)
{
	MD5_CTX md5;
	char  szBuffer[25];
	static char  strPWD[15];
	sprintf(szBuffer, "%02d%d%02d-%s-%d",
			aSystemTime[1], aSystemTime[0] + 2000, aSystemTime[2], strDeviceID, aSystemTime[3]);

	MD5Init(&md5, dwSeed);
	MD5Update(&md5, (unsigned char *)szBuffer, strlen(szBuffer));
	MD5Final(&md5);

	memset(strPWD, 0, sizeof(strPWD));
	sprintf(szBuffer, "%d", (md5.digest[2] + md5.digest[1] + md5.digest[0]) / 3);
	strcat(strPWD, szBuffer);
	sprintf(szBuffer, "%d", (md5.digest[7] + md5.digest[8] + md5.digest[9]) / 2);
	strcat(strPWD, szBuffer);
	sprintf(szBuffer, "%d", (md5.digest[12] + md5.digest[13] + md5.digest[14] + md5.digest[15]) / 5);
	strcat(strPWD, szBuffer);

	return strPWD;
}
