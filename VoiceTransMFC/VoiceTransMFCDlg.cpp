
// VoiceTransMFCDlg.cpp: 实现文件
//

#include "stdafx.h"

#include "MyRTPSession.h"

#include "VoiceTransMFC.h"
#include "VoiceTransMFCDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma once
CAudioData CVoiceTransMFCDlg::m_AudioDataIn[InBlocks] = {};
CAudioData CVoiceTransMFCDlg::m_AudioDataOut[OutBlocks] = {};
int CVoiceTransMFCDlg::nAudioIn = 0;
int CVoiceTransMFCDlg::nSend = 0;
int CVoiceTransMFCDlg::nAudioOut = 0;
int CVoiceTransMFCDlg::nReceive = 0;

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
	//首先对数据进行压缩
	m_729a.Compress(((PWAVEHDR)lParam)->lpData, ((PWAVEHDR)lParam)->dwBytesRecorded,m_AComped,&m_com_length);


	int nextBlock = (nAudioIn + 1) % InBlocks;
	if (m_AudioDataIn[nextBlock].dwLength != 0)//下一“块”没发走
	{  //把PWAVEHDR(即pBUfferi)里的数据拷贝到当前“块”中

		m_AudioDataIn[nAudioIn].lpdata
			= (PBYTE)realloc(m_AudioDataIn[nAudioIn].lpdata, (m_com_length + m_AudioDataIn[nAudioIn].dwLength));
		if (m_AudioDataIn[nAudioIn].lpdata == NULL)
		{
			waveInClose(hWaveIn);
			MessageBeep(MB_ICONEXCLAMATION);
			AfxMessageBox((CString)"erro memory OnMM_WIM_DATA");
			return TRUE;
		}
		CopyMemory((m_AudioDataIn[nAudioIn].lpdata + m_AudioDataIn[nAudioIn].dwLength),
			m_AComped,
			m_com_length);//(*destination,*resource,nLen);

		m_AudioDataIn[nAudioIn].dwLength += m_com_length;

	}
	else //把PWAVEHDR(即pBUfferi)里的数据拷贝到下一“块”中
	{
		nAudioIn = (nAudioIn + 1) % InBlocks;
		m_AudioDataIn[nAudioIn].lpdata = (PBYTE)realloc
		(0, m_com_length);
		CopyMemory(m_AudioDataIn[nAudioIn].lpdata,
			m_AComped,
			m_com_length);
		m_AudioDataIn[nAudioIn].dwLength = m_com_length;

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
	m_729a.UnCompress((LPSTR)m_AudioDataOut[nAudioOut].lpdata, m_AudioDataOut[nAudioOut].dwLength, m_AUnComped, &m_uncom_length);

	// Set up header    
	pWaveHdrOut->lpData = m_AUnComped;
	pWaveHdrOut->dwBufferLength = m_uncom_length;
	pWaveHdrOut->dwBytesRecorded = 0;
	pWaveHdrOut->dwUser = 0;
	pWaveHdrOut->dwFlags = WHDR_BEGINLOOP;
	pWaveHdrOut->dwLoops = 1;
	pWaveHdrOut->lpNext = NULL;
	pWaveHdrOut->reserved = 0;
	// Prepare and write
	waveOutPrepareHeader(hWaveOut, pWaveHdrOut, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, pWaveHdrOut, sizeof(WAVEHDR));

	// Set up header    
	pWaveHdrOutcpy->lpData = m_AUnComped;
	pWaveHdrOutcpy->dwBufferLength = m_uncom_length;
	pWaveHdrOutcpy->dwBytesRecorded = 0;
	pWaveHdrOutcpy->dwUser = 0;
	pWaveHdrOutcpy->dwFlags = WHDR_BEGINLOOP;
	pWaveHdrOutcpy->dwLoops = 1;
	pWaveHdrOutcpy->lpNext = NULL;
	pWaveHdrOutcpy->reserved = 0;

	//第二通道未启用
	// Prepare and write
	//waveOutPrepareHeader(hWaveOut, pWaveHdrOutcpy, sizeof(WAVEHDR));
	//waveOutWrite(hWaveOut, pWaveHdrOutcpy, sizeof(WAVEHDR));

	return TRUE;

}

LRESULT CVoiceTransMFCDlg::OnMM_WOM_DONE(UINT wParam, LONG lParam)
{
	//if (nAudioOut != nReceive) {
		free(m_AudioDataOut[nAudioOut].lpdata);
		m_AudioDataOut[nAudioOut].lpdata = reinterpret_cast<PBYTE>(malloc(1));
		m_AudioDataOut[nAudioOut].dwLength = 0;
	//}

	nAudioOut = (nAudioOut + 1) % OutBlocks;
	m_729a.UnCompress((LPSTR)m_AudioDataOut[nAudioOut].lpdata, m_AudioDataOut[nAudioOut].dwLength,m_AUnComped, &m_uncom_length);

	((PWAVEHDR)lParam)->lpData = m_AUnComped;
	((PWAVEHDR)lParam)->dwBufferLength = m_uncom_length;
	if (((PWAVEHDR)lParam)->dwBufferLength != 0)
		TRACE("the next length %d\n", ((PWAVEHDR)lParam)->dwBufferLength);

	/*nAudioOut = (nAudioOut + 1) % OutBlocks;
	((PWAVEHDR)lParam)->lpData = (LPSTR)m_AudioDataOut[nAudioOut].lpdata;
	((PWAVEHDR)lParam)->dwBufferLength = m_AudioDataOut[nAudioOut].dwLength;
	if(((PWAVEHDR)lParam)->dwBufferLength != 0)
		TRACE("the next length %d\n", ((PWAVEHDR)lParam)->dwBufferLength);*/
	waveOutPrepareHeader(hWaveOut, (PWAVEHDR)lParam, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, (PWAVEHDR)lParam, sizeof(WAVEHDR));
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
	m_PortShow.SetWindowText(_T("9000"));
	//m_IPAddress.SetWindowText(_T("192.168.137.204"));
	//m_TargetIp = "192.168.137.204";
	m_IPAddress.SetWindowText(_T("127.0.0.1"));
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
	pWaveHdrOutcpy = reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
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

	m_AUnComped = new char[INP_BUFFER_SIZE];
	m_AComped = new char[SEND_BUF];
	m_com_length = 0;
	m_uncom_length = 0;
	
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
	m_TargetPort = _ttoi(strPortNum);
	DWORD dwIP;
	if (m_IPAddress.GetAddress(dwIP) < 4)	//获取目标IP地址
	{
		AfxMessageBox(_T("注意!\nIP地址填写不完整!"), MB_ICONWARNING);
		return;
	}
	unsigned char *pIP;
	pIP = (unsigned char*)&dwIP;
	unsigned char *IP1 = pIP + 0;
	unsigned char *IP2 = pIP + 1;
	unsigned char *IP3 = pIP + 2;
	unsigned char *IP4 = pIP + 3;
	m_TargetIp.Format(_T("%u.%u.%u.%u"), *IP1, *IP2, *IP3, *IP4);
	GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);

}


MyRTPSession sess;
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
	sessparams.SetOwnTimestampUnit(1.0 / 10.0);

	sessparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(m_TargetPort);
	status = sess.Create(sessparams, &transparams);
	checkerror(status);
	
	RTPIPv4Address addr(inet_addr((LPSTR)(LPCSTR)m_TargetIp), m_TargetPort);
	status = sess.AddDestination(addr);
	checkerror(status);
	
	myInit();
	//::AfxBeginThread(Audio_Listen_Thread, this);
	
	::AfxBeginThread(Audio_Send_Thread, this);

}

UINT Audio_Send_Thread(LPVOID lParam) {
	CVoiceTransMFCDlg *pdlg = (CVoiceTransMFCDlg*)lParam;

	while (1)//循环使用指针nSend
	{
		int length = pdlg->m_AudioDataIn[pdlg->nSend].dwLength;
		if (length == 0) continue;
		DWORD hasSent = 0;
		while (1) {
			status = sess.SendPacket(pdlg->m_AudioDataIn[pdlg->nSend].lpdata + hasSent , SEND_BUF, 0, false, 10);
			checkerror(status);
			hasSent += SEND_BUF;
			if(hasSent == length)
			{
				free(pdlg->m_AudioDataIn[pdlg->nSend].lpdata);
				pdlg->m_AudioDataIn[pdlg->nSend].dwLength = 0;
				break;
			}
			
		}
		pdlg->nSend = (pdlg->nSend + 1) % InBlocks;
	}

}
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

