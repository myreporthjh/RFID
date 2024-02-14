
// RFIDDlg.cpp: 구현 파일
//
#include "is_d2xx.h"
#include "pch.h"
#include "framework.h"
#include "RFID.h"
#include "RFIDDlg.h"
#include "afxdialogex.h"

// sound 출력용
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include "mmsystem.h"
#pragma comment(lib,"winmm.lib") // 라이브러리 불러오기

#include <iostream>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

  
//콘솔출력
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")


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


// CRFIDDlg 대화 상자



CRFIDDlg::CRFIDDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RFID_DIALOG, pParent)
	, m_strRfid(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CRFIDDlg::~CRFIDDlg()
{
	OnDisconnect();
}

void CRFIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT3, info_str);
	DDX_Text(pDX, IDC_EDIT2, log_str);
	DDX_Control(pDX, IDC_PIC, m_picture_control);
}

BEGIN_MESSAGE_MAP(CRFIDDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CRFIDDlg::OnConnect)
	ON_BN_CLICKED(IDC_BUTTON3, &CRFIDDlg::OnReadOnce)
	ON_BN_CLICKED(IDC_BUTTON4, &CRFIDDlg::OnReadContinue)
	ON_BN_CLICKED(IDC_BUTTON2, &CRFIDDlg::OnDisconnect)
	ON_BN_CLICKED(IDC_BUTTON5, &CRFIDDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CRFIDDlg::OnBnClickedButton6)
	ON_MESSAGE(WM_USER, &CRFIDDlg::UpdateFlag)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

UINT ThreadForFlag(LPVOID param)
{
	CRFIDDlg* pMain = (CRFIDDlg*)param;

	while (pMain->m_isWorkingThread)
	{
		Sleep(100);
		PostMessage(pMain->m_hWnd, WM_USER, NULL, NULL); // UpdateFlag 호출
	}
	return 0;
}

LRESULT CRFIDDlg::UpdateFlag(WPARAM wParam, LPARAM lParam)
{
	if (flag_r)
	{
		CString temp, temp1 = _T("");
		std::string query;
		std::string result_str = "";

		check_count++;

		if (card_check_flag == TRUE) // 카드 flag ON(인식하고 난뒤 상태)일때
		{
			if (check_count > 10) // 카드한번찍으면 에러방지를 위해 1초 딜레이
			{
				check_count = 0;
				card_check_flag = FALSE; // FLAG 초기화
			}
		}
		else // 카드 flag OFF 상태일때만 인식가능하도록 설정
		{
			if (is_WriteReadCommand(ftHandle, CM1_ISO15693, CM2_ISO15693_ACTIVE + BUZZER_ON,
				writeLength, wirteData, &readLength, readData) == IS_OK)
			{
				printf("카드인식 완료1 : %d\n ", check_count);
				check_count = 0;
				card_check_flag = TRUE; // 카드 리딩 flag ON

				int i;
				printf("ISO 15693 UID : ");
				for (i = 0; i < readLength; i++)
				{
					printf("%02x ", readData[i]);
					temp.Format(_T("%02x "), readData[i]);
					temp1 += temp;
				}
				m_strRfid = temp1;
				printf("\n");

				if (db_connect_flag)
				{
					std::string card_id = CT2CA(temp1);
					query = "INSERT into card_check_log (card_id) VALUES(\"";
					query += card_id;
					query += "\") ";

					//SEND QUERY
					if (mysql_query(conn, query.c_str()))
					{
						printf("DB Fail!\n");
					}

					query = "SELECT * FROM card_check_log ORDER BY log_num DESC";
					//SEND QUERY
					if (mysql_query(conn, query.c_str()))
					{
						printf("DB Fail!\n");
					}

					result = mysql_store_result(conn);
					int fields = mysql_num_fields(result);
					while (row = mysql_fetch_row(result))
					{
						for (int i = 0; i < fields; i++)
						{
							result_str += row[i];
							result_str += "\t";
						}
						result_str += "\r\n";
					}
					log_str = result_str.c_str();

				}
				UpdateData(FALSE);
			}
		}
	}
	return 0;
}


// CRFIDDlg 메시지 처리기

BOOL CRFIDDlg::OnInitDialog()
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

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CRFIDDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRFIDDlg::OnPaint()
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
HCURSOR CRFIDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRFIDDlg::OnConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
		//열린 포트번호 찾기
	if (is_GetDeviceNumber(&usbnumber) == IS_OK)
	{
		printf("FTDI USB To Serial 연결된 개수 : %d\n", usbnumber);
		if (is_GetSerialNumber(0, readSerialNumber) == IS_OK)
		{
			if (memcmp(changeSerialNumber, readSerialNumber, sizeof(changeSerialNumber) != 0))
			{
				if (is_SetSerialNumber(0, changeSerialNumber) == IS_OK) 
				{
					printf(" USB To Serial Number 를 변경 하였습니다.\n");
					printf(" FTDI SerialNumber :  %s \n", changeSerialNumber);
				}
			}
			else
				printf(" FTDI SerialNumber :  %s \n", changeSerialNumber);
		}
	}

	//열린 포트번호로 연결
	unsigned long portNumber;
	if (is_GetCOMPort_NoConnect(0, &portNumber) == IS_OK)
	{
		printf("COM Port : %d\n", portNumber);
	}
	if (is_OpenSerialNumber(&ftHandle, readSerialNumber, 115200) != IS_OK)
	{
		printf("USB To Serial과 통신 연결 실패\n");
		//return -1;
	}
	else {
		printf("Serial포트 %d와 통신 연결성공!! \n", portNumber);

		on_off_flag = TRUE; // set ON

		// -----DB-----
		conn = mysql_init(NULL);
		mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_sec);
		conn_result = mysql_real_connect(conn, "127.0.0.1", "root", "1234", "mysql", 3306, NULL, 0);

		if (NULL == conn_result)
		{
			printf("DB Connection Fail\n");
		}
		else
		{
			db_connect_flag = TRUE; // DB connect flag ON
			printf("DB Connection Success\n");
		}
		
	}
	flag_r = 0;
	Sleep(100);
}


void CRFIDDlg::OnReadOnce()
{
	if (on_off_flag)
	{
		CString temp, temp1 = _T("");
		std::string query;
		std::string result_str="";

		// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
		// ISO15693모드로 읽기( 싱글모드 읽기 )
		if (flag_r == 0) // 계속읽기 상태아니면
		{
			if (is_WriteReadCommand(ftHandle, CM1_ISO15693, CM2_ISO15693_ACTIVE + BUZZER_ON,
				writeLength, wirteData, &readLength, readData) == IS_OK)
			{
				int i;
				printf("ISO 15693 UID : ");
				for (i = 0; i < readLength; i++)
				{
					printf("%02x ", readData[i]);
					temp.Format(_T("%02x "), readData[i]);
					temp1 += temp;
				}
				m_strRfid = temp1;
				printf("\n");

				if (db_connect_flag)
				{
					std::string card_id = CT2CA(temp1);
					query = "INSERT into card_check_log (card_id) VALUES(\"";
					query += card_id;
					query += "\") ";

					//SEND QUERY
					if (mysql_query(conn, query.c_str()))
					{
						printf("DB Fail!\n");
					}

					query = "SELECT * FROM card_check_log ORDER BY log_num DESC";
					//SEND QUERY
					if (mysql_query(conn, query.c_str()))
					{
						printf("DB Fail!\n");
					}

					result = mysql_store_result(conn);
					int fields = mysql_num_fields(result);
					while (row = mysql_fetch_row(result))
					{
						for (int i = 0; i < fields; i++)
						{
							result_str += row[i];
							result_str += "\t";
						}
						result_str += "\r\n";
					}
					log_str = result_str.c_str();

				}
				UpdateData(FALSE);

			}
		}
		else
		{
			// read continue mode
			printf("계속읽기 상태 입니다!\n");
		}
	}
	else
	{
		printf("동작 버튼을 눌러주세요.\n");
	}
}


// 계속읽기
void CRFIDDlg::OnReadContinue()
{
	
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (on_off_flag)
	{
		if (flag_r == 1)
		{
			printf("-  계속읽기 종료  -\n");
			flag_r = 0;
			m_isWorkingThread = false;
			WaitForSingleObject(m_pThread->m_hThread, 5000);
		}
		else
		{
			printf("-  계속읽기 시작 set flag_r:1  -\n");
			flag_r = 1;

			m_isWorkingThread = true;
			m_pThread = AfxBeginThread(ThreadForFlag, this);
		}
	}
	else
	{
		printf("동작 버튼을 눌러주세요.\n");
	}
}


void CRFIDDlg::OnDisconnect()
{
	if (on_off_flag)
	{
		// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
		on_off_flag = FALSE; // set OFF

		// 스레드 동작 중지
		if (flag_r)
		{
			flag_r = 0;
			m_isWorkingThread = false;
			WaitForSingleObject(m_pThread->m_hThread, 5000);
		}

		// MYSQL 연결해제
		if (db_connect_flag)
		{
			mysql_free_result(result);
			mysql_close(conn);
			printf("DB 연결해제\n");
		}

		// 무선파워를 끊어요.
		is_RfOff(ftHandle);

		//USB 포트를 Close
		if (is_Close(ftHandle) == IS_OK)
		{
			printf("연결을 닫습니다. \n");
		}
	}
	else
	{
		printf("이미 중지 상태입니다.\n");
	}

}


void CRFIDDlg::OnBnClickedButton5()
{
	/////////////////////////////////  이미지 출력  /////////////////////////////////
	CString temp;
	GetDlgItemText(IDC_EDIT1, temp);
	std::string card_id = CT2CA(temp);

	std::string query;
	std::string result_str = "";

	if (on_off_flag) // ON
	{
		if (db_connect_flag)
		{
			query = "SELECT user_name, user_age, user_gender FROM card_info WHERE card_id=\"";
			query += card_id;
			query += "\"";

			//SEND QUERY
			if (mysql_query(conn, query.c_str()))
			{
				printf("DB Fail!\n");
			}

			result = mysql_store_result(conn);
			int fields = mysql_num_fields(result);
			while (row = mysql_fetch_row(result))
			{
				for (int i = 0; i < fields; i++)
				{
					result_str += row[i];
					result_str += " / ";
				}
			}
			info_str = result_str.c_str();
			
			// image 파일 얻기 /////////////////////////////////////////////
			result_str = "img\\";
			query = "SELECT user_image FROM card_info WHERE card_id = \"";
			query += card_id;
			query += "\"";

			if (mysql_query(conn, query.c_str()))
			{
				printf("DB Fail!\n");
			}

			result = mysql_store_result(conn);
			fields = mysql_num_fields(result);
			
			while (row = mysql_fetch_row(result))
			{
				for (int i = 0; i < fields; i++)
				{
					result_str += row[i];
					std::cout << result_str<<", ";
				}
			}

			CRect rect;//픽쳐 컨트롤의 크기를 저장할 CRect 객체
			m_picture_control.GetWindowRect(rect);//GetWindowRect를 사용해서 픽쳐 컨트롤의 크기를 받는다.
			CDC* dc; //픽쳐 컨트롤의 DC를 가져올  CDC 포인터
			dc = m_picture_control.GetDC(); //픽쳐 컨트롤의 DC를 얻는다.

			std::remove(result_str.begin(), result_str.end(), ' '); // 공백 제거
			if (result_str=="img\\")
			{
				std::cout << "이미지없음\n";
				result_str += "empty.bmp";
			}
			CString temp_str; // String -> CString -> LPCTSTR 변환을 위한 동작
			temp_str = result_str.c_str();
			CImage image;
			image.Load(temp_str); //이미지 로드

			image.StretchBlt(dc->m_hDC, 0, 0, rect.Width(), rect.Height(), SRCCOPY); //이미지를 픽쳐 컨트롤 크기로 조정
			ReleaseDC(dc); //DC 해제
		}
		UpdateData(FALSE);
		
	}
	else
	{
		printf("조회불가 - 동작 상태일때만 가능합니다.\n");
	}
}


void CRFIDDlg::OnBnClickedButton6()
{
	//sndPlaySound("sound.wav", SND_ASYNC | SND_NODEFAULT);
	PlaySoundW(_T("sound.wav"), NULL, SND_FILENAME | SND_ASYNC);
	//_getch();
}
