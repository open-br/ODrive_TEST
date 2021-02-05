
// ROBOT_CTLDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "ROBOT_CTL.h"
#include "ROBOT_CTLDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include "futaba_rs.h"
#include "ODriveArduino.h"

#include <time.h>     // for clock()
#include <mmsystem.h>	// timeGetTime()
#pragma comment( lib, "winmm.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//iniファイルから取得するシリアル設定
#define SETFILE _T( "./setting.ini")

// スレッドはクラス外から呼び出すため、そこで使用する変数は、グローバル変数として扱う方針

futaba_rs Futaba_RS;	// サーボ制御用クラス
ODriveArduino ODrive;

float pos_m0 = 0.0f;

char get_port[10];
int get_baud;

int sv_count = 1;		// サーボの個数 (あとでsettingからオーバーライドする)
int sv_ctl_f = 0;		// サーボコントロールフラグ

sv_r Data[100];			// とりあえずID:0からID:99まで確保(サーボのデータ)

clock_t Processing_time, Processing_time1, Processing_time2, Processing_time3;	// 処理時間の入れ物


CString filetime;		// CSVを保存するときのファイル名
int flag_csv = 0;		// CSVフラグ
FILE* csv_fp;			// CSVのファイルポインタ

int sv_angle_rw_all = 0;

int pointA;
int pointB;
int timeAB;
int timeBA;
int time_A;
int time_B;

int f_testmove = 0;
int f_testmove_do = 0;
int f_testmove2 = 0;

int sv_diff[100];

BOOL sv_torque_w[100];
BOOL sv_reverse_w[100];

// 関数
void RSGetDataALL(void);	// 全サーボ受信
void thread_motor(void);	// サーボ制御用スレッド
void thread_testmove(void);	    // テスト動作用スレッド
void thread_testmove2(void);	    // テスト動作用スレッド

// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
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


// CROBOT_CTLDlg ダイアログ


IMPLEMENT_DYNAMIC(CROBOT_CTLDlg, CDialogEx);

CROBOT_CTLDlg::CROBOT_CTLDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ROBOT_CTL_DIALOG, pParent)
	, com_number(_T(""))
	, com_baudrate(_T(""))
	, com_openclode(_T(""))

	, temp0001(_T(""))
	, flip_radio(0)
	, csvlist(_T(""))
	, current_filename(_T(""))
	, select_move(_T(""))
	, motion_file_select(_T(""))
	, motion_name(_T(""))

	, Current_lim(_T("0"))
	, Vel_limit(_T("0"))

{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
}

CROBOT_CTLDlg::~CROBOT_CTLDlg()
{
	// このダイアログ用のオートメーション プロキシがある場合は、このダイアログ
	//  へのポインターを NULL に戻します、それによってダイアログが削除されたこと
	//  がわかります。
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;
}

void CROBOT_CTLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, com_number);
	DDX_Text(pDX, IDC_EDIT3, com_baudrate);
	DDX_Text(pDX, IDC_EDIT5, com_openclode);

	DDX_Text(pDX, IDC_ANGLE_61, sv_angle_r[1]);
	DDX_Text(pDX, IDC_SPEED_61, sv_speed_r[1]);
	DDX_Text(pDX, IDC_LOAD_61, sv_load_r[1]);


	DDX_Text(pDX, IDC_ANGLE_62, sv_angle_r[2]);
	DDX_Text(pDX, IDC_SPEED_62, sv_speed_r[2]);
	DDX_Text(pDX, IDC_LOAD_62, sv_load_r[2]);



	DDX_Slider(pDX, IDC_ANGLE_SL61, sv_angle_rw[1]);
	DDX_Slider(pDX, IDC_ANGLE_SL62, sv_angle_rw[2]);


	DDX_Slider(pDX, IDC_ANGLE_SALL, sv_angle_rw_all);



	DDX_Text(pDX, IDC_ANGLE_VALL, sv_angle_rw_v_all);

	DDX_Control(pDX, IDC_ANGLE_SL61, sv_angle_sl_ctl[1]);
	DDX_Control(pDX, IDC_ANGLE_SL62, sv_angle_sl_ctl[2]);


	DDX_Control(pDX, IDC_ANGLE_SALL, sv_angle_sl_ctl_all);


	DDX_Control(pDX, IDC_SV_OPEN, sv_opne_ctlf);
	DDX_Control(pDX, IDC_SV_CLOSE, sv_close_ctlf);
	DDX_Text(pDX, IDC_TEMP0001, temp0001);
	DDX_Control(pDX, IDC_BUTTON_CSV, sv_csv_cap);

	DDX_Control(pDX, IDC_PLAY, test_play);
	DDX_Control(pDX, IDC_PLAY2, test_play2);
	DDX_Control(pDX, IDC_STOP, test_stop);

	
	//DDX_Text(pDX, IDC_sv_cnt, sv_count_s);

	DDX_Text(pDX, IDC_pointA, pointA);
	DDX_Text(pDX, IDC_pointB, pointB);
	DDX_Text(pDX, IDC_time_A, time_A);
	DDX_Text(pDX, IDC_time_B, time_B);
	DDX_Text(pDX, IDC_timeAB, timeAB);
	DDX_Text(pDX, IDC_timeBA, timeBA);


	// ODrve コンフィグ
	DDX_Text(pDX, IDC_vel_lim, Vel_limit);
	DDX_Text(pDX, IDC_current_lim, Current_lim);

}

BEGIN_MESSAGE_MAP(CROBOT_CTLDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()

	ON_BN_CLICKED(IDOK, &CROBOT_CTLDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CROBOT_CTLDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CROBOT_CTLDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_SV_OPEN, &CROBOT_CTLDlg::OnBnClickedSvOpen)
	ON_BN_CLICKED(IDC_SV_CLOSE, &CROBOT_CTLDlg::OnBnClickedSvClose)

	ON_BN_CLICKED(IDC_BUTTON_CSV, &CROBOT_CTLDlg::OnBnClickedButtonCsv)
	ON_BN_CLICKED(IDC_BUTTON_ZERO, &CROBOT_CTLDlg::OnBnClickedButtonZero)
	ON_BN_CLICKED(IDC_BUTTON_DIFF, &CROBOT_CTLDlg::OnBnClickedButtonDiff)
	ON_BN_CLICKED(IDC_PLAY, &CROBOT_CTLDlg::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP, &CROBOT_CTLDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_PLAY2, &CROBOT_CTLDlg::OnBnClickedPlay2)
	ON_BN_CLICKED(IDC_BUTTON4, &CROBOT_CTLDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_READ_CONF, &CROBOT_CTLDlg::OnBnClickedReadConf)
	ON_BN_CLICKED(IDC_WRITE_CONF, &CROBOT_CTLDlg::OnBnClickedWriteConf)
	ON_BN_CLICKED(IDC_SAVE_ODRIVE, &CROBOT_CTLDlg::OnBnClickedSaveOdrive)
	ON_BN_CLICKED(IDC_BUTTON_aaa, &CROBOT_CTLDlg::OnBnClickedButtonaaa)
END_MESSAGE_MAP()


// CROBOT_CTLDlg メッセージ ハンドラー

BOOL CROBOT_CTLDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。 ☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆

	int i;

	UpdateData(TRUE);	// GUI→値


	//全サーボ動作速度を0msecに設定
	for (i = 0; i < 100; i++) {
		Data[i].g_time = 0;
	}
	
	// スライダの設定用の一時変数最大・最少・デフォルト／名前
	int max, min;
	char part_name[10];


	for (i = 1; i < 3; i++) {
		//////////////////スライダーの範囲を設定する

		sprintf_s(part_name, "SURVO_%d", i);

		min = GetPrivateProfileInt("SURVO_MIN", part_name, -1800, SETFILE);
		max = GetPrivateProfileInt("SURVO_MAX", part_name, 1800, SETFILE);
		

		//スライダー設定
		sv_angle_sl_ctl[i].SetRange(min, max);		// 最大・最少
		sv_angle_rw[i] = 50;						// デフォルト値
		if (i != 1) sv_reverse_w[i] = GetPrivateProfileInt("SURVO_REVERSE", part_name, 0, SETFILE);

		//////////////////スライダーの範囲を設定する
	}

	sv_angle_sl_ctl_all.SetRange(-1800,1800);
	sv_angle_rw_all = 100;
	UpdateData(FALSE);	// 値→GUI
	sv_angle_rw_all = 0;
	UpdateData(FALSE);	// 値→GUI

	// INIファイルから取得
	GetPrivateProfileString("serial", "COM", "INI ERROR", get_port, sizeof(get_port), SETFILE);
	get_baud = GetPrivateProfileInt("serial", "BAUD", 0, SETFILE);

	com_number = get_port;
	com_baudrate.Format(_T("%d bps"), get_baud);
	com_openclode = "CLOSE";

	sv_count = GetPrivateProfileInt("SURVO", "COUNT", 0, SETFILE);
	sv_count_s.Format(_T("%d 個"), sv_count);


	pointA = 200;
	pointB = -200;
	time_A = 500;
	time_B = 500;
	timeAB = 500;
	timeBA = 500;


	
	UpdateData(FALSE);	// 値→GUI

	

	//表示更新用のタイマーを16msで起動 (タイマーを起動しなければ、設定モードに入れる)
	SetTimer(5678, 16, NULL);

	

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CROBOT_CTLDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CROBOT_CTLDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);

	}
	else
	{

		CPaintDC paintDC(this);

		// 自分で描画する
		if (m_bmp.GetSafeHandle() == NULL)
		{
			// 適当にビットマップを作成
			// ビットマップをロード
			m_bmp.LoadBitmap(IDB_BITMAP1);
		}
		// ビットマップの大きさを取得
		BITMAP bitmap;
		m_bmp.GetBitmap(&bitmap);
		CSize sz(bitmap.bmWidth, bitmap.bmHeight);

		CDC dc;
		dc.CreateCompatibleDC(&paintDC);
		// ディバイスコンテキストで選択
		CBitmap* pOld = dc.SelectObject(&m_bmp);
		// もともとのディバイスコンテキストに
		// ビットマップを透過で転送
		if (::TransparentBlt(paintDC, 690, 15, sz.cx, sz.cy,
			dc, 0, 0, sz.cx, sz.cy, (UINT)RGB(255, 0, 255))
			== FALSE)
		{
			// エラー番号を取得
			DWORD dwErr = ::GetLastError();
			// ここにエラー時の処理を記述すること
		}

		// ビットマップ選択解除
		dc.SelectObject(pOld);


		//初期値入れ
		UpdateData(FALSE);


		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CROBOT_CTLDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// コントローラーがオブジェクトの 1 つをまだ保持している場合、
//  オートメーションサーバーはユーザーが UI を閉じる際に終了で
//  きません。これらのメッセージ ハンドラーはプロキシがまだ使用中
//  かどうかを確認し、それから UI が非表示になりますがダイアロ
//  グはそれが消された場合その場所に残ります。

void CROBOT_CTLDlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}


BOOL CROBOT_CTLDlg::CanExit()
{
	// プロキシ オブジェクトがまだ残っている場合、オートメーション
	//  コントローラーはこのアプリケーションをまだ保持しています。
	//  ダイアログの周囲は残しますが UI は非表示になります。
	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}



void CROBOT_CTLDlg::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CDialogEx::OnOK();
}


void CROBOT_CTLDlg::OnBnClickedButton1()	// 全身トルクオン！
{
	
	// TODO: ここにコントロール通知ハンドラー コードを追加します。




	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};


	ODrive.reset();

	ODrive.close();	// シリアルポートクローズ


}

void CROBOT_CTLDlg::OnBnClickedButton2()	// 全身脱力！
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	int i;

	for (i = 1; i < 9; i++) {
		sv_torque_w[i] = 0;
	}

	UpdateData(FALSE);
}


void CROBOT_CTLDlg::OnBnClickedSvOpen()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	com_openclode = "OPEN";
	UpdateData(FALSE);
	
	//	sv_open();
	sv_ctl_f = 1;	//スレッドコントロール用のフラグ (スレッドを立ち上げる前に1にして、これを0にするとスレッドの無限ループが終わる)
	
	// とりあえず全サーボトルクオフ
	OnBnClickedButton2();	// 脱力ボタンを呼び出し

	////////////スレッド用パラメータ
	HANDLE handle;

	////////////スレッドの生成
	handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_motor, NULL, 0, NULL);

	sv_opne_ctlf.EnableWindow(FALSE);
	sv_close_ctlf.EnableWindow(TRUE);

	// 0.1秒待って差分を取得
	Sleep(100);
	OnBnClickedButtonDiff();

	sv_angle_rw_all = Data[1].angle;
	UpdateData(FALSE);

}



void CROBOT_CTLDlg::OnBnClickedSvClose()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	sv_ctl_f = 0;
	//Sleep(1000);

	com_openclode = "CLOSE";
	UpdateData(FALSE);

	sv_opne_ctlf.EnableWindow(TRUE);
	sv_close_ctlf.EnableWindow(FALSE);

}



//----------------------------サーボモーターの関連関数 -------------------------


void RSGetDataALL(void)
{
	int i;
	sv_r rData[100];		// とりあえずID:0からID:99まで確保(サーボのデータ)

	for (i = 1; i < sv_count+1; i++) {		// ここの数値を変数に！☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
		
		rData[i] = Futaba_RS.sv_read2(i);
		
		Data[i].angle		= rData[i].angle;
		Data[i].load		= rData[i].load;
		Data[i].speed		= rData[i].speed;
		Data[i].temperature = rData[i].temperature;
		Data[i].time		= rData[i].time;
		Data[i].error		= rData[i].error;
	}

}



//モーター制御用のスレッド
void thread_motor(void)
{

	int i;
	SYSTEMTIME st;
	clock_t now_time = 0,old_time = 0;	// 時間計測用


	//if (Futaba_RS.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
	//											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
	//	while (1);
	//};


	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
												//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};





	// とりあえず全サーボトルクオフ
	//for (i = 0; i < 100; i++) {
	//	Futaba_RS.sv_torque(i, 0);
	//}

	while (sv_ctl_f) {
		
		// 処理時間の取得
		now_time = clock();						// 現在時間の更新
		Processing_time = now_time - old_time;	// 処理時間の算出
		old_time = now_time;					// 過去時間の更新

		// サーボの値を取得
		
		// 速度取得
		Data[1].speed = ODrive.GetVelocity(0) *1000 ;	//
		Data[1].angle = ODrive.GetPosition(0) * 3.6f;
		Data[1].load = ODrive.GetCurrent(0) * 1000;
		

		Processing_time1 = clock() - now_time;




		Processing_time2 = clock() - Processing_time1 - now_time;




		pos_m0 = (float)sv_angle_rw_all/36.0f;
		ODrive.SetPosition(0, pos_m0);









		Processing_time3 = clock() - Processing_time2 - now_time - Processing_time1;




		if (flag_csv) {	// CSVに記録

			// 時間の取得
			GetSystemTime(&st);
			// wHourを９時間足して、日本時間にする
			fprintf_s(csv_fp, "\"%04d/%02d/%02d_%02d:%02d:%02d.%03d\",",
				st.wYear, st.wMonth, st.wDay,
				st.wHour + 9, st.wMinute, st.wSecond, st.wMilliseconds);

			for (i = 1; i < 3; i++) {
				//トルクオフなら取得値　オンなら指示角 g_angle	
				fprintf_s(csv_fp, "%d,", Data[i].g_angle); // 目標角度
				fprintf_s(csv_fp, "%d,", Data[i].angle);   // 実測角度
				fprintf_s(csv_fp, "%d,", Data[i].speed);   // 実測角速度
				fprintf_s(csv_fp, "%d,", Data[i].load);    // 実測電流
				fprintf_s(csv_fp, "%d,", Data[i].temperature); // 実測温度
				fprintf_s(csv_fp, "0x%04x,", Data[i].error);   // エラー
			}

			fprintf_s(csv_fp, "\n");

		}




	}

	//Futaba_RS.close();	// シリアルポートクローズ


	ODrive.close();	// シリアルポートクローズ
}


void CROBOT_CTLDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。


	if (nIDEvent == 5678) {

		int i;


		if (f_testmove_do == 0) UpdateData(TRUE);		// スライダの位置・ボックスの値を変数に代入 (再生中でなければ)

		//for (i = 1; i < sv_count + 1; i++) {
		//	sv_angle_r[i].Format(_T("%.1f deg"), ((float)Data[i].angle / 10));	//受信した値[角度]を格納
		//	sv_time_r[i].Format(_T("%.2f sec"), ((float)Data[i].time / 100));	//受信した値[時間]を格納
		//	sv_speed_r[i].Format(_T("%d d/s"), Data[i].speed);					//受信した値[速度]を格納
		//	sv_load_r[i].Format(_T("%d mA"), Data[i].load);						//受信した値[負荷]を格納
		//	sv_temperature_r[i].Format(_T("%d℃"), Data[i].temperature);		//受信した値[温度]を格納

		//	sv_error_r[i].Format(_T("%o"), Data[i].error);						// エラーを格納
		//	if (Data[i].error != 0) {
		//		sv_angle_r[i].Format(_T("XXXXX"));			// エラーのときは[XXXXX]を表示
		//		sv_time_r[i].Format(_T("XXXXX"));			// エラーのときは[XXXXX]を表示
		//		sv_speed_r[i].Format(_T("XXXXX"));			// エラーのときは[XXXXX]を表示
		//		sv_load_r[i].Format(_T("XXXXX"));			// エラーのときは[XXXXX]を表示
		//		sv_temperature_r[i].Format(_T("XXXXX"));	// エラーのときは[XXXXX]を表示
		//	}

		//	sv_diff_w[i].Format(_T("%d"), sv_diff[i]);							// 差分を格納


		//	Data[i].torque = sv_torque_w[i];									// トルクチェックボックスをトルクの値に反映

		//	if (Data[i].torque == 0) {			// トルクがオフの場合
		//		sv_angle_rw[i] = Data[i].angle;							// 角度→スライダー
		//		sv_angle_rw_v[i].Format(_T("%d"), Data[i].angle);		// 角度→値ボックス
		//		sv_angle_rw_all = Data[1].angle;

		//	}
		//	else {								// トルクがオンの場合

		//		//if (sv_reverse_w[i] == TRUE) {	// リバースがオンの時
		//		//	Data[i].g_angle = -(do_sv_angle_rw_all + sv_diff[i]);
		//		//}
		//		//else {							// リバースがオフの時
		//		//	Data[i].g_angle = do_sv_angle_rw_all + sv_diff[i];
		//		//}
		//		sv_angle_rw[i] = Data[i].g_angle;						// 角度 → スライダー
		//		sv_angle_rw_v[i].Format(_T("%d"), Data[i].g_angle);		// 角度 → 値ボックス
		//	}

		//}

		sv_speed_r[1].Format(_T("%.2f"), (float)Data[1].speed / 1000);					//受信した値[速度]を格納
		sv_angle_r[1].Format(_T("%.2f"), (float)Data[1].angle / 100);	//受信した値[角度]を格納
		sv_load_r[1].Format(_T("%d"), Data[1].load);						//受信した値[負荷]を格納
		sv_angle_rw[1] = Data[1].angle;
		
		sv_angle_rw_v_all.Format(_T("%d"), sv_angle_rw_all);		// 角度→値ボックス
		
	


		// 制御時間の表示
		temp0001.Format(_T("処理時間: %d msec (%d /%d /%d)"), Processing_time, Processing_time1, Processing_time2, Processing_time3);

		// ラジオボタンのいろいろ

		UpdateData(FALSE);	//変数の値をスライダの位置・ボックスの値に反映

	}

	
	CDialogEx::OnTimer(nIDEvent);
	
}





void CROBOT_CTLDlg::OnBnClickedButtonCsv()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	int i;

	//現在時間の取得
	CTime cTime = CTime::GetCurrentTime();           // 現在時刻

	//現在時間をファイル名に
	filetime = cTime.Format("FB_TEST_%Y%m%d_%H%M%S.csv");   // "YYYY/mm/dd HH:MM:SS"形式の時刻文字列を取得	// ファイル名 RTT リアルタイムティーチ


	if (flag_csv == 0){


		fopen_s(&csv_fp, filetime, "w");	// ファイルオープン

		sv_csv_cap.SetWindowTextA("● CSV出力中...");

		// タイトル出力
		fprintf_s(csv_fp, "出力時間,");
		for (i = 1; i < 3; i++) {
			//トルクオフなら取得値　オンなら指示角 g_angle	
			fprintf_s(csv_fp, "目標角度(ID:%d),", i); // 目標角度
			fprintf_s(csv_fp, "実測角度(ID:%d),", i);   // 実測角度
			fprintf_s(csv_fp, "実測角速度(ID:%d),", i);   // 実測角速度
			fprintf_s(csv_fp, "実測電流(ID:%d),", i);    // 実測電流
			fprintf_s(csv_fp, "実測温度(ID:%d),", i); // 実測温度
			fprintf_s(csv_fp, "エラー(ID:%d),", i);   // エラー
		}

		fprintf_s(csv_fp, "\n");


		flag_csv = 1;
	}
	else {
		flag_csv = 0;
		Sleep(100);
		fclose(csv_fp); // ファイルクローズ
		sv_csv_cap.SetWindowTextA("CSV出力");
	}

	UpdateData(FALSE);

}


void CROBOT_CTLDlg::OnBnClickedButtonZero()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。


	sv_angle_rw_all = 0;

	UpdateData(FALSE);	//変数の値をスライダの位置・ボックスの値に反映
}


void CROBOT_CTLDlg::OnBnClickedButtonDiff()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	int i;
	int max = 0, min = 0;

	sv_reverse_w[1] = FALSE;

	for (i = 1; i < sv_count + 1; i++) {

		sv_diff[i] = Data[i].angle - Data[1].angle;
		
		if (sv_reverse_w[i] == FALSE) {		// 正転のとき
			sv_diff[i] = Data[i].angle - Data[1].angle;
		}
		else {								// 逆転のとき
			sv_diff[i] = -Data[i].angle - Data[1].angle;
		}

		if (max < sv_diff[i]) max = sv_diff[i];
		if (min > sv_diff[i]) min = sv_diff[i];

	}

	sv_angle_sl_ctl_all.SetRange(-1800 - min, 1800 - max);

}


void thread_testmove(void) {


	// 現在位置→A→B→A→

	int i;
	int temp_point;

	f_testmove_do = 1;

	temp_point = sv_angle_rw_all;	// 現在地を保存

	timeBeginPeriod(1);// タイマーの最小精度を1msecにする

	while (f_testmove) {

		// 現在位置からA
		for (i = 0; i <= (timeBA/10); i++) {
			sv_angle_rw_all = temp_point + (int)(((double)i / (double)(timeBA / 10)) * (pointA - temp_point));			// キャストのトラブル
			Sleep(10);	// 10ミリ秒
		}

		// A地点で静止
		Sleep(time_A);

		// A地点からB
		for (i = 0; i <= (timeAB/10); i++) {
			sv_angle_rw_all = pointA + (int)(((double)i / (double)(timeAB / 10)) * (pointB - pointA));			// キャストのトラブル
			Sleep(10);	// 10ミリ秒
		}

		// A地点で静止
		Sleep(time_A);

		temp_point = pointB;	// 現在地にBを保存

	}

	timeEndPeriod(1);// タイマーの最小精度を戻す
	// B地点で静止
	f_testmove_do = 0;

}


void thread_testmove2(void) {


	// 現在位置→A→B→A→

	int i=0;
	int temp_point;

	int do_sv_angle_rw_all;

	temp_point = sv_angle_rw_all;	// 現在地を保存

	sv_ctl_f = 0;	// 本来の制御スレッドの終了
	Sleep(1000);		// 1秒待つ

	if (Futaba_RS.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};


	while (f_testmove2) {

		// 現在位置からA
		do_sv_angle_rw_all = pointA + (int)(((double)i / (double)timeAB) * (pointB - pointA));			// キャストのトラブル
		for (i = 1; i < sv_count + 1; i++) {
			if (sv_reverse_w[i] == TRUE) {	// リバースがオンの時
				Data[i].g_angle = -(do_sv_angle_rw_all + sv_diff[i]);
			}
			else {							// リバースがオフの時
				Data[i].g_angle = do_sv_angle_rw_all + sv_diff[i];
			}
			//全サーボ動作速度を[timeBA]msecに設定
			Data[i].g_time = timeBA / 10;
		}
		// 全サーボに目標角度と目標速度を送信！
		Futaba_RS.sv_move_long(Data);
		Sleep(timeBA);



		// A地点で静止
		Sleep(time_A);

		// A地点からB
		do_sv_angle_rw_all = temp_point + (int)(((double)i / (double)timeBA) * (pointA - temp_point));			// キャストのトラブル	
		for (i = 1; i < sv_count + 1; i++) {
			if (sv_reverse_w[i] == TRUE) {	// リバースがオンの時
				Data[i].g_angle = -(do_sv_angle_rw_all + sv_diff[i]);
			}
			else {							// リバースがオフの時
				Data[i].g_angle = do_sv_angle_rw_all + sv_diff[i];
			}
			//全サーボ動作速度を[timeBA]msecに設定
			Data[i].g_time = timeAB / 10;
		}
		// 全サーボに目標角度と目標速度を送信！
		Futaba_RS.sv_move_long(Data);
		Sleep(timeAB);

		// A地点で静止
		Sleep(time_A);

		temp_point = pointB;	// 現在地にBを保存

	}
	// B地点で静止

	//全サーボ動作速度を0msecに設定
	for (i = 0; i < 100; i++) {
		Data[i].g_time = 0;
	}


	Futaba_RS.close();	// シリアルポートクローズ
	Sleep(1000);

	sv_ctl_f = 1;	// 本来の制御スレッドの終了

	////////////スレッド用パラメータ
	HANDLE handle;

	////////////スレッドの生成
	handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_motor, NULL, 0, NULL);

	// とりあえず全サーボトルクオフ
	for (i = 1; i < 9; i++) {
		sv_torque_w[i] = 0;
	}

}





void CROBOT_CTLDlg::OnBnClickedPlay()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	f_testmove = 1;

	////////////スレッド用パラメータ
	HANDLE handle;

	////////////スレッドの生成
	handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_testmove, NULL, 0, NULL);



	test_play.EnableWindow(FALSE);
	test_stop.EnableWindow(TRUE);
	test_play2.EnableWindow(FALSE);

	UpdateData(FALSE);


}


void CROBOT_CTLDlg::OnBnClickedStop()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	int i;

	f_testmove = 0; 
	f_testmove2 = 0;

	test_play.EnableWindow(TRUE);
	test_play2.EnableWindow(TRUE);
	test_stop.EnableWindow(FALSE);


	UpdateData(FALSE);

}


void CROBOT_CTLDlg::OnBnClickedPlay2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	f_testmove2 = 1;

	////////////スレッド用パラメータ
	HANDLE handle;

	////////////スレッドの生成
	handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_testmove2, NULL, 0, NULL);

	test_play.EnableWindow(FALSE);
	test_stop.EnableWindow(TRUE);
	test_play2.EnableWindow(FALSE);

}


void CROBOT_CTLDlg::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。


	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
												//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};



	ODrive.ODriveINIT();


	ODrive.close();	// シリアルポートクローズ

}


void CROBOT_CTLDlg::OnBnClickedReadConf()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。





	float a=0, b=0;
	
	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};


	a = ODrive.Get_Vel_limit(0);
	Vel_limit.Format(_T("%f []"), a);
	

	b = ODrive.Get_Current_lim(0);
	Current_lim.Format(_T("%f []"), b);


	UpdateData(FALSE);	// 値→GUI


	ODrive.close();	// シリアルポートクローズ
}


void CROBOT_CTLDlg::OnBnClickedWriteConf()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
										//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};


	ODrive.Set_Vel_limit(0, 10000.0f);
	ODrive.Set_Current_lim(0, 11.0f);


	ODrive.close();	// シリアルポートクローズ


}


void CROBOT_CTLDlg::OnBnClickedSaveOdrive()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。



	//Current_lim.Format(_T("sssss"));



	//UpdateData(FALSE);	// 値→GUI


	float a = 0, b = 0;

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};


	a = ODrive.Get_Vel_limit(0);
	Vel_limit.Format(_T("%f []"), a);


	b = ODrive.Get_Current_lim(0);
	Current_lim.Format(_T("%f []"), b);


	UpdateData(FALSE);	// 値→GUI


	ODrive.close();	// シリアルポートクローズ

}


void CROBOT_CTLDlg::OnBnClickedButtonaaa()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	float a = 0, b = 0;

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};


	a = ODrive.Get_Vel_limit(0);
	b = ODrive.Get_Current_lim(0);

	Vel_limit.Format(_T("%f"), a);


	Current_lim.Format(_T("%f"), b);



	ODrive.close();	// シリアルポートクローズ

	UpdateData(FALSE);	// 値→GUI
}
