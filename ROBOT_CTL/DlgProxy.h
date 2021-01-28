
// DlgProxy.h: ヘッダー ファイル
//

#pragma once

class CROBOT_CTLDlg;


// CROBOT_CTLDlgAutoProxy コマンド ターゲット

class CROBOT_CTLDlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CROBOT_CTLDlgAutoProxy)

	CROBOT_CTLDlgAutoProxy();           // 動的生成で使用される protected コンストラクター

// 属性
public:
	CROBOT_CTLDlg* m_pDialog;

// 操作
public:

// オーバーライド
	public:
	virtual void OnFinalRelease();

// 実装
protected:
	virtual ~CROBOT_CTLDlgAutoProxy();

	// 生成された、メッセージ割り当て関数

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CROBOT_CTLDlgAutoProxy)

	// 生成された OLE ディスパッチ割り当て関数

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

