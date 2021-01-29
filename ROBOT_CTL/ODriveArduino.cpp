//#include "Arduino.h"
#include <stdio.h>						// 標準ヘッダー
#include <windows.h>					// Windows API用ヘッダー
#include "stdafx.h"
#include "ODriveArduino.h"
#include <string>

#define COM_PORT		(char *)"COM1"	// 通信ポートの指定


int sjlen(const char* str);

static const int kMotorOffsetFloat = 2;
static const int kMotorStrideFloat = 28;
static const int kMotorOffsetInt32 = 0;
static const int kMotorStrideInt32 = 4;
static const int kMotorOffsetBool = 0;
static const int kMotorStrideBool = 4;
static const int kMotorOffsetUint16 = 0;
static const int kMotorStrideUint16 = 2;

// Print with stream operator
//template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
//template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }

//ODriveArduino::ODriveArduino(Stream& serial)
//    : serial_(serial) {}

void ODriveArduino::SetPosition(int motor_number, float position) {
    SetPosition(motor_number, position, 0.0f, 0.0f);
}

void ODriveArduino::SetPosition(int motor_number, float position, float velocity_feedforward) {
    SetPosition(motor_number, position, velocity_feedforward, 0.0f);
}

void ODriveArduino::SetPosition(int motor_number, float position, float velocity_feedforward, float current_feedforward) {

	char sendbuf[100];
	// 送信バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	//serial_ << "p " << motor_number  << " " << position << " " << velocity_feedforward << " " << current_feedforward << "\n";
	sprintf_s(sendbuf, "p %d %f %f %f\n", motor_number, position, velocity_feedforward, current_feedforward);


	send((char*)sendbuf, sjlen((char*)sendbuf));
}

void ODriveArduino::SetVelocity(int motor_number, float velocity) {
    SetVelocity(motor_number, velocity, 0.0f);
}

void ODriveArduino::SetVelocity(int motor_number, float velocity, float current_feedforward) {
    


	char sendbuf[100];
	// 送信バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	//serial_ << "v " << motor_number  << " " << velocity << " " << current_feedforward << "\n";
	sprintf_s(sendbuf, "v %d %f %f\n", motor_number, velocity, current_feedforward);


	send((char*)sendbuf, sjlen((char*)sendbuf));


}

void ODriveArduino::SetCurrent(int motor_number, float current) {



	char sendbuf[100];
	// 送信バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	//serial_ << "c " << motor_number << " " << current << "\n";
	sprintf_s(sendbuf, "c %d %f\n", motor_number, current);


	send((char*)sendbuf, sjlen((char*)sendbuf));



}

void ODriveArduino::TrapezoidalMove(int motor_number, float position){



	char sendbuf[100];
	// 送信バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));

	//serial_ << "t " << motor_number << " " << position << "\n";
	sprintf_s(sendbuf, "t %d %f\n", motor_number, position);


	send((char*)sendbuf, sjlen((char*)sendbuf));




}

void ODriveArduino::ODriveINIT() {

	char sendbuf1[100];
	char sendbuf2[100];
	char sendbuf3[100];
	char sendbuf4[100];

	int requested_state;


	// 読み込みバッファクリア
	memset(sendbuf1, 0x00, sizeof(sendbuf1));
	// 読み込みバッファクリア
	memset(sendbuf2, 0x00, sizeof(sendbuf2));
	// 読み込みバッファクリア
	memset(sendbuf3, 0x00, sizeof(sendbuf3));
	// 読み込みバッファクリア
	memset(sendbuf4, 0x00, sizeof(sendbuf4));


	sprintf_s(sendbuf1, "w axis0.controller.config.vel_limit %f\n", 10.0f);
//	send((char*)sendbuf1, sizeof(sendbuf1));
	send((char*)sendbuf1, sjlen((char*)sendbuf1));

	sprintf_s(sendbuf2, "w axis0.motor.config.current_lim %f\n", 11.0f);
//	send((char*)sendbuf2, sizeof(sendbuf2));
	send((char*)sendbuf2, sjlen((char*)sendbuf2));

	sprintf_s(sendbuf3, "w axis1.controller.config.vel_limit %f\n", 10.0f);
//	send((char*)sendbuf3, sizeof(sendbuf3));
	send((char*)sendbuf3, sjlen((char*)sendbuf3));

	sprintf_s(sendbuf4, "w axis1.motor.config.current_lim %f\n", 11.0f);
//	send((char*)sendbuf4, sizeof(sendbuf4));
	send((char*)sendbuf4, sjlen((char*)sendbuf4));



	requested_state = AXIS_STATE_MOTOR_CALIBRATION;
	//sprintf_s(sendbuf5, "Axis0: Requesting state %d\n", requested_state);
	//send((char*)sendbuf5, sizeof(sendbuf5));
	if (!run_state(0, requested_state, true)) return;



	requested_state = AXIS_STATE_ENCODER_OFFSET_CALIBRATION;
	//sprintf_s(sendbuf6, "Axis0: Requesting state %d\n", requested_state);
	//send((char*)sendbuf6, sizeof(sendbuf6));
	if (!run_state(0, requested_state, true, 25.0f)) return;





	requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL;
	//sprintf_s(sendbuf7, "Axis0: Requesting state %d\n", requested_state);
	//send((char*)sendbuf7, sizeof(sendbuf7));
	if (!run_state(0, requested_state, false /*don't wait*/)) return;



}





float ODriveArduino::readFloat(void) {
	//return readString().toFloat();
	return atof(readString());  
}

float ODriveArduino::GetVelocity(int motor_number){

	char sendbuf[100];
	// 送信バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));
	//serial_<< "r axis" << motor_number << ".encoder.vel_estimate\n";
	sprintf_s(sendbuf, "r axis%d.encoder.vel_estimate\n", motor_number);


	send((char*)sendbuf, sjlen((char*)sendbuf));

	return readFloat();


}

int ODriveArduino::readInt() {
    //return readString().toInt();
	return atoi(readString());
}

bool ODriveArduino::run_state(int axis, int requested_state, bool wait_for_idle, float timeout) {
	
	char sendbuf[100];
	int timeout_ctr = (int)(timeout * 10.0f);



	// 送信バッファクリア
	memset(sendbuf, 0x00, sizeof(sendbuf));
    //serial_ << "w axis" << axis << ".requested_state " << requested_state << '\n';
	sprintf_s(sendbuf, "w axis%d.requested_state %d\n", axis,requested_state);
	send((char*)sendbuf, sjlen((char*)sendbuf));

    if (wait_for_idle) {
        do {
            //delay(100);
			Sleep(100);
			// 送信バッファクリア
			memset(sendbuf, 0x00, sizeof(sendbuf));
            //serial_ << "r axis" << axis << ".current_state\n";
			sprintf_s(sendbuf, "r axis%d.current_state\n", axis);
			send((char*)sendbuf, sjlen((char*)sendbuf));

        } while (readInt() != AXIS_STATE_IDLE && --timeout_ctr > 0);
    }

    return timeout_ctr > 0;
}

CString ODriveArduino::readString() {
    CString str = "";
	char cbuf;
	
	//static const unsigned long timeout = 1000;
    //unsigned long timeout_start = millis();
    for (;;) {
    //    while (!serial_.available()) {
    //        if (millis() - timeout_start >= timeout) {
    //            return str;
    //        }
    //    }
		receive(&cbuf, 1);
        if (cbuf == '\n')
            break;
        str += cbuf;
    }
    

	
	return str;
}



//---------------------------------------------------------------------------
// Futabaのsampleから拝借
bool ODriveArduino::init(char* comport_in, int baudrate)
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
bool ODriveArduino::close(void)
{
	if (flag_opened != 1)return false;

	if (hComm) {
		CloseHandle(hComm);
	}

	return 1;
	//return CloseHandle(hComm);

}
//---------------------------------------------------------------------------
int ODriveArduino::receive(char* buf_ptr, int size)
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
int ODriveArduino::send(char* buf_ptr, int size)
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
int ODriveArduino::CommClose()
{
	if (hComm) {
		CloseHandle(hComm);
	}

	return 1;
}


int sjlen(const char* str)
{
	int count = 0;  // 文字数のカウント用
	int skip = 0;  // skip=1の場合は文字カウントをスキップする
	while (*str != '\0') {
		if (skip) {  // 2バイト文字の2バイト目の場合はカウントしない
			skip = 0;
		}
		else {
			if ((*str & 0xE0) == 0x80 || (*str & 0xE0) == 0xE0) { skip = 1; } //2バイト文字に該当する場合
			count++;
		}
		str++;
	}
	return count;
}
