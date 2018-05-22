
// VoiceTransMFCDlg.cpp: 实现文件
//

#include "stdafx.h"

#include "MyRTPSession.h"

#include "VoiceTransMFC.h"
#include "VoiceTransMFCDlg.h"
#include "afxdialogex.h"
#include "g72x.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVoiceTransMFCDlg 对话框



CVoiceTransMFCDlg::CVoiceTransMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VOICETRANSMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVoiceTransMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_IPAddress);
	DDX_Control(pDX, IDC_EDIT1, m_PortShow);
}

BEGIN_MESSAGE_MAP(CVoiceTransMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MM_WIM_DATA, OnMM_WIM_DATA)
	ON_MESSAGE(MM_WIM_CLOSE, OnMM_WIM_CLOSE)
	ON_MESSAGE(MM_WOM_OPEN, OnMM_WOM_OPEN)
	ON_MESSAGE(MM_WOM_DONE, OnMM_WOM_DONE)
	ON_MESSAGE(MM_WOM_CLOSE, OnMM_WOM_CLOSE)
	ON_BN_CLICKED(IDC_YES, &CVoiceTransMFCDlg::OnBnClickedYes)
	ON_BN_CLICKED(IDC_CONNECT, &CVoiceTransMFCDlg::OnBnClickedConnect)
END_MESSAGE_MAP()

//缓冲区已满
LRESULT CVoiceTransMFCDlg::OnMM_WIM_DATA(UINT wParam, LONG lParam)
{

	int nextBlock = (nAudioIn + 1) % InBlocks;
	if (m_AudioDataIn[nextBlock].dwLength != 0)//下一“块”没发走
	{  //把PWAVEHDR(即pBUfferi)里的数据拷贝到当前“块”中

		m_AudioDataIn[nAudioIn].lpdata
			= (PBYTE)realloc(m_AudioDataIn[nAudioIn].lpdata, (((PWAVEHDR)lParam)->dwBytesRecorded + m_AudioDataIn[nAudioIn].dwLength));
		if (m_AudioDataIn[nAudioIn].lpdata == NULL)
		{
			waveInClose(hWaveIn);
			MessageBeep(MB_ICONEXCLAMATION);
			AfxMessageBox((CString)"erro memory OnMM_WIM_DATA");
			return TRUE;
		}
		CopyMemory((m_AudioDataIn[nAudioIn].lpdata + m_AudioDataIn[nAudioIn].dwLength),
			((PWAVEHDR)lParam)->lpData,
			((PWAVEHDR)lParam)->dwBytesRecorded);//(*destination,*resource,nLen);

		m_AudioDataIn[nAudioIn].dwLength += ((PWAVEHDR)lParam)->dwBytesRecorded;

	}
	else //把PWAVEHDR(即pBUfferi)里的数据拷贝到下一“块”中
	{
		nAudioIn = (nAudioIn + 1) % InBlocks;
		m_AudioDataIn[nAudioIn].lpdata = (PBYTE)realloc
		(0, ((PWAVEHDR)lParam)->dwBytesRecorded);
		CopyMemory(m_AudioDataIn[nAudioIn].lpdata,
			((PWAVEHDR)lParam)->lpData,
			((PWAVEHDR)lParam)->dwBytesRecorded);
		m_AudioDataIn[nAudioIn].dwLength = ((PWAVEHDR)lParam)->dwBytesRecorded;

	}
	// Send out a new buffer	
	waveInAddBuffer(hWaveIn, (PWAVEHDR)lParam, sizeof(WAVEHDR));
	return TRUE;


}

LRESULT CVoiceTransMFCDlg::OnMM_WIM_CLOSE(UINT wParam, LONG lParam)
{

	waveInUnprepareHeader(hWaveIn, pWaveHdr1, sizeof(WAVEHDR));
	waveInUnprepareHeader(hWaveIn, pWaveHdr2, sizeof(WAVEHDR));

	free(pBuffer1);
	free(pBuffer2);

	return TRUE;
}

LRESULT CVoiceTransMFCDlg::OnMM_WOM_OPEN(UINT wParam, LONG lParam)
{
	// Set up header    
	pWaveHdrOut->lpData = (LPSTR)m_AudioDataOut[nAudioOut].lpdata;
	pWaveHdrOut->dwBufferLength = m_AudioDataOut[nAudioOut].dwLength;
	pWaveHdrOut->dwBytesRecorded = 0;
	pWaveHdrOut->dwUser = 0;
	pWaveHdrOut->dwFlags = WHDR_BEGINLOOP;
	pWaveHdrOut->dwLoops = 1;
	pWaveHdrOut->lpNext = NULL;
	pWaveHdrOut->reserved = 0;

	// Prepare and write
	waveOutPrepareHeader(hWaveOut, pWaveHdrOut, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, pWaveHdrOut, sizeof(WAVEHDR));

	return TRUE;

}

LRESULT CVoiceTransMFCDlg::OnMM_WOM_DONE(UINT wParam, LONG lParam)
{
	free(m_AudioDataOut[nAudioOut].lpdata);
	m_AudioDataOut[nAudioOut].lpdata = reinterpret_cast<PBYTE>(malloc(1));
	m_AudioDataOut[nAudioOut].dwLength = 0;

	nAudioOut = (nAudioOut + 1) % OutBlocks;
	((PWAVEHDR)lParam)->lpData = (LPSTR)m_AudioDataOut[nAudioOut].lpdata;
	((PWAVEHDR)lParam)->dwBufferLength = m_AudioDataOut[nAudioOut].dwLength;
	TRACE("the next length %d\n", ((PWAVEHDR)lParam)->dwBufferLength);
	waveOutPrepareHeader(hWaveOut, (PWAVEHDR)lParam, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, (PWAVEHDR)lParam, sizeof(WAVEHDR));//cut
	return TRUE;

}

LRESULT CVoiceTransMFCDlg::OnMM_WOM_CLOSE(UINT wParam, LONG lParam)
{
	waveOutUnprepareHeader(hWaveOut, pWaveHdrOut, sizeof(WAVEHDR));

	int i;
	//release all the memory of the AudioData
	for (i = 0; i<InBlocks; i++)
	{
		if (m_AudioDataIn[i].dwLength != 0)
			free(m_AudioDataIn[i].lpdata);

	}
	for (i = 0; i<OutBlocks; i++)
	{
		if (m_AudioDataOut[i].dwLength != 0)
			free(m_AudioDataOut[i].lpdata);
	}
	return TRUE;
}

// CVoiceTransMFCDlg 消息处理程序

BOOL CVoiceTransMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//----------------------------------------------------------------------
	m_IsSocCreated = FALSE;
	m_PortShow.SetWindowTextW(_T("9000"));
	m_IPAddress.SetWindowTextW(_T("127.0.0.1"));
	m_TargetIp = "127.0.0.1";
	m_TargetPort = 9000;
	GetDlgItem(IDC_IPADDRESS1)->EnableWindow(TRUE);//IP ADDR
	GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_YES)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT1)->EnableWindow(TRUE);//PORT

	//allocate memory for wave header
	pWaveHdr1 = reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
	pWaveHdr2 = reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
	pWaveHdrOut = reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
	//***reinterpret_cast ****  operation is used to convert any type of
	//pointer to other type
	//allocate memory for save buffer
	int i;
	for (i = 0; i<InBlocks; i++)
	{
		m_AudioDataIn[i].dwLength = 0;
		m_AudioDataIn[i].lpdata = reinterpret_cast<PBYTE>(malloc(1));

	}

	for (i = 0; i<OutBlocks; i++)
	{
		m_AudioDataOut[i].dwLength = 0;
		m_AudioDataOut[i].lpdata = reinterpret_cast<PBYTE>(malloc(1));
	}
	nAudioIn = 0;
	nAudioOut = 0;
	nSend = 0;
	nReceive = 0;
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CVoiceTransMFCDlg::myInit(void) {

	
	
	//allocate buffer memory
	pBuffer1 = (PBYTE)malloc(INP_BUFFER_SIZE);
	pBuffer2 = (PBYTE)malloc(INP_BUFFER_SIZE);
	if (!pBuffer1 || !pBuffer2)
	{
		if (pBuffer1) free(pBuffer1);
		if (pBuffer2) free(pBuffer2);
		MessageBeep(MB_ICONEXCLAMATION);
		MessageBox(_T("Memory erro!"));
		return;
	}

	//open waveform audio for input

	m_waveformin.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	m_waveformin.nChannels = 1;//设置音频文件的通道数量
	m_waveformin.nSamplesPerSec = 8000;//设置每个声道播放和记录时的样本频率
	m_waveformin.wBitsPerSample = 16;
	m_waveformin.nBlockAlign = m_waveformin.nChannels * (m_waveformin.wBitsPerSample / 8);//以字节为单位设置块对齐
	m_waveformin.nAvgBytesPerSec = m_waveformin.nSamplesPerSec * m_waveformin.nBlockAlign; //设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	m_waveformin.cbSize = 0;//额外信息的大小


	if (waveInOpen(&hWaveIn, WAVE_MAPPER, &m_waveformin, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW))
	{   //打开录音设备函数 
		free(pBuffer1);
		free(pBuffer2);
		MessageBeep(MB_ICONEXCLAMATION);
		MessageBox(_T("Audio can not be open!"));
	}
	pWaveHdr1->lpData = (LPSTR)pBuffer1;
	pWaveHdr1->dwBufferLength = INP_BUFFER_SIZE;
	pWaveHdr1->dwBytesRecorded = 0;
	pWaveHdr1->dwUser = 0;
	pWaveHdr1->dwFlags = 0;
	pWaveHdr1->dwLoops = 1;
	pWaveHdr1->lpNext = NULL;
	pWaveHdr1->reserved = 0;

	waveInPrepareHeader(hWaveIn, pWaveHdr1, sizeof(WAVEHDR));

	pWaveHdr2->lpData = (LPSTR)pBuffer2;
	pWaveHdr2->dwBufferLength = INP_BUFFER_SIZE;
	pWaveHdr2->dwBytesRecorded = 0;
	pWaveHdr2->dwUser = 0;
	pWaveHdr2->dwFlags = 0;
	pWaveHdr2->dwLoops = 1;
	pWaveHdr2->lpNext = NULL;
	pWaveHdr2->reserved = 0;

	waveInPrepareHeader(hWaveIn, pWaveHdr2, sizeof(WAVEHDR));

	// Add the buffers

	waveInAddBuffer(hWaveIn, pWaveHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, pWaveHdr2, sizeof(WAVEHDR));

	// Begin sampling
	waveInStart(hWaveIn);

	m_waveformout.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	m_waveformout.nChannels = 1;//设置音频文件的通道数量
	m_waveformout.nSamplesPerSec = 8000;//设置每个声道播放和记录时的样本频率
	m_waveformout.wBitsPerSample = 16;
	m_waveformout.nBlockAlign = m_waveformout.nChannels * (m_waveformout.wBitsPerSample / 8);//以字节为单位设置块对齐
	m_waveformout.nAvgBytesPerSec = m_waveformout.nSamplesPerSec * m_waveformout.nBlockAlign; //设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	m_waveformout.cbSize = 0;//额外信息的大小


	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &m_waveformout, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW)) {
		MessageBeep(MB_ICONEXCLAMATION);
		MessageBox(_T("Audio output erro"));
	}
}
void CVoiceTransMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVoiceTransMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVoiceTransMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVoiceTransMFCDlg::OnOK()
{
	// TODO: Add extra validation here
	waveInReset(hWaveIn);
	waveInClose(hWaveIn);

	waveOutReset(hWaveOut);
	waveOutClose(hWaveOut);

	
	CDialog::OnOK();
}




void CVoiceTransMFCDlg::OnBnClickedYes()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strPortNum;
	m_PortShow.GetWindowText(strPortNum);//获取端口号
	m_TargetPort = _wtoi(strPortNum);
	DWORD dwIP;
	if (m_IPAddress.GetAddress(dwIP) < 4)	//获取目标IP地址
	{
		AfxMessageBox(_T("注意!\nIP地址填写不完整!"), MB_ICONWARNING);
		return;
	}
	unsigned char *pIP;
	pIP = (unsigned char*)&dwIP;
	unsigned char *IP1 = pIP + 3;
	unsigned char *IP2 = pIP + 2;
	unsigned char *IP3 = pIP + 1;
	unsigned char *IP4 = pIP;
	m_TargetIp.Format(_T("%u.%u.%u.%u"), *IP1, *IP2, *IP3, *IP4);
	GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);

}

RTPSession sess;
int status;

void CVoiceTransMFCDlg::OnBnClickedConnect()//客户端连接服务器
{

	// TODO: 在此添加控件通知处理程序代码
#ifdef RTP_SOCKETTYPE_WINSOCK
	WSADATA dat;
	WSAStartup(MAKEWORD(2, 2), &dat);
#endif // RTP_SOCKETTYPE_WINSOCK
	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessparams;
	sessparams.SetOwnTimestampUnit(1.0 / 8000.0);

	sessparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(m_TargetPort);
	status = sess.Create(sessparams, &transparams);
	checkerror(status);

#include <afxpriv.h>
	
	USES_CONVERSION;
	LPSTR charIP = T2A(m_TargetIp);
	RTPIPv4Address addr(inet_addr(charIP), m_TargetPort);
	status = sess.AddDestination(addr);
	checkerror(status);


	myInit();
	//::AfxBeginThread(Audio_Listen_Thread, this);
	
	::AfxBeginThread(Audio_Send_Thread, this);

}
#if 1
UINT Audio_Send_Thread(LPVOID lParam) {
	CVoiceTransMFCDlg *pdlg = (CVoiceTransMFCDlg*)lParam;
	while (1)//循环使用指针nSend
	{
		int length = pdlg->m_AudioDataIn[pdlg->nSend].dwLength;
		if (length == 0) continue;
	
		while (1) {
			status = sess.SendPacket(pdlg->m_AudioDataIn[pdlg->nSend].lpdata , length, 0, false, 10);
			checkerror(status);
			if(status >= 0)
			{
				free(pdlg->m_AudioDataIn[pdlg->nSend].lpdata);
				pdlg->m_AudioDataIn[pdlg->nSend].dwLength = 0;
				break;
			}
		}
		pdlg->nSend = (pdlg->nSend + 1) % InBlocks;
	}

}

UINT Audio_Listen_Thread(LPVOID lParam) {
	CVoiceTransMFCDlg *pdlg = (CVoiceTransMFCDlg*)lParam;
	sess.BeginDataAccess();
	while (1) {

		// check incoming packets
		if (sess.GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket *pack;
				while ((pack = sess.GetNextPacket()) != NULL)
				{
					// You can examine the data here
					uint8_t *data = pack->GetPayloadData();
					size_t len = pack->GetPayloadLength();

					pdlg->m_AudioDataOut[pdlg->nReceive].lpdata = (PBYTE)realloc(0, len);
					if (pdlg->m_AudioDataOut[pdlg->nReceive].lpdata == NULL)
					{
						MessageBeep(MB_ICONEXCLAMATION);
						MessageBox(NULL, (CString)"erro memory_ReceiveAudio", (CString)"提示", MB_OK);
						pdlg->OnOK();
						return -1;
					}
					else//内存申请成功，可以进行循环检测接受
					{
						pdlg->m_AudioDataOut[pdlg->nReceive].lpdata = data;
						pdlg->m_AudioDataOut[pdlg->nReceive].dwLength = len;
						break;
					}
				}//本轮声音文件接收完毕	

					pdlg->nReceive = (pdlg->nReceive + 1) % OutBlocks;
					// we don't longer need the packet, so
					// we'll delete it
					sess.DeletePacket(pack);
			} while (sess.GotoNextSourceWithData());
		}
		sess.EndDataAccess();


		RTPTime::Wait(RTPTime(1, 0));
	}
	


	sess.BYEDestroy(RTPTime(10, 0), 0, 0);

#ifdef RTP_SOCKETTYPE_WINSOCK
	WSACleanup();
#endif // RTP_SOCKETTYPE_WINSOCK
}
#elif 0
UINT Audio_Send_Thread(LPVOID lParam)//声音发送线程
{
	CVoiceTransMFCDlg *pdlg = (CVoiceTransMFCDlg*)lParam;
	CSocket m_Client;
	BOOL isConnected = FALSE;
	m_Client.Create();
	try {
		if (m_Client.Connect(pdlg->m_TargetIp, pdlg->m_TargetPort))
			isConnected = TRUE;
	}
	catch (...) {
		m_Client.Close();
		throw;
	}
	if (isConnected)
	{
		pdlg->m_IsSocCreated = TRUE;
		DWORD ret, length;
		int count = 0;
		while (1)//循环使用指针nSend
		{
			
			length = pdlg->m_AudioDataIn[pdlg->nSend].dwLength;
			if (length != 0)
			{   //首先发送块的长度
				if (((ret = m_Client.Send(&length, sizeof(DWORD))) != sizeof(DWORD)) || (ret == SOCKET_ERROR))
				{
					AfxMessageBox((CString)"声音文件头传输错误!" + pdlg->GetError(GetLastError()));
					pdlg->OnOK();
					break;
				}//其次发送块的内容	,循环检测发送
				DWORD dwSent = 0;//已经发送掉的字节数
				while (1)//==============================发送声音文件开始
				{
#if 0		
					va_g729a_init_encoder();


					char *pout = 0;
					va_g729a_encoder((short*)pdlg->m_AudioDataIn[pdlg->nSend].lpdata + dwSent, (unsigned char*)pout);
				
					ret = m_Client.Send(pout, sizeof(pout));

					if (ret == SOCKET_ERROR)//检错
					{
						AfxMessageBox((CString)"声音文件传输错误!" + pdlg->GetError(GetLastError()));
						pdlg->OnOK();
						break;
					}
					else //发送未发送完的
					{
						dwSent += ret;
						if (dwSent == length)
						{
							free(pdlg->m_AudioDataIn[pdlg->nSend].lpdata);
							pdlg->m_AudioDataIn[pdlg->nSend].dwLength = 0;
							break;
						}
					}
#elif 0
					//进行g726编码，发送编译过的信号
					char *pout = "";
					g726_Encode(pdlg->m_AudioDataIn[pdlg->nSend].lpdata + dwSent, pout);
					ret = m_Client.Send(pout,sizeof(pout));

					if (ret == SOCKET_ERROR)//检错
					{
						AfxMessageBox((CString)"声音文件传输错误!" + pdlg->GetError(GetLastError()));
						pdlg->OnOK();
						break;
					}
					else //发送未发送完的
					{
						dwSent += ret;
						if (dwSent == length)
						{
							free(pdlg->m_AudioDataIn[pdlg->nSend].lpdata);
							pdlg->m_AudioDataIn[pdlg->nSend].dwLength = 0;
							break;
						}
					}
			
#elif 1
					ret = m_Client.Send((pdlg->m_AudioDataIn[pdlg->nSend].lpdata + dwSent), (length - dwSent));
					if (ret == SOCKET_ERROR)//检错
					{
						AfxMessageBox((CString)"声音文件传输错误!" + pdlg->GetError(GetLastError()));
						pdlg->OnOK();
						break;
					}
					else //发送未发送完的
					{
						dwSent += ret;
						if (dwSent == length)
						{
							free(pdlg->m_AudioDataIn[pdlg->nSend].lpdata);
							pdlg->m_AudioDataIn[pdlg->nSend].dwLength = 0;
							break;
						}
					}
#endif
				}  //======================================发送声音文件结束
			}
			pdlg->nSend = (pdlg->nSend + 1) % InBlocks;

		}
	}
	else
		AfxMessageBox((CString)"Socket连接失败" + pdlg->GetError(GetLastError()));
	
	m_Client.Close();
	return 0;
}
UINT Audio_Listen_Thread(LPVOID lParam)//声音接收线程
{
	CVoiceTransMFCDlg *pdlg = (CVoiceTransMFCDlg*)lParam;
	CSocket m_Server;
	DWORD 	length;
	try {
		if (!m_Server.Create(pdlg->m_TargetPort))//初始化错误
			AfxMessageBox((CString)"Listen Socket create error" + pdlg->GetError(GetLastError()));//不可单个运行，重复产生同一端口显然是错误的
	}
	catch (...) {
		m_Server.Close();
		throw;
	}
	if (!m_Server.Listen())
		AfxMessageBox((CString)"m_server.Listen ERROR" + pdlg->GetError(GetLastError()));
	CSocket recSo;
	try
	{
		if (!m_Server.Accept(recSo))
			AfxMessageBox((CString)"m_server.Accept() error" + pdlg->GetError(GetLastError()));
	}
	catch (...)
	{
		m_Server.Close();
	}
	m_Server.Close();
	int sendcount = 0;//just for test
	int ret; 
	pdlg->m_IsSocCreated = TRUE;
	while (1)
	{   //开始循环接收声音文件，首先接收文件长度
		ret = recSo.Receive(&length, sizeof(DWORD));
		if (ret == SOCKET_ERROR)
			AfxMessageBox((CString)"服务器端接收声音文件长度出错，原因： " + pdlg->GetError(GetLastError()));
		if (ret != sizeof(DWORD))
		{
			MessageBox(NULL, (CString)"接收文件头错误，将关闭该线程。", (CString)"提示", MB_OK);
			recSo.Close();
			return -1;
		}//接下来开辟length长的内存空间
		pdlg->m_AudioDataOut[pdlg->nReceive].lpdata = (PBYTE)realloc(0, length);
		if (pdlg->m_AudioDataOut[pdlg->nReceive].lpdata == NULL)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			MessageBox(NULL, (CString)"erro memory_ReceiveAudio", (CString)"提示", MB_OK);
			pdlg->OnOK();
			recSo.Close();
			return -1;
		}
		else//内存申请成功，可以进行循环检测接受
		{
			DWORD dwReceived = 0, dwret;
			while (length>dwReceived)
			{
				dwret = recSo.Receive((pdlg->m_AudioDataOut[pdlg->nReceive].lpdata + dwReceived), (length - dwReceived));
				dwReceived += dwret;
				if (dwReceived == length)
				{
					pdlg->m_AudioDataOut[pdlg->nReceive].dwLength = length;
					break;
				}

			}
		}//本轮声音文件接收完毕	
		pdlg->nReceive = (pdlg->nReceive + 1) % OutBlocks;
	}
	recSo.Close();
	pdlg->OnOK();
	return 0;
}
#endif 
CString CVoiceTransMFCDlg::GetError(DWORD error)	//返回错误信息
{
	CString strError;
	switch (error)
	{
	case WSANOTINITIALISED:
		strError = "初始化错误";
		break;
	case WSAENOTCONN:
		strError = "对方没有启动";
		break;
	case WSAEWOULDBLOCK:
		strError = "对方已经关闭";
		break;
	case WSAECONNREFUSED:
		strError = "连接的尝试被拒绝";
		break;
	case WSAENOTSOCK:
		strError = "在一个非套接字上尝试了一个操作";
		break;
	case WSAEADDRINUSE:
		strError = "特定的地址已在使用中";
		break;
	case WSAECONNRESET:
		strError = "与主机的连接被关闭";
		break;
	default:
		strError = "一般错误";
	}
	return strError;

}

