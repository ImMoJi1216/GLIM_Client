
// GLIM_ClientDlg.h: 헤더 파일
//

#pragma once
#include "CConnectSocket.h"
#include "opencv2/opencv.hpp"
#include "Arduino.h"

#include <iostream>
#include <fstream> // F스트림은 파일스트림임.
#include <sstream>
#include <vector>
#include <future>
#include <thread>

// CV:: 생략을 위한거 
using namespace cv;

// CGLIMClientDlg 대화 상자
class CGLIMClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CGLIMClientDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	
	// 소켓
	CConnectSocket m_Socket;

	// 카메라에서 캡쳐해오는 포인터
	VideoCapture* capture;

	// Mat클래스 변수 선언 (배열형태)
	Mat mat_frame;

	// CBitmap 클래스의 확장개념, 사용법은 크게 다르지 않음.
	CImage* cimage_mfc;

	// 아두이노
	Arduino* m_pArduino;

	// 서버에게 전송할 데이터 간격 설정
	const int SEND_INTERVAL = 0;

	//스레드 할꺼임
	std::atomic<bool> m_bThreadRunning; // 스레드 실행 여부
	std::thread m_thread;				// 이미지 전송을 처리할 스레드
	
	std::thread m_Arduino_thread;		// 이미지 전송을 처리할 스레드
	
	void rec_Arduino();
	void ImageSendThread();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GLIM_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();

	// 내가 추가함
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 변수
	CListBox m_List;												// Listbox의 맴버변수
	CStatic m_picture;												// Picture 컨트롤의 맴버변수
	CEdit m_Textedit;												// Text 컨트롤의 맴버변수
	std::atomic<bool> m_bInspection;								// 검사 : 시작/중지

	// 내가 생성한 함수
	void SendImage(CSocket& socket, const Mat& image);				// 이미지 전송
	void Saveing_IMG(Mat& mat_temp);								// 이미지 저장후 서버에 전송
	void Send_Streaming(Mat& mat_temp);								// 서버에 실시간 전송
	void Arduino_control();											// 아두이노 컨트롤 시작
	void Send_Result_Arduino(const char * num);								// 아두이노에 검사 결과 전송

	// MFC에서 생성한 함수
	afx_msg void OnDestroy();										// 나중에 더찾아봐야할듯;
	afx_msg void OnClose();											// 프로그램 종료시
	
	afx_msg void Btn_StartThread();									// Btn_쓰레드 시작
	afx_msg void Btn_EndThread();									// Btn_쓰레드 종료
	afx_msg void Btn_Inspection();									// Btn_검사 시작
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
};
