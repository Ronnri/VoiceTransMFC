#include "stdafx.h"
#include "./lib/G729aCompress.h"
#include "VoiceTransMFCDlg.h"


CG729aCompress::CG729aCompress()
{
	va_g729a_init_encoder();
	va_g729a_init_decoder();
}

CG729aCompress::~CG729aCompress()
{
}

BOOL CG729aCompress::Compress(char* src, int srclen, char* dst, int* dstlen)
{
	//	TRACE("CG729aCompress::Compress %d Bytes\n",srclen);
	if (!src || srclen != INP_BUFFER_SIZE || !dst)
		return FALSE;

	for (int i = 0; i < SEND_BUF / 10; i++)
	{
		va_g729a_encoder((short*)(src + i * 160), (BYTE*)dst + i * 10);
	}

	if (dstlen)
		*dstlen = SEND_BUF;
	//	TRACE("CG729aCompress::UnCompress Done\n");
	return TRUE;
}

BOOL CG729aCompress::UnCompress(char* src, int srclen, char* dst, int* dstlen)
{
	//	TRACE("CG729aCompress::UnCompress %d Bytes\n",srclen);
	if (!src || srclen != SEND_BUF || !dst)
		return FALSE;

	for (int i = 0; i < SEND_BUF / 10; i++)
	{
		va_g729a_decoder((BYTE*)src + i * 10, (short*)(dst + i * 160), 0);
	}

	if (dstlen)
		*dstlen = INP_BUFFER_SIZE;
	//	TRACE("CG729aCompress::UnCompress Done\n");
	return TRUE;
}