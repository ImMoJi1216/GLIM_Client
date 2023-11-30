
// GLIM_ClientDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "GLIM_Client.h"
#include "GLIM_ClientDlg.h"

#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CGLIMClientDlg 대화 상자



CGLIMClientDlg::CGLIMClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GLIM_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// 성훈
	capture = nullptr;
	cimage_mfc = nullptr;
}

void CGLIMClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_PIC, m_picture);
}

BEGIN_MESSAGE_MAP(CGLIMClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CGLIMClientDlg::Btn_StartThread)
	ON_BN_CLICKED(IDC_BUTTON2, &CGLIMClientDlg::Btn_EndThread)
	ON_BN_CLICKED(IDC_BUTTON3, &CGLIMClientDlg::Btn_Inspection)
END_MESSAGE_MAP()


// CGLIMClientDlg 메시지 처리기

BOOL CGLIMClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// 서버연결
	m_Socket.Create();
	if (m_Socket.Connect(_T("10.10.21.118"), 25000) == FALSE)
	{
		AfxMessageBox(_T("ERROR : Failed to connect Server"));
		PostQuitMessage(0);
		return FALSE;
	}
	
	// 아두이노 연결
	m_pArduino = new Arduino("\\\\.\\COM4");

	if (m_pArduino->IsConnected())
	{
		// 시리얼 통신이 연결되었음을 알림
		m_List.AddString(_T("시리얼 통신 연결 성공"));
	}
	else
	{
		// 시리얼 통신 연결 실패
		m_List.AddString(_T("시리얼 통신 연결 실패"));
		// 실패 시 적절한 처리를 추가할 수 있습니다.
	}
	// 웹캠연결
	capture = new VideoCapture(0, CAP_DSHOW);
	if (!capture->isOpened())
	{
		m_List.AddString(_T("ERROR : 웹캠 연결 실패"));
	}
	else
	{
		m_List.AddString(_T("LOG : 웹캠 연결 성공"));
		Sleep(200);
		//cv::Size frameSize(1920, 1080);  // 적절한 해상도로 변경
		//capture->set(CAP_PROP_FRAME_WIDTH, frameSize.width);
		//capture->set(CAP_PROP_FRAME_HEIGHT, frameSize.height);


		//웹캠 크기를  320x240으로 지정

		// 기본사이즈
		//capture->set(CAP_PROP_FRAME_WIDTH, 320);
		//capture->set(CAP_PROP_FRAME_HEIGHT, 240);

		// 맞춤 사이즈
		capture->set(CAP_PROP_FRAME_WIDTH, 380);
		capture->set(CAP_PROP_FRAME_HEIGHT, 305);

		// 고화질
		//capture->set(CAP_PROP_FRAME_WIDTH, 1920);
		//capture->set(CAP_PROP_FRAME_HEIGHT, 1080);
		//cimage_mfc = NULL;

		// 캡처 버퍼 초기화
		capture->grab();
		SetTimer(1000, 60, NULL);
	}

	m_Arduino_thread = std::thread(&CGLIMClientDlg::rec_Arduino, this);
	m_Arduino_thread.detach();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CGLIMClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CGLIMClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CGLIMClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGLIMClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

void CGLIMClientDlg::OnClose()
{
	// 스레드 종료
	m_bThreadRunning = false;
	if (m_thread.joinable())
	{
		m_thread.join();
	}
	// 웹캠 종료
	if (capture)
	{
		capture->release();
		delete capture;
	}
	CDialogEx::OnClose();
}

void CGLIMClientDlg::Btn_Inspection()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//m_bInspection = true;
}

// 웹캠 연결 시작
void CGLIMClientDlg::Btn_StartThread()
{
	////스레드 시작할꺼임
	//if (m_thread.joinable())
	//{
	//	m_List.AddString(_T("LOG : 이미 웹캠 캡쳐가 진행중입니다"));
	//}
	//else
	//{
	//	m_bThreadRunning = true;
	//	m_thread = std::thread(&CGLIMClientDlg::ImageSendThread, this);
	//	m_List.AddString(_T("LOG : 웹캠 캡쳐 시작됨"));
	//}
}

// 웹캠 연결 종료
void CGLIMClientDlg::Btn_EndThread()
{
	// 스레드 종료할꺼임
	m_bThreadRunning = false;
	// 스레드가 작동상태라면
	if (m_thread.joinable())
	{
		// 스레드 종료 대기
		m_thread.join();
		m_List.AddString(_T("LOG : 웹캠 캡쳐 종료됨"));
	}
}

void CGLIMClientDlg::ImageSendThread()
{
	while (m_bThreadRunning)
	{
		// VideoCapture 객체가 현재 프레임을 캡처할 수 있는지 확인(캡쳐시 true, 노캡쳐시 false)
		if (capture->grab())
		{
			if (capture->retrieve(mat_frame, 0))
			{
				// 프레임이 정상적으로 읽어지면 처리
				// 카메라에서 프레임을 읽어옴
				capture->read(mat_frame);

				// 비트 당 픽셀 수 계산
				int bpp = 8 * mat_frame.elemSize();
				assert((bpp == 8 || bpp == 24 || bpp == 32));  // 적절한 비트 당 픽셀 수가 아니면 프로그램 중단

				// 이미지가 32비트 미만의 비트 당 픽셀 수를 가질 때, 패딩 계산
				int padding = 0;
				if (bpp < 32)
					padding = 4 - (mat_frame.cols % 4);

				// 패딩이 4이면 0으로 설정
				if (padding == 4)
					padding = 0;

				// 이미지가 32비트 미만의 비트 당 픽셀 수를 가질 때, 이미지 오른쪽에 추가해야 하는 픽셀 수 계산
				int border = 0;
				if (bpp < 32)
				{
					border = 4 - (mat_frame.cols % 4);
				}

				// 임시 Mat 객체 생성
				Mat mat_temp;

				// 이미지의 오른쪽에 추가해야 하는 픽셀이 있거나 이미지가 연속적이지 않은 경우
				if (border > 0 || mat_frame.isContinuous() == false)
				{
					// copyMakeBorder 함수를 사용하여 경계 추가
					copyMakeBorder(mat_frame, mat_temp, 0, 0, 0, border, cv::BORDER_CONSTANT, 0);
				}
				else
				{
					// 추가해야 할 픽셀이 없거나 이미지가 연속적인 경우
					mat_temp = mat_frame;
				}

				// m_picture 컨트롤의 클라이언트 영역 크기를 얻음
				RECT r;
				m_picture.GetClientRect(&r);

				// 윈도우 크기를 나타내는 Size 객체 생성
				Size winSize(r.right, r.bottom);

				// CImage 객체 동적 할당
				cimage_mfc = new CImage();
				// CImage 객체 생성
				cimage_mfc->Create(winSize.width, winSize.height, 24);
				// BITMAPINFO 구조체 동적 할당
				BITMAPINFO* bitInfo = new BITMAPINFO;
				// 비트맵 정보 설정
				bitInfo->bmiHeader.biBitCount = bpp;					// 비트 당 픽셀 수 (색상 깊이)
				bitInfo->bmiHeader.biWidth = mat_temp.cols;				// 이미지 너비
				bitInfo->bmiHeader.biHeight = -mat_temp.rows;			// 이미지 높이 (음수로 설정하여 상하 반전)
				bitInfo->bmiHeader.biPlanes = 1;						// 평면 수 (항상 1)
				bitInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);	// BITMAPINFOHEADER의 크기
				bitInfo->bmiHeader.biCompression = BI_RGB;				// 압축 방식 (RGB는 압축하지 않음)
				bitInfo->bmiHeader.biClrImportant = 0;					// 중요한 색 인덱스 수 (0은 모두 중요)
				bitInfo->bmiHeader.biClrUsed = 0;						// 실제 사용되는 색 인덱스 수
				bitInfo->bmiHeader.biSizeImage = 0;						// 이미지 데이터의 크기 (0은 압축하지 않은 경우 사용)
				bitInfo->bmiHeader.biXPelsPerMeter = 0;					// 가로 해상도 (미터당 픽셀 수, 0은 알 수 없음)
				bitInfo->bmiHeader.biYPelsPerMeter = 0;					// 세로 해상도 (미터당 픽셀 수, 0은 알 수 없음)

				// 이미지의 너비와 높이가 윈도우의 크기와 일치하는 경우에만 이미지를 출력
				if (mat_temp.cols == winSize.width && mat_temp.rows == winSize.height)
				{
					// CImage의 DC를 얻어와서 SetDIBitsToDevice 함수를 사용하여 이미지를 윈도우에 그림
					SetDIBitsToDevice(
						cimage_mfc->GetDC(),							// CImage의 DC
						0, 0, winSize.width, winSize.height,			// 출력 영역의 좌표 및 크기
						0, 0, 0, mat_temp.rows,							// 입력 영역의 좌표 및 크기
						mat_temp.data,									// 이미지 데이터
						bitInfo,										// 비트맵 정보
						DIB_RGB_COLORS									// 색상 사용 방식 (이 경우는 RGB 색상 사용)
					);
				}
				else
				{
					// 이미지의 너비와 높이가 윈도우의 크기와 일치하지 않는 경우,
					// StretchDIBits 함수를 사용하여 이미지를 윈도우에 그림

					// 목적지 사각형 (Destination Rectangle) 설정
					int destx = 0, desty = 0;							// 출력 영역의 좌표
					int destw = winSize.width;							// 출력 영역의 너비
					int desth = winSize.height;							// 출력 영역의 높이

					// 원본 비트맵에 정의된 사각형 (Source Rectangle) 설정
					// mat_temp.cols에서 border를 뺀 것은 패딩된 픽셀을 무시하기 위함
					int imgx = 0, imgy = 0;								// 입력 영역의 좌표
					int imgWidth = mat_temp.cols - border;				// 입력 영역의 너비
					int imgHeight = mat_temp.rows;						// 입력 영역의 높이

					// StretchDIBits 함수를 사용하여 이미지를 윈도우에 그림
					StretchDIBits(
						cimage_mfc->GetDC(),							// CImage의 DC
						destx, desty, destw, desth,						// 출력 영역의 좌표 및 크기
						imgx, imgy, imgWidth, imgHeight,				// 입력 영역의 좌표 및 크기
						mat_temp.data,									// 이미지 데이터
						bitInfo,										// 비트맵 정보
						DIB_RGB_COLORS,									// 색상 사용 방식 (이 경우는 RGB 색상 사용)
						SRCCOPY											// 이미지를 복사하여 출력
					);
				}

				//cimage_mfc->Draw(::GetDC(m_picture.m_hWnd), 0, 0, cimage_mfc->GetWidth(), cimage_mfc->GetHeight());
				cimage_mfc->BitBlt(::GetDC(m_picture.m_hWnd), 0, 0);

				// 임호진이 추가함
				static DWORD lastSendTime = 0;
				// 현재 시간 가져오기
				DWORD currentTime = GetTickCount();

				TRACE(_T("Boolean Value: %d\n"), m_bInspection ? 1 : 0);

				if (m_bInspection)     // 사진 저장후 전송
				{
					m_List.AddString(_T("LOG : 사진 찰칵"));

					std::future<void> result = std::async(std::launch::async, [this, &mat_temp]() {
						Saveing_IMG(mat_temp);
						});

					result.get();  // 작업이 완료될때까지 대기
				}

				if (currentTime - lastSendTime >= SEND_INTERVAL) // 실시간 전송
				{
					Send_Streaming(mat_temp);
				}

				if (cimage_mfc)
				{
					cimage_mfc->ReleaseDC();
					delete cimage_mfc;
					cimage_mfc = nullptr;
				}
				if (bitInfo)
				{
					delete bitInfo;
					bitInfo = nullptr;
				}
			}
			Sleep(80); // 약 60프레임 
		}
	}
}

void CGLIMClientDlg::Saveing_IMG(Mat &mat_temp)
{
	imwrite("C:/Users/LMS23/Desktop/GLIM_Client_Temp/image.jpg", mat_temp);

	CString filePath = _T("C:/Users/LMS23/Desktop/GLIM_Client_Temp/image.jpg");	// 파일 경로
	CString fileName = _T("image.jpg");											// 파일 이름

	CFile file;
	if (file.Open(filePath, CFile::modeRead))		// 읽기전용
	{
		ULONGLONG fileSize = file.GetLength();		// 파일크기
		char* fileData = new char[fileSize];
		file.Read(fileData, (UINT)fileSize);
		file.Close();

		CString strMessageToSend;
		strMessageToSend.Format(_T("Type:File,%I64u,%s"), fileSize, fileName);					// 파일 크기와 파일 이름을 구분자 ,로 구분하여 문자열로 만들기
		m_Socket.Send((LPVOID)(LPCTSTR)strMessageToSend, strMessageToSend.GetLength() * 2);		// 서버로 파일 크기와 파일 이름을 포함한 문자열을 전송
		if (fileSize <= 100000)
		{
			Sleep(1000);																		// 1초동안 딜레이 (서버에서의 버퍼 준비시간)
		}
		else if (fileSize <= 1000000)
		{
			Sleep(3000);																		// 3초동안 딜레이 (서버에서의 버퍼 준비시간)
		}
		else
		{
			Sleep(4000);																		// 4초동안 딜레이 (서버에서의 버퍼 준비시간)
		}
		m_Socket.Send((LPVOID)(LPCTSTR)fileData, (UINT)fileSize);								// 서버로 파일 전송

		delete[] fileData;

		m_bInspection = false;	// 테스트 호진
	}
}

void CGLIMClientDlg::Send_Streaming(Mat &mat_temp)
{
	imwrite("C:/Users/LMS23/Desktop/GLIM_Client_Temp/Temp_Streaming_image.jpg", mat_temp);

	CString filePath = _T("C:/Users/LMS23/Desktop/GLIM_Client_Temp/Temp_Streaming_image.jpg");	// 파일 경로
	CString fileName = _T("Temp_Streaming_image.jpg");											// 파일 이름

	CFile file;
	if (file.Open(filePath, CFile::modeRead))		// 읽기전용
	{
		ULONGLONG fileSize = file.GetLength();		// 파일크기
		char* fileData = new char[fileSize];
		file.Read(fileData, (UINT)fileSize);
		file.Close();

		CString strMessageToSend;
		strMessageToSend.Format(_T("Type:File,%I64u,%s"), fileSize, fileName);					// 파일 크기와 파일 이름을 구분자 ,로 구분하여 문자열로 만들기
		m_Socket.Send((LPVOID)(LPCTSTR)strMessageToSend, strMessageToSend.GetLength() * 2);		// 서버로 파일 크기와 파일 이름을 포함한 문자열을 전송
		Sleep(1000);																			// 1초동안 딜레이 (서버에서의 버퍼 준비시간)
		m_Socket.Send((LPVOID)(LPCTSTR)fileData, (UINT)fileSize);								// 서버로 파일 전송

		delete[] fileData;

		m_bInspection = false;	// 테스트 호진
	}
}

void CGLIMClientDlg::SendImage(CSocket& socket, const Mat& image)
{
	CFileDialog fileDialog(TRUE);						// TRUE: 열기 다이얼로그, FALSE: 저장 다이얼로그
	if (fileDialog.DoModal() == IDOK)
	{
		CString filePath = fileDialog.GetPathName();	// 파일 경로 가져오기
		CString fileName = fileDialog.GetFileName();	// 파일 이름 가져오기
		CFile file;
		if (file.Open(filePath, CFile::modeRead))		// 읽기전용
		{
			ULONGLONG fileSize = file.GetLength();		// 파일크기
			char* fileData = new char[fileSize];
			file.Read(fileData, (UINT)fileSize);
			file.Close();

			CString strMessageToSend;
			strMessageToSend.Format(_T("Type:File,%I64u,%s"), fileSize, fileName);					// 파일 크기와 파일 이름을 구분자 ,로 구분하여 문자열로 만들기
			m_Socket.Send((LPVOID)(LPCTSTR)strMessageToSend, strMessageToSend.GetLength() * 2);		// 서버로 파일 크기와 파일 이름을 포함한 문자열을 전송
			if (fileSize <= 100000)
			{
				Sleep(1000);																		// 1초동안 딜레이 (서버에서의 버퍼 준비시간)
			}
			else if (fileSize <= 1000000)
			{
				Sleep(3000);																		// 3초동안 딜레이 (서버에서의 버퍼 준비시간)
			}
			else
			{
				Sleep(4000);																		// 4초동안 딜레이 (서버에서의 버퍼 준비시간)
			}
			m_Socket.Send((LPVOID)(LPCTSTR)fileData, (UINT)fileSize);								// 서버로 파일 전송

			delete[] fileData;

			m_bInspection = false;	// 테스트 호진
		}
	}
}

void CGLIMClientDlg::rec_Arduino()
{
	char buffer[256]; // 읽은 데이터를 저장할 버퍼
	while (true)
	{
		UINT bytesRead = m_pArduino->ReadData(buffer, sizeof(buffer));
		if (bytesRead > 0)
		{
			CString str(buffer);
			TRACE(str);
			if (str == "C")
			{
				m_pArduino->WriteData("0", 1);

				//스레드 시작할꺼임
				if (m_thread.joinable())
				{
					m_List.AddString(_T("LOG : 이미 웹캠 캡쳐가 진행중입니다"));
				}
				else
				{
					m_bThreadRunning = true;
					m_thread = std::thread(&CGLIMClientDlg::ImageSendThread, this);
					m_List.AddString(_T("LOG : 웹캠 캡쳐 시작됨"));
				}
			}
		}
	}
}

void CGLIMClientDlg::Send_Result_Arduino(const char * num)
{
	m_pArduino->WriteData(num, 1);
}