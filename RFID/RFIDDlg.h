
// RFIDDlg.h: 헤더 파일


#pragma once
#include "is_d2xx.h"

// DB 연동
#include <mysql.h>

// string
#include <string>

// algorithm
#include <algorithm>

// CRFIDDlg 대화 상자
class CRFIDDlg : public CDialogEx
{
// 생성입니다.
public:
	CRFIDDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	~CRFIDDlg();
	IS_HANDLE ftHandle = 0;
	char readSerialNumber[100] = "COM07";
	char changeSerialNumber[100] = "RFID01";
	short usbnumber;
	unsigned char wirteData[1024];
	unsigned short writeLength = 0;
	unsigned char readData[1024];
	unsigned short readLength = 0;
	BOOL flag_r = 0;
	BOOL card_check_flag = FALSE; // 카드 체크 flag
	int check_count = 0; // 카드 체크 카운트
	BOOL on_off_flag = FALSE;

	// 스레드 사용 부분
	CWinThread* m_pThread;
	bool m_isWorkingThread;
	LRESULT UpdateFlag(WPARAM wParam, LPARAM lParam);
	
	// DB 사용 부분
	MYSQL* conn;
	MYSQL* conn_result;	
	MYSQL_RES* result;  // 응답: 결과
	MYSQL_ROW row;		// 응답: 결과행
	//char query[1024];	// 쿼리 실행문
	unsigned int timeout_sec = 1;
	BOOL db_connect_flag = FALSE; // DB연결 flag

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RFID_DIALOG };
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
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnConnect();
	afx_msg void OnDisconnect();
	afx_msg void OnReadOnce();
	afx_msg void OnReadContinue();
	CString m_strRfid;
	CString log_str;
	CString info_str;

	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	CStatic m_picture_control;
};
