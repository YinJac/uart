#ifndef __UARTD_TEST_MODE_H__
#define __UARTD_TEST_MODE_H__

enum buttons
{
    BUTTON_WPS_PRESSED = 0,			// WPS按键
	BUTTON_PLAY_PAUSE_PRESSED,		// 暂停/播放按键
    BUTTON_PREV_PRESSED,			// 上一曲按键
    BUTTON_NEXT_PRESSED,			// 下一曲按键
    BUTTON_RECODE_PRESSED,			// 录音按键
    BUTTON_VOLUME_UP_PRESSED,		// 音量加按键
    BUTTON_VOLUME_DOWN_PRESSED,		// 音量减按键
	BUTTON_MODE_SWITCH_PRESSED,		// 模式切换按键
	BUTTON_COLLECTION_PRESSED,		// 收藏按键
	BUTTON_UNCOLLECTION_PRESSED,	// 解除收藏按键
	BUTION_LOOPMODE_PRESSED,		// 循环模式按键
    BUTTON_MUTE_PRESSED,			// 语音对话按键
    BUTTON_MCU1_PRESSED,			// MCU按键1
	BUTTON_MCU2_PRESSED,			// MCU按键2
	BUTTON_MCU3_PRESSED,			// MCU按键3
	BUTTON_MCU4_PRESSED,			// MCU按键4
	BUTTON_MCU5_PRESSED,			// MCU按键5
	BUTTON_MCU6_PRESSED,			// MCU按键6
	BUTTON_MCU7_PRESSED,			// MCU按键7
	BUTTON_MCU8_PRESSED,			// MCU按键8
    BUTTON_MCU9_PRESSED,			// MCU按键9
    BUTTON_RESET_PRESSED,			// 预制键
    BUTTON_GROUP_PRESSED,			// Group键
    RECORD_SUCCEED,					// 录音成功
    RECORD_FAILED,					// 录音失败
	MIC_GAIN_SUCCEED,				// MIC一致性测试成功
	MIC_GAIN_FAILE					// MIC一致性测试失败
};

//static int create_broadcast_socket(char* ifname);
//static int send_broadcast_to_server(char* buffer);

//提供给外部的接口
int send_cmd_to_server(char cmd);


#endif
