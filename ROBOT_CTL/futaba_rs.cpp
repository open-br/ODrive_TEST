#include <stdio.h>						// 標準ヘッダー
#include <windows.h>					// Windows API用ヘッダー
#include "stdafx.h"
#include "futaba_rs.h"

#define SERVO_ID		1				// サーボIDの指定「1」に設定
#define COM_PORT		(char *)"COM1"	// 通信ポートの指定

// サーボの動作
void futaba_rs::sv_move(int id, short sPos, unsigned short sTime) {
	
	unsigned char	sendbuf[28];
	unsigned char	sum;
	int				i;

	// バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	// パケット作成
	sendbuf[0] = (unsigned char)0xFA;						// ヘッダー1
	sendbuf[1] = (unsigned char)0xAF;						// ヘッダー2
	sendbuf[2] = (unsigned char)id;							// サーボID
	sendbuf[3] = (unsigned char)0x00;						// フラグ
	sendbuf[4] = (unsigned char)0x1E;						// アドレス(0x1E=30)
	sendbuf[5] = (unsigned char)0x04;						// 長さ(4byte)
	sendbuf[6] = (unsigned char)0x01;						// 個数
	sendbuf[7] = (unsigned char)(sPos & 0x00FF);			// 位置
	sendbuf[8] = (unsigned char)((sPos & 0xFF00) >> 8);		// 位置
	sendbuf[9] = (unsigned char)(sTime & 0x00FF);			// 時間
	sendbuf[10] = (unsigned char)((sTime & 0xFF00) >> 8);	// 時間
															// チェックサムの計算
	sum = sendbuf[2];
	for (i = 3; i < 11; i++) {
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[11] = sum;								// チェックサム

	send((char *)sendbuf, 12);

}

// サーボIDとトルクのオンオフ
void futaba_rs::sv_torque(int id, int torque) {

	unsigned char	sendbuf[28];
	unsigned char	sum;
	int				i;

	short sPos = 0;
	unsigned short sTime = 0;

	// バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	// パケット作成
	sendbuf[0] = (unsigned char)0xFA;			// ヘッダー1
	sendbuf[1] = (unsigned char)0xAF;			// ヘッダー2
	sendbuf[2] = (unsigned char)id;				// サーボID
	sendbuf[3] = (unsigned char)0x00;			// フラグ
	sendbuf[4] = (unsigned char)0x24;			// アドレス(0x1E=30)
	sendbuf[5] = (unsigned char)0x01;			// 長さ(1byte)
	sendbuf[6] = (unsigned char)0x01;			// 個数
	sendbuf[7] = (unsigned char)torque;		    // トルク

												// チェックサムの計算
	sum = sendbuf[2];
	for (i = 3; i < 8; i++) {
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[8] = sum;								// チェックサム

	send((char *)sendbuf, 9);

}

sv_r futaba_rs::sv_read(int id)
{
	unsigned char	sendbuf[32];
	unsigned char	readbuf[128];
	unsigned char	sum;
	int				i;
	sv_r rDATA;

	// バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	// パケット作成
	sendbuf[0] = (unsigned char)0xFA;				// ヘッダー1
	sendbuf[1] = (unsigned char)0xAF;				// ヘッダー2
	sendbuf[2] = (unsigned char)id;					// サーボID
	sendbuf[3] = (unsigned char)0x09;				// フラグ(0x01 | 0x04<<1)
	sendbuf[4] = (unsigned char)0x00;				// アドレス(0x00)
	sendbuf[5] = (unsigned char)0x00;				// 長さ(0byte)
	sendbuf[6] = (unsigned char)0x01;				// 個数
													// チェックサムの計算
	sum = sendbuf[2];
	for (i = 3; i < 7; i++) {
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[7] = sum;								// チェックサム


	send((char *)sendbuf, 8);


	// 読み込みバッファクリア
	memset(readbuf, 0x00, sizeof(readbuf));

	// 受信！
	receive((char *)readbuf, 26);

	// 受信データの確認
	sum = readbuf[2];
	for (i = 3; i < 26; i++) {
		sum = sum ^ readbuf[i];
	}

	if (sum) {
		// チェックサムエラー
		rDATA.error = CHECKSUM_ERROR;
		return rDATA;
	}

	rDATA.angle = ((readbuf[8] << 8) & 0x0000FF00) | (readbuf[7] & 0x000000FF);
	rDATA.time = ((readbuf[10] << 8) & 0x0000FF00) | (readbuf[9] & 0x000000FF);
	rDATA.speed = ((readbuf[12] << 8) & 0x0000FF00) | (readbuf[11] & 0x000000FF);
	rDATA.load = ((readbuf[14] << 8) & 0x0000FF00) | (readbuf[13] & 0x000000FF);
	rDATA.temperature = ((readbuf[16] << 8) & 0x0000FF00) | (readbuf[15] & 0x000000FF);

	rDATA.error = NO_ERROR;	// エラーないよ
	if (readbuf[3] != NO_ERROR)	rDATA.error = readbuf[3];

	return rDATA;
}


sv_r futaba_rs::sv_read2(int id)
{
	unsigned char	sendbuf[32];
	unsigned char	readbuf[128];
	unsigned char	sum;
	int				i;
	sv_r rDATA;

	// バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	// パケット作成
	sendbuf[0] = (unsigned char)0xFA;				// ヘッダー1
	sendbuf[1] = (unsigned char)0xAF;				// ヘッダー2
	sendbuf[2] = (unsigned char)id;					// サーボID
	sendbuf[3] = (unsigned char)0x0F;				// フラグ(0x0F) 指定アドレスからの指定の長さを返答
	sendbuf[4] = (unsigned char)0x2A;				// アドレス(0x2A) 現在位置
	sendbuf[5] = (unsigned char)0x0A;				// 長さ(10byte)
	sendbuf[6] = (unsigned char)0x00;				// 個数 (任意アドレスリターンの場合はcnt=0x00)

													// チェックサムの計算
	sum = sendbuf[2];
	for (i = 3; i < 7; i++) {
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[7] = sum;								// チェックサム


	send((char *)sendbuf, 8);


	// 読み込みバッファクリア
	memset(readbuf, 0x00, sizeof(readbuf));

	// 受信！
	receive((char *)readbuf, 18);

	// 受信データの確認
	sum = readbuf[2];
	for (i = 3; i < 18; i++) {
		sum = sum ^ readbuf[i];
	}


	if (sum) {
		// チェックサムエラー
		rDATA.error = CHECKSUM_ERROR;
		return rDATA;
	}

	rDATA.angle = ((readbuf[8] << 8) & 0x0000FF00) | (readbuf[7] & 0x000000FF);
	rDATA.time = ((readbuf[10] << 8) & 0x0000FF00) | (readbuf[9] & 0x000000FF);
	rDATA.speed = ((readbuf[12] << 8) & 0x0000FF00) | (readbuf[11] & 0x000000FF);
	rDATA.load = ((readbuf[14] << 8) & 0x0000FF00) | (readbuf[13] & 0x000000FF);
	rDATA.temperature = ((readbuf[16] << 8) & 0x0000FF00) | (readbuf[15] & 0x000000FF);

	rDATA.error = NO_ERROR;	// エラーないよ
	if (readbuf[3] != NO_ERROR)	rDATA.error = readbuf[3];

	return rDATA;
}


void futaba_rs::sv_move_long(sv_r sendDATA[100]) {

	unsigned char	sendbuf[200];
	unsigned char	sum;
	int				i,j;

	// バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	// パケット作成
	sendbuf[0] = (unsigned char)0xFA;				    // ヘッダー1
	sendbuf[1] = (unsigned char)0xAF;				    // ヘッダー2
	sendbuf[2] = (unsigned char)0x00;				    // ID
	sendbuf[3] = (unsigned char)0x00;				    // フラグ
	sendbuf[4] = (unsigned char)0x1E;				    // アドレス(0x1E=30)
	sendbuf[5] = (unsigned char)0x05;				    // 長さ(5byte)
	sendbuf[6] = (unsigned char)0x08;				    // 個数8個(0x08=0)

	j = 7;

	for (i = 1; i < 9; i++) {

		sendbuf[j] = i;															// ID

		sendbuf[j + 1] = (unsigned char)(sendDATA[i].g_angle & 0x00FF);			// 位置
		sendbuf[j + 2] = (unsigned char)((sendDATA[i].g_angle & 0xFF00) >> 8);	// 位置
		
		sendbuf[j + 3] = (unsigned char)(sendDATA[i].g_time & 0x00FF);			// 速度
		sendbuf[j + 4] = (unsigned char)((sendDATA[i].g_time & 0xFF00) >> 8);	// 速度

		j = j + 5;
	}


													// チェックサムの計算
	sum = sendbuf[2];
	for (i = 3; i < j; i++) {
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[j] = sum;								// チェックサム

	j++;
	send((char *)sendbuf, j);						// 送信

}



int futaba_rs::sv_read_torque(int id)
{
	unsigned char	sendbuf[32];
	unsigned char	readbuf[128];
	unsigned char	sum;
	int				i;
	//	sv_r rDATA;

	// バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	// パケット作成
	sendbuf[0] = (unsigned char)0xFA;				// ヘッダー1
	sendbuf[1] = (unsigned char)0xAF;				// ヘッダー2
	sendbuf[2] = (unsigned char)id;					// サーボID

	sendbuf[3] = (unsigned char)0x0F;				// フラグ(0x0F) 指定アドレスからの指定の長さを返答

	sendbuf[4] = (unsigned char)0x24;				// アドレス(0x07) 返信遅延時間
	sendbuf[5] = (unsigned char)0x01;				// 長さ(1byte)
	sendbuf[6] = (unsigned char)0x00;				// 個数 (任意アドレスリターンの場合はcnt=0x00)
													// チェックサムの計算
	sum = sendbuf[2];
	for (i = 3; i < 7; i++) {
		sum = (unsigned char)(sum ^ sendbuf[i]);
	}
	sendbuf[7] = sum;								// チェックサム


	send((char *)sendbuf, 8);


	// 読み込みバッファクリア
	memset(readbuf, 0x00, sizeof(readbuf));

	// 受信！
	receive((char *)readbuf, 9);

	// 受信データの確認
	sum = readbuf[2];
	for (i = 3; i < 9; i++) {
		sum = sum ^ readbuf[i];
	}

	if (sum) {
		// チェックサムエラー
		return -3;
	}

	return readbuf[7];
}



//---------------------------------------------------------------------------
// Futabaのsampleから拝借
bool futaba_rs::init(char* comport_in, int baudrate)
{
	bool flag = true;


	//HANDLE			hComm;		// 通信用ハンドル
	DCB				cDcb;		// 通信設定用
	COMMTIMEOUTS	cTimeouts;	// 通信ポートタイムアウト用

	sprintf_s(comport, comport_in);


	// 通信ポートを開く
	hComm = CreateFileA(comport,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	// ハンドルの確認
	if (hComm == INVALID_HANDLE_VALUE) {
		hComm = NULL;
		goto FuncEnd;
	}


	// 通信設定
	if (!GetCommState(hComm, &cDcb)) {
		// 通信設定エラーの場合
		printf("ERROR:GetCommState error\n");
		CommClose();
		hComm = NULL;
		goto FuncEnd;
	}
	cDcb.BaudRate = baudrate;				// 通信速度
	cDcb.ByteSize = 8;					// データビット長
	cDcb.fParity = TRUE;				// パリティチェック
	cDcb.Parity = NOPARITY;			// ノーパリティ
	cDcb.StopBits = ONESTOPBIT;			// 1ストップビット

	if (!SetCommState(hComm, &cDcb)) {
		// 通信設定エラーの場合
		printf("ERROR:GetCommState error\n");
		CommClose();
		hComm = NULL;
		goto FuncEnd;
	}


	// 通信設定(通信タイムアウト設定)
	// 文字の読み込み待ち時間(ms)
	cTimeouts.ReadIntervalTimeout = 50;
	// 読み込みの１文字あたりの時間(ms)
	cTimeouts.ReadTotalTimeoutMultiplier = 50;
	// 読み込みの定数時間(ms)
	cTimeouts.ReadTotalTimeoutConstant = 50;
	// 書き込みの１文字あたりの時間(ms)
	cTimeouts.WriteTotalTimeoutMultiplier = 0;

	if (!SetCommTimeouts(hComm, &cTimeouts)) {
		// 通信設定エラーの場合
		printf("ERROR:SetCommTimeouts error\n");
		CommClose();
		hComm = NULL;
		goto FuncEnd;
	}


	// 通信バッファクリア
	PurgeComm(hComm, PURGE_RXCLEAR);


FuncEnd:
	//return hComm;
	flag_opened = 1;
	return(flag);

}

//---------------------------------------------------------------------------
bool futaba_rs::close(void)
{
	if (flag_opened != 1)return false;

	if (hComm) {
		CloseHandle(hComm);
	}

	return 1;
	//return CloseHandle(hComm);

}
//---------------------------------------------------------------------------
int futaba_rs::receive(char* buf_ptr, int size)
{
	//受け取ったバイト長さを返す->良くない．EOFを超えたときにはまる
	//ReadFileの返り値がtrueで，byteが0のとき
	//->ファイルポインタがEOFを超えてしまった，という意味らしい
	unsigned long byte, event;
	byte = 0;
	bool flag = false;


	while (!flag)
	{
		//文字を受信するまで待つ
		WaitCommEvent(hComm, &event, NULL);
		if (event | EV_RXCHAR)//文字を受信したか？
		{
			if (ReadFile(hComm, buf_ptr, size, &byte, NULL))
				flag = true;
			break;
		}
		byte = 0;//初期化
	}


	return byte;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int futaba_rs::send(char* buf_ptr, int size)
{
	//バッファの内容を送る
	unsigned long byte;
	if (flag_opened != 1)
	{
		printf("send() error. port Not opend\n");//debug
		printf("flag_opened=%d\n", flag_opened);//debug
		return -1;
	}


	if (WriteFile(hComm, buf_ptr, size, &byte, NULL))
		//↑bufferから読み出し＆hcom(通信ポート)へ書き込み
	{
		return byte;
	}
	else return -1;

}
//---------------------------------------------------------------------------
/*----------------------------------------------------------------------------*/
/*
 *	概要：通信ポートを閉じる
 *
 *	関数：HANDLE CommClose( HANDLE hComm )
 *	引数：
 *		HANDLE			hComm		通信ポートのハンドル
 *	戻り値：
 *		1				成功
 *
 */
int futaba_rs::CommClose()
{
	if( hComm ){
		CloseHandle( hComm );
	}

	return 1;
}