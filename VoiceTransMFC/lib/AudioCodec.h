#ifndef	_AUDIOCODEC_H_
#define	_AUDIOCODEC_H_

#include<afxmt.h>
#include <mmsystem.h>
#include <mmreg.h>

#define HZ_POOR		8000
#define HZ_LOW		11025
#define HZ_NORMAL	22050
#define HZ_HIGH		44100

#define SAMPLEPSEC   HZ_LOW

#define SIZE_AUDIO_FRAME	8192
#define SIZE_AUDIO_PACKED	512

class CAudioCodec
{
public:
	CAudioCodec() {}
	virtual ~CAudioCodec() {}

public:
	virtual BOOL Compress(char* src,int srclen,char* dst,int* dstlen) = 0;
	virtual BOOL UnCompress(char* src,int srclen,char* dst,int* dstlen) = 0;
};

#endif