#pragma once

#pragma comment(lib, "jrtplib_d.lib")
#pragma comment(lib, "jthread_d.lib")
#pragma comment(lib,"ws2_32.lib") 

#include "./lib/rtpconfig.h"
#include <iostream>

using namespace std;

#ifdef RTP_SUPPORT_THREAD

#include "./lib/rtpsession.h"
#include "./lib/rtpudpv4transmitter.h"
#include "./lib/rtpipv4address.h"
#include "./lib/rtpsessionparams.h"
#include "./lib/rtperrors.h"
#include "./lib/rtplibraryversion.h"
#include "./lib/rtpsourcedata.h"
#include "./lib/rtpabortdescriptors.h"
#include "./lib/rtpselect.h"
#include "./lib/rtprandom.h"
#include "./lib/jthread.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace jrtplib;
using namespace jthread;
#endif

class MyRTPSession : public RTPSession
{
protected:
	void OnPollThreadStep();
	void ProcessRTPPacket(const RTPSourceData &srcdat, const RTPPacket &rtppack);
};

