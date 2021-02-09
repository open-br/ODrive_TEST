
// ROBOT_CTLDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "ROBOT_CTL.h"
#include "ROBOT_CTLDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
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

// ODrive制御用クラス
ODriveArduino ODrive;

// ODriveのパラメータ(送信用)
float pos_m[2] = { 0.0f ,0.0f };

// ODriveのパラメータ(受信用)
float r_pos_m[2] = { 0.0f ,0.0f };
float r_vel_m[2] = { 0.0f ,0.0f };
float r_current_m[2] = { 0.0f ,0.0f };

// シリアルポート関連
char get_port[10];
int get_baud;

// サーボコントロールフラグ(制御スレッドのフラグ)
int sv_ctl_f = 0;		

// 処理時間計測用の変数
clock_t Processing_time;
clock_t Processing_time1;
clock_t Processing_time2;
clock_t Processing_time3;

// CSV関連
CString filetime;		// CSVを保存するときのファイル名
int flag_csv = 0;		// CSVフラグ
FILE* csv_fp;			// CSVのファイルポインタ

// 指示値(スライダ)
int sv_angle_rw_all = 0;
int sv_angle_rw_all1 = 0;
int goal_angle[2] = {0,0};

// チェックボックス
BOOL chk_axis[2] = {FALSE,FALSE };

// 負荷テストのパラメータ
int pointA;
int pointB;
int timeAB;
int timeBA;
int time_A;
int time_B;

// 負荷テストの動作に使うフラグ
int f_testmove = 0;			// テスト動作開始フラグ
int f_testmove_do = 0;		// テスト動作実行中フラグ

// 関数
void thread_motor(void);		// サーボ制御用スレッド
void thread_testmove(void);	    // テスト動作用スレッド

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

	// シリアルポート制御関連
	DDX_Text(pDX, IDC_EDIT2, com_number);						// COM番号
	DDX_Text(pDX, IDC_EDIT3, com_baudrate);						// ボーレート
	DDX_Text(pDX, IDC_EDIT5, com_openclode);					// ステータス
	DDX_Control(pDX, IDC_SV_OPEN, sv_opne_ctlf);				// OPEN
	DDX_Control(pDX, IDC_SV_CLOSE, sv_close_ctlf);				// CLOSE
	DDX_Text(pDX, IDC_TEMP0001, temp0001);						// 制御時間表示用

	// ODrive 受信表示 axis0
	DDX_Text(pDX, IDC_ANGLE_00, sv_angle_r[0]);
	DDX_Text(pDX, IDC_SPEED_00, sv_speed_r[0]);
	DDX_Text(pDX, IDC_LOAD_00, sv_load_r[0]);
	DDX_Slider(pDX, IDC_ANGLE_SL00, sv_angle_rw[0]);			// 値
	DDX_Control(pDX, IDC_ANGLE_SL00, sv_angle_sl_ctl[0]);		// スライダのコントロール

	// ODrive 受信表示 axis1
	DDX_Text(pDX, IDC_ANGLE_01, sv_angle_r[1]);
	DDX_Text(pDX, IDC_SPEED_01, sv_speed_r[1]);
	DDX_Text(pDX, IDC_LOAD_01, sv_load_r[1]);
	DDX_Slider(pDX, IDC_ANGLE_SL01, sv_angle_rw[1]);			// 値
	DDX_Control(pDX, IDC_ANGLE_SL01, sv_angle_sl_ctl[1]);		// スライダのコントロール

	// 位置制御 送信用 axis0
	DDX_Text(pDX, IDC_ANGLE_VALL, goal_angle_str[0]);			// 角度の値(数値表示(文字))
	DDX_Slider(pDX, IDC_ANGLE_SALL, goal_angle[0]);				// スライダの値
	DDX_Control(pDX, IDC_ANGLE_SALL, goal_angle_ctl[0]);		// スライダのコントローラー(上限・下限の設定用)

	// 位置制御 送信用 axis1
	DDX_Text(pDX, IDC_ANGLE_VALL1, goal_angle_str[1]);			// 角度の値(数値表示(文字))
	DDX_Slider(pDX, IDC_ANGLE_SALL1, goal_angle[1]);			// スライダの値
	DDX_Control(pDX, IDC_ANGLE_SALL1, goal_angle_ctl[1]);		// スライダのコントローラー(上限・下限の設定用)

	// CSV出力ボタン
	DDX_Control(pDX, IDC_BUTTON_CSV, sv_csv_cap);

	// ODrve コンフィグaxis0
	DDX_Text(pDX, IDC_vel_lim, Vel_limit[0]);
	DDX_Text(pDX, IDC_current_lim, Current_lim[0]);
	DDX_Text(pDX, IDC_pos_gain, Pos_gain[0]);
	DDX_Text(pDX, IDC_vel_gain, Vel_gain[0]);
	DDX_Text(pDX, IDC_vel_integrator_gain, Vel_integrator_gain[0]);

	// ODrve コンフィグaxis1
	DDX_Text(pDX, IDC_vel_lim1, Vel_limit[1]);
	DDX_Text(pDX, IDC_current_lim1, Current_lim[1]);
	DDX_Text(pDX, IDC_pos_gain1, Pos_gain[1]);
	DDX_Text(pDX, IDC_vel_gain1, Vel_gain[1]);
	DDX_Text(pDX, IDC_vel_integrator_gain1, Vel_integrator_gain[1]);

	// 負荷テスト開始・停止ボタンコントロール
	DDX_Control(pDX, IDC_PLAY, test_play);
	DDX_Control(pDX, IDC_STOP, test_stop);

	// 負荷テストパラメーター
	DDX_Text(pDX, IDC_pointA, pointA);
	DDX_Text(pDX, IDC_pointB, pointB);
	DDX_Text(pDX, IDC_time_A, time_A);
	DDX_Text(pDX, IDC_time_B, time_B);
	DDX_Text(pDX, IDC_timeAB, timeAB);
	DDX_Text(pDX, IDC_timeBA, timeBA);

	// axis0,1使用チェック
	DDX_Check(pDX, IDC_CHECK00, chk_axis[0]);
	DDX_Check(pDX, IDC_CHECK01, chk_axis[1]);

}

BEGIN_MESSAGE_MAP(CROBOT_CTLDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()

	ON_BN_CLICKED(IDOK, &CROBOT_CTLDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CROBOT_CTLDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_SV_OPEN, &CROBOT_CTLDlg::OnBnClickedSvOpen)
	ON_BN_CLICKED(IDC_SV_CLOSE, &CROBOT_CTLDlg::OnBnClickedSvClose)
	ON_BN_CLICKED(IDC_BUTTON_CSV, &CROBOT_CTLDlg::OnBnClickedButtonCsv)
	ON_BN_CLICKED(IDC_PLAY, &CROBOT_CTLDlg::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_STOP, &CROBOT_CTLDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BUTTON4, &CROBOT_CTLDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_WRITE_CONF, &CROBOT_CTLDlg::OnBnClickedWriteConf)
	ON_BN_CLICKED(IDC_SAVE_ODRIVE, &CROBOT_CTLDlg::OnBnClickedSaveOdrive)
	ON_BN_CLICKED(IDC_BUTTON3, &CROBOT_CTLDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CROBOT_CTLDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON20, &CROBOT_CTLDlg::OnBnClickedButton20)
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

	// 送信用スライダ
	goal_angle_ctl[0].SetRange(-1800, 1800);	// 最大・最少180度 ※0.1度きざみに出来るよう1800
	goal_angle_ctl[1].SetRange(-1800, 1800);	// 最大・最少180度 ※0.1度きざみに出来るよう1800

	// 受信用スライダ
	sv_angle_sl_ctl[0].SetRange(-180, 180);		// 最大・最少180度
	sv_angle_sl_ctl[1].SetRange(-180, 180);		// 最大・最少180度
	
	// 一旦適当な数値にして画面更新(最大最小の反映のため)
	goal_angle[0] = 100;
	goal_angle[1] = 100;
	sv_angle_rw[0] = 100;
	sv_angle_rw[1] = 100;
	UpdateData(FALSE);	// 値→GUI

	// 0位置でリセット＆画面更新
	goal_angle[0] = 0;
	goal_angle[1] = 0;
	sv_angle_rw[0] = 0;
	sv_angle_rw[1] = 0;
	UpdateData(FALSE);	// 値→GUI

	// INIファイルから取得
	GetPrivateProfileString("serial", "COM", "INI ERROR", get_port, sizeof(get_port), SETFILE);
	get_baud = GetPrivateProfileInt("serial", "BAUD", 0, SETFILE);

	com_number = get_port;
	com_baudrate.Format(_T("%d bps"), get_baud);
	com_openclode = "CLOSE";


	// A-B 試験 基本パラメータ
	pointA = 0;
	pointB = 20;
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


void CROBOT_CTLDlg::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};

	// リセットコマンド
	ODrive.reset();

	// シリアルポートクローズ
	ODrive.close();

}

void CROBOT_CTLDlg::OnBnClickedSvOpen()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	com_openclode = "OPEN";
	UpdateData(FALSE);
	
	//	sv_open();
	sv_ctl_f = 1;	//スレッドコントロール用のフラグ (スレッドを立ち上げる前に1にして、これを0にするとスレッドの無限ループが終わる)

	////////////スレッド用パラメータ
	HANDLE handle;

	////////////スレッドの生成
	handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)thread_motor, NULL, 0, NULL);


	// 表示の変更
	sv_opne_ctlf.EnableWindow(FALSE);
	sv_close_ctlf.EnableWindow(TRUE);

	UpdateData(FALSE);

}

void CROBOT_CTLDlg::OnBnClickedSvClose()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	sv_ctl_f = 0;

	com_openclode = "CLOSE";
	UpdateData(FALSE);

	sv_opne_ctlf.EnableWindow(TRUE);
	sv_close_ctlf.EnableWindow(FALSE);

}

//----------------------------サーボモーターの関連関数 -------------------------

//モーター制御用のスレッド
void thread_motor(void)
{

	int i;
	SYSTEMTIME st;
	clock_t now_time = 0,old_time = 0;	// 時間計測用

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
												//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};

	while (sv_ctl_f) {
		
		// 処理時間の取得
		now_time = clock();						// 現在時間の更新
		Processing_time = now_time - old_time;	// 処理時間の算出
		old_time = now_time;					// 過去時間の更新

		
		// ODriveパラメータ取得(axis0)
		if (chk_axis[0]){
			r_vel_m[0] = ODrive.GetVelocity(0);
			r_pos_m[0] = ODrive.GetPosition(0);
			r_current_m[0] = ODrive.GetCurrent(0);
		}

		// ODriveパラメータ取得(axis1)
		if (chk_axis[1]) {
			r_vel_m[1] = ODrive.GetVelocity(1);
			r_pos_m[1] = ODrive.GetPosition(1);
			r_current_m[1] = ODrive.GetCurrent(1);
		}

		// 時間計測
		Processing_time1 = clock() - now_time;

		// ODriveパラメータ送信(axis0)
		if (chk_axis[0]) {
			pos_m[0] = (float)goal_angle[0] / 36.0f;
			ODrive.SetPosition(0, pos_m[0]);
		}
		// ODriveパラメータ送信(axis1)
		if (chk_axis[1]) {
			pos_m[1] = (float)goal_angle[1] / 36.0f;
			ODrive.SetPosition(1, pos_m[1]);
		}

		// 時間計測
		Processing_time2 = clock() - Processing_time1 - now_time;


		if (flag_csv) {	// CSVに記録

			// 時間の取得
			GetSystemTime(&st);
			// wHourを９時間足して、日本時間にする
			fprintf_s(csv_fp, "\"%04d/%02d/%02d_%02d:%02d:%02d.%03d\",",
				st.wYear, st.wMonth, st.wDay,
				st.wHour + 9, st.wMinute, st.wSecond, st.wMilliseconds);

			for (i = 0; i < 2; i++) {
				//トルクオフなら取得値　オンなら指示角 g_angle	
				fprintf_s(csv_fp, "%f,", pos_m[i] * 3.6f); // 目標角度
				fprintf_s(csv_fp, "%f,", r_pos_m[i] * 3.6f);   // 実測角度
				fprintf_s(csv_fp, "%f,", r_vel_m[i] * 3.6f);   // 実測角速度
				fprintf_s(csv_fp, "%f,", r_current_m[i] * 3.6f);    // 実測電流
			}
			fprintf_s(csv_fp, "\n");
		}

		// 時間計測
		Processing_time3 = clock() - Processing_time2 - now_time - Processing_time1;
	}

	ODrive.close();	// シリアルポートクローズ

}


void CROBOT_CTLDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	if (nIDEvent == 5678) {

		if (f_testmove_do == 0) UpdateData(TRUE);		// スライダの位置・ボックスの値を変数に代入 (再生中でなければ)

		sv_speed_r[0].Format(_T("%.1f"), r_vel_m[0] * 3.6f);			//受信した値[速度]を格納 (減速後のdeg/sに変換)
		sv_angle_r[0].Format(_T("%.1f"), r_pos_m[0] * 3.6f);			//受信した値[角度]を格納 (減速後のdegに変換)
		sv_angle_rw[0] = r_pos_m[0] * 3.6f;
		sv_load_r[0].Format(_T("%.0f"), r_current_m[0] * 1000);			//受信した値[負荷]を格納 (mAに変換)

		goal_angle_str[0].Format(_T("%.1f"), goal_angle[0]/10.0f);			// 角度→値ボックス
		
		sv_speed_r[1].Format(_T("%.1f"), r_vel_m[1] * 3.6f);			//受信した値[速度]を格納 (減速後のdeg/sに変換)
		sv_angle_r[1].Format(_T("%.1f"), r_pos_m[1] * 3.6f);			//受信した値[角度]を格納 (減速後のdegに変換)
		sv_angle_rw[1] = r_pos_m[1] * 3.6f;
		sv_load_r[1].Format(_T("%.0f"), r_current_m[1] * 1000);			//受信した値[負荷]を格納 (mAに変換)
						
		goal_angle_str[1].Format(_T("%.1f"), goal_angle[1]/10.0f);			// 角度→値ボックス

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
		for (i = 0; i < 2; i++) {
			//トルクオフなら取得値　オンなら指示角 g_angle	
			fprintf_s(csv_fp, "目標角度(deg)[axis:%d],", i); // 目標角度
			fprintf_s(csv_fp, "実測角度(deg)[axis:%d],", i);   // 実測角度
			fprintf_s(csv_fp, "実測角速度(deg/s)[axis:%d],", i);   // 実測角速度
			fprintf_s(csv_fp, "実測電流(mA)[axis:%d],", i);    // 実測電流
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

void thread_testmove(void) {

	// 現在位置→A→B→A→

	int i;
	int temp_point;

	// テスト動作実行中フラグ
	f_testmove_do = 1;

	temp_point = goal_angle[0];	// 現在地を保存

	timeBeginPeriod(1);// タイマーの最小精度を1msecにする

	while (f_testmove) {

		// 現在位置からA
		for (i = 0; i <= (timeBA/10); i++) {
			goal_angle[0] = temp_point + (int)(((double)i / (double)(timeBA / 10)) * ((pointA*10) - temp_point));			// キャストのトラブル
			Sleep(10);	// 10ミリ秒
		}

		// A地点で静止
		Sleep(time_A);

		// A地点からB
		for (i = 0; i <= (timeAB/10); i++) {
			goal_angle[0] = (pointA * 10) + (int)(((double)i / (double)(timeAB / 10)) * ((pointB * 10) - (pointA * 10)));			// キャストのトラブル
			Sleep(10);	// 10ミリ秒
		}

		// A地点で静止
		Sleep(time_A);

		temp_point = (pointB * 10);	// 現在地にBを保存

	}

	timeEndPeriod(1);// タイマーの最小精度を戻す
	// B地点で静止

	// テスト動作実行中フラグ
	f_testmove_do = 0;
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

	UpdateData(FALSE);

}

void CROBOT_CTLDlg::OnBnClickedStop()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	f_testmove = 0; 

	test_play.EnableWindow(TRUE);
	test_stop.EnableWindow(FALSE);

	UpdateData(FALSE);
}


void CROBOT_CTLDlg::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
												//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};

	if (chk_axis[0]) ODrive.ODriveINIT(0);	// 使用するなら初期化
	if (chk_axis[1]) ODrive.ODriveINIT(1);	// 使用するなら初期化

	ODrive.close();	// シリアルポートクローズ
}


void CROBOT_CTLDlg::OnBnClickedWriteConf()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
										//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};

	UpdateData(TRUE);	// 値 <- GUI

	ODrive.Set_Vel_limit(0, atof(Vel_limit[0]));
	ODrive.Set_Current_lim(0, atof(Current_lim[0]));
	ODrive.Set_pos_gain(0, atof(Pos_gain[0]));
	ODrive.Set_vel_gain(0, atof(Vel_gain[0]));
	ODrive.Set_vel_integrator_gain(0, atof(Vel_integrator_gain[0]));

	ODrive.Set_Vel_limit(1, atof(Vel_limit[1]));
	ODrive.Set_Current_lim(1, atof(Current_lim[1]));
	ODrive.Set_pos_gain(1, atof(Pos_gain[1]));
	ODrive.Set_vel_gain(1, atof(Vel_gain[1]));
	ODrive.Set_vel_integrator_gain(1, atof(Vel_integrator_gain[1]));

	ODrive.close();	// シリアルポートクローズ

}

void CROBOT_CTLDlg::OnBnClickedSaveOdrive()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。


	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};

	// コンフィグセーブ
	ODrive.save_conf();

	// シリアルポートクローズ
	ODrive.close();	

}

void CROBOT_CTLDlg::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	float temp = 0;

	if (ODrive.init(get_port, get_baud) != true) {		//	COMポート RS232Cの初期化
											//	printf("ポート(%s)がオープン出来ませんでした。\n",OPN_COM);
		while (1);
	};

	temp = ODrive.Get_Vel_limit(0);
	Vel_limit[0].Format(_T("%.2f"), temp);

	temp = ODrive.Get_Current_lim(0);
	Current_lim[0].Format(_T("%.2f"), temp);

	temp = ODrive.Get_pos_gain(0);
	Pos_gain[0].Format(_T("%.2f"), temp);

	temp = ODrive.Get_vel_gain(0);
	Vel_gain[0].Format(_T("%.2f"), temp);

	temp = ODrive.Get_integrator_gain(0);
	Vel_integrator_gain[0].Format(_T("%.2f"), temp);


	temp = ODrive.Get_Vel_limit(1);
	Vel_limit[1].Format(_T("%.2f"), temp);

	temp = ODrive.Get_Current_lim(1);
	Current_lim[1].Format(_T("%.2f"), temp);

	temp = ODrive.Get_pos_gain(1);
	Pos_gain[1].Format(_T("%.2f"), temp);

	temp = ODrive.Get_vel_gain(1);
	Vel_gain[1].Format(_T("%.2f"), temp);

	temp = ODrive.Get_integrator_gain(1);
	Vel_integrator_gain[1].Format(_T("%.2f"), temp);

	UpdateData(FALSE);	// 値→GUI

	ODrive.close();	// シリアルポートクローズ
}


void CROBOT_CTLDlg::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	goal_angle[0] = 0;

	UpdateData(FALSE);	// 値→GUI

}


void CROBOT_CTLDlg::OnBnClickedButton20()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。


	goal_angle[1] = 0;

	UpdateData(FALSE);	// 値→GUI
}
