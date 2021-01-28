
// ROBOT_CTL.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CROBOT_CTLApp:
// このクラスの実装については、ROBOT_CTL.cpp を参照してください。
//

class CROBOT_CTLApp : public CWinApp
{
public:
	CROBOT_CTLApp();

// オーバーライド
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CROBOT_CTLApp theApp;
