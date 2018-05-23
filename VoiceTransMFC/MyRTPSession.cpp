#include "stdafx.h"
#include "MyRTPSession.h"

#pragma once
#if 1
DWORD MyRTPSession::hasRecv = 0;
void MyRTPSession::OnPollThreadStep()
{
	BeginDataAccess();

	// check incoming packets
	if (GotoFirstSourceWithData())
	{
		do
		{
			RTPPacket *pack;
			RTPSourceData *srcdat;
		
			srcdat = GetCurrentSourceInfo();
		
			while ((pack = GetNextPacket()) != NULL)
			{
				ProcessRTPPacket(*srcdat, *pack);

				
				DeletePacket(pack);
			}
		} while (GotoNextSourceWithData());
	}

	EndDataAccess();
}

void MyRTPSession::ProcessRTPPacket(const RTPSourceData &srcdat, const RTPPacket &rtppack)
{
	// You can inspect the packet and the source's info here
	//std::cout << "Got packet " << rtppack.GetExtendedSequenceNumber() << " from SSRC " << srcdat.GetSSRC() << std::endl;
	//std::cout << rtppack.GetPayloadData() << std::endl;
	//std::cout << rtppack.GetPayloadLength() << std::endl;
	uint8_t *data = rtppack.GetPayloadData();
	size_t len = rtppack.GetPayloadLength();

	if (len == SEND_BUF) {
		CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata = (PBYTE)realloc(0, SEND_BUF);
		if (CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata == NULL)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			return;
		}
		CopyMemory(CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata, data, len);
		CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].dwLength = len;
		CVoiceTransMFCDlg::nReceive = (CVoiceTransMFCDlg::nReceive + 1) % OutBlocks;
	}

	//if (hasRecv == SEND_BUF) {
	//	//一个缓冲区满了

	//	CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].dwLength = hasRecv;

	//	CVoiceTransMFCDlg::nReceive = (CVoiceTransMFCDlg::nReceive + 1) % OutBlocks;
	//	hasRecv = 0;
	//}
	//if (hasRecv == 0) {
	//	//开辟新的缓冲

	//	CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata = (PBYTE)realloc(0, SEND_BUF);
	//	if (CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata == NULL)
	//	{
	//		MessageBeep(MB_ICONEXCLAMATION);
	//		return;
	//	}
	//	CopyMemory(CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata, data, len);

	//	hasRecv += len;
	//}
	//if (hasRecv != 0 && hasRecv != SEND_BUF) {
	//
	//	CopyMemory(CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata + hasRecv, data, len);

	//	hasRecv += len;
	//}
}
#endif