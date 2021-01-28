
// DlgProxy.cpp : 実装ファイル
//

#include "stdafx.h"
#include "ROBOT_CTL.h"
#include "DlgProxy.h"
#include "ROBOT_CTLDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CROBOT_CTLDlgAutoProxy

IMPLEMENT_DYNCREATE(CROBOT_CTLDlgAutoProxy, CCmdTarget)

CROBOT_CTLDlgAutoProxy::CROBOT_CTLDlgAutoProxy()
{
	EnableAutomation();
	
	// オートメーション オブジェクトがアクティブである限り、アプリケーションを 
	//	実行状態にしてください、コンストラクターは AfxOleLockApp を呼び出します。
	AfxOleLockApp();

	// アプリケーションのメイン ウィンドウ ポインターをとおしてダイアログ
	//  へアクセスします。プロキシの内部ポインターからダイアログへのポイ
	//  ンタを設定し、ダイアログの戻りポインターをこのプロキシへ設定しま
	//  す。
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CROBOT_CTLDlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CROBOT_CTLDlg)))
		{
			m_pDialog = reinterpret_cast<CROBOT_CTLDlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CROBOT_CTLDlgAutoProxy::~CROBOT_CTLDlgAutoProxy()
{
	// すべてのオブジェクトがオートメーションで作成された場合にアプリケーション
	//	を終了するために、デストラクターが AfxOleUnlockApp を呼び出します。
	//  他の処理の間に、メイン ダイアログを破壊します。
	if (m_pDialog != NULL)
		m_pDialog->m_pAutoProxy = NULL;
	AfxOleUnlockApp();
}

void CROBOT_CTLDlgAutoProxy::OnFinalRelease()
{
	// オートメーション オブジェクトに対する最後の参照が解放されるときに
	// OnFinalRelease が呼び出されます。基底クラスは自動的にオブジェクトを
	// 削除します。追加のクリーンアップが必要です
	// 別な後処理を追加してください。

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CROBOT_CTLDlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CROBOT_CTLDlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// メモ: VBA からタイプ セーフなバインドをサポートするために、IID_IROBOT_CTL のサポートを追加します。
//  この IID は、.IDL ファイルのディスパッチ インターフェイスへアタッチされる 
//  GUID と一致しなければなりません。

// {7247C1E0-D0DC-46E5-9EAC-AB57596BF892}
static const IID IID_IROBOT_CTL =
{ 0x7247C1E0, 0xD0DC, 0x46E5, { 0x9E, 0xAC, 0xAB, 0x57, 0x59, 0x6B, 0xF8, 0x92 } };

BEGIN_INTERFACE_MAP(CROBOT_CTLDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CROBOT_CTLDlgAutoProxy, IID_IROBOT_CTL, Dispatch)
END_INTERFACE_MAP()

// IMPLEMENT_OLECREATE2 マクロが、このプロジェクトの StdAfx.h で定義されます。
// {F971C2A3-EE25-4C5F-A5E2-86C3A36DC739}
IMPLEMENT_OLECREATE2(CROBOT_CTLDlgAutoProxy, "ROBOT_CTL.Application", 0xf971c2a3, 0xee25, 0x4c5f, 0xa5, 0xe2, 0x86, 0xc3, 0xa3, 0x6d, 0xc7, 0x39)


// CROBOT_CTLDlgAutoProxy メッセージ ハンドラー
