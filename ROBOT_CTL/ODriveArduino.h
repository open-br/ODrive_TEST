
// ODriveArduino のベースのソースコードは、odriveroboticsが著作権を保有しています。
// https://github.com/odriverobotics/ODrive/tree/devel/Arduino/ODriveArduino


//#ifndef ODriveArduino_h
//#define ODriveArduino_h
//#include "Arduino.h"

class ODriveArduino {
public:

    char flag_opened;	//comポートが開かれているかどうか
    char comport[16];	//comポート名
    int baudrate;		//ボーレートをここに出力

    enum AxisState_t {
        AXIS_STATE_UNDEFINED = 0,           //<! will fall through to idle
        AXIS_STATE_IDLE = 1,                //<! disable PWM and do nothing
        AXIS_STATE_STARTUP_SEQUENCE = 2, //<! the actual sequence is defined by the config.startup_... flags
        AXIS_STATE_FULL_CALIBRATION_SEQUENCE = 3,   //<! run all calibration procedures, then idle
        AXIS_STATE_MOTOR_CALIBRATION = 4,   //<! run motor calibration
        AXIS_STATE_SENSORLESS_CONTROL = 5,  //<! run sensorless control
        AXIS_STATE_ENCODER_INDEX_SEARCH = 6, //<! run encoder index search
        AXIS_STATE_ENCODER_OFFSET_CALIBRATION = 7, //<! run encoder offset calibration
        AXIS_STATE_CLOSED_LOOP_CONTROL = 8  //<! run closed loop control
    };

    //ODriveArduino(Stream& serial);

    // Commands
    void SetPosition(int motor_number, float position);
    void SetPosition(int motor_number, float position, float velocity_feedforward);
    void SetPosition(int motor_number, float position, float velocity_feedforward, float current_feedforward);
    void SetVelocity(int motor_number, float velocity);
    void SetVelocity(int motor_number, float velocity, float current_feedforward);
    void SetCurrent(int motor_number, float current);
    void TrapezoidalMove(int motor_number, float position);


    void Set_Vel_limit(int motor_number, float Vel_limit);
    void Set_Current_lim(int motor_number, float Current_lim);
    void Set_pos_gain(int motor_number, float pos_gain);
    void Set_vel_gain(int motor_number, float vel_gain);
    void Set_vel_integrator_gain(int motor_number, float vel_integrator_gain);


    void ODriveINIT(int axis);
    void reset();
    void save_conf();

    // Getters
    float GetVelocity(int motor_number);
    float GetPosition(int motor_number);
    float GetCurrent(int motor_number);

    float Get_Vel_limit(int motor_number);
    float Get_Current_lim(int motor_number);
    float Get_pos_gain(int motor_number);
    float Get_vel_gain(int motor_number);
    float Get_integrator_gain(int motor_number);

    
    // General params
    float readFloat();
    int readInt();

    // State helper
    bool run_state(int axis, int requested_state, bool wait_for_idle, float timeout = 10.0f);

    bool init(char* comport_in, int baudrate);
    bool close(void);


private:
    //Stream& serial_;

    HANDLE		hComm = NULL;		// 通信用ハンドル
    int			ret = 0;			// リターン用
    int			c;					// 入力待ち用

    CString readString();

    int receive(char* buf_ptr, int size);
    int send(char* buf_ptr, int size);
    int CommClose();

    // 送信バイト数のカウント
    int sjlen(const char* str);

};

//#endif //ODriveArduino_h

