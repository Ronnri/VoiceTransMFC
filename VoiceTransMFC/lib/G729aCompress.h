#ifndef _CG729aCompress_H_
#define _CG729aCompress_H_

#include "stdafx.h"
#pragma comment(lib,"legacy_stdio_definitions.lib")


extern "C" void va_g729a_init_encoder();
extern "C" void va_g729a_encoder(short *speech, unsigned char *bitstream);
extern "C" void va_g729a_init_decoder();
extern "C" void va_g729a_decoder(unsigned char *bitstream, short *synth_short, int bfi);
#pragma comment(lib,"G729a.lib")

#define  L_FRAME_COMPRESSED 10
#define  L_FRAME            80

class CG729aCompress 
{
public:
	CG729aCompress();
	virtual  ~CG729aCompress();

	virtual BOOL Compress(char* src,int srclen,char* dst,int* dstlen);
	virtual BOOL UnCompress(char* src,int srclen,char* dst,int* dstlen);

	
};


#endif