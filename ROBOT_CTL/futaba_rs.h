

// エラーコード
#define TEMPERATURE_ERROR 0x80		// 温度エラー(トルクオフ)
#define TEMPERATURE_ALARM 0x20		// 温度アラート
#define FLASH_WRITE_ERROR 0x08		// フラッシュ書き込みエラー
#define PROCESSING_ERROR  0x02		// 受信パケット処理エラー
#define TIMEOUT_ERROR     0x0100	// タイムアウトエラー
#define CHECKSUM_ERROR    0x0200	// チェックサムエラー
#define NO_ERROR_RS       0x0000	// エラーなし

struct sv_r {
	int				enable;
	short			angle;
	short			time;
	short			speed;
	short			load;
	short			temperature;
	int				torque;
	int				old_torque;
	int				error;
	short			g_angle;
	short			old_g_angle;
	unsigned short	g_time;
};

// サーボのメモリマップの送受信に使用する構造体の定義
struct frs_t {
	int 			id;				// サーボID

	short			angle;			// 現在の角度 (サーボから受信した値)
	short			time;			// 移動時間 (サーボから受信した値)
	short			speed;			// 角速度 (サーボから受信した値)
	short			load;			// 負荷 (サーボから受信した値)
	short			temperature;	// 温度 (サーボから受信した値)
	char			torque;			// トルクON/OFF (サーボから受信した値)

	short			g_angle;		// 目標角度 (サーボに送信する目標値)
	unsigned short	g_time;			// 目標への移動時間 (サーボに送信する目標値)

	unsigned short	error;			// エラー
};

class futaba_rs
{
	//HANDLE hcom;
	DWORD mask;
	COMMTIMEOUTS ctmo;
	OVERLAPPED o;
	COMMPROP cmp;


private:

	HANDLE		hComm = NULL;		// 通信用ハンドル
	int			ret = 0;			// リターン用
	int			c;					// 入力待ち用

	int receive(char* buf_ptr, int size);//受け取るバッファの場所とサイズ
	int send(char* buf_ptr, int size);//送るバッファの場所とサイズ

	int CommClose();


public:

	char flag_opened;	//comポートが開かれているかどうか
	char comport[16];	//comポート名
	int baudrate;		//ボーレートをここに出力

	bool init(char* comport_in, int baudrate);
	bool close(void);

	void sv_move(int id, short sPos, unsigned short sTime);
	void sv_move_long(sv_r sendDATA[100]);

	void sv_torque(int id, int torque);		// トルクのオンオフ
	int sv_read_torque(int id);

	sv_r sv_read(int id);
	sv_r sv_read2(int id);

};

