
// ROBOT_CTLDlg.h : ヘッダー ファイル
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"




class CROBOT_CTLDlgAutoProxy;


// CROBOT_CTLDlg ダイアログ
class CROBOT_CTLDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CROBOT_CTLDlg);
	friend class CROBOT_CTLDlgAutoProxy;

// コンストラクション
public:
	CROBOT_CTLDlg(CWnd* pParent = NULL);	// 標準コンストラクター
	virtual ~CROBOT_CTLDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ROBOT_CTL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	CROBOT_CTLDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedSvOpen();
	afx_msg void OnBnClickedSvClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedRec();



	CEdit com_num;
	CString com_number;
	CString com_baudrate;
	CString com_openclode;
	CString sv_count_s;
	CString sv_angle_r[100];
	CString sv_time_r[100];
	CString sv_speed_r[100];
	CString sv_load_r[100];
	CString sv_temperature_r[100];

	CBitmap m_bmp;  // 表示するビットマップ

	CString sv_error_r[100];
	CString sv_diff_w[100];





	int sv_angle_rw[100];
	CString sv_angle_rw_v[100];


	CString sv_angle_rw_v_all;


	CSliderCtrl sv_angle_sl_ctl[100];
	CSliderCtrl sv_angle_sl_ctl_all;

	CButton sv_opne_ctlf;
	CButton sv_close_ctlf;
	CButton sv_csv_cap;

	CButton test_play;
	CButton test_play2;
	CButton test_stop;

	CString temp0001;
	int flip_radio;



	CString csvlist;
	CListBox csv_list;

	CString current_filename;

	CListBox move_list;
	CString select_move;

	CListBox motion_file_list;
	CString motion_file_select;
	CString motion_name;

	int saisei_kaishi;


	afx_msg void OnBnClickedButtonCsv();
	afx_msg void OnBnClickedButtonZero();
	afx_msg void OnBnClickedButtonDiff();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedPlay2();
	afx_msg void OnBnClickedButton4();
};
