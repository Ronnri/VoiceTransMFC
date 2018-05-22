#include "stdafx.h"
#include "MyRTPSession.h"


#if 1

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

	CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata = (PBYTE)realloc(0, len);
	if (CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata == NULL)
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}
	else
	{
		CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].lpdata = data;
		CVoiceTransMFCDlg::m_AudioDataOut[CVoiceTransMFCDlg::nReceive].dwLength = len;
		CVoiceTransMFCDlg::nReceive = (CVoiceTransMFCDlg::nReceive + 1) % OutBlocks;
	}

}
#endif