#include "stdio.h"
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include "unistd.h"
#include "func_rtc3231.h"
#include "func_ntp.h"
int main()
{
	//---
	struct termios save_settings;
	struct termios settings;
	char c;
	//--- 
	tcgetattr(0,&save_settings);
	settings = save_settings;
	//---
	//settings.c_lflag &= ~(ECHO|ICANON);  /* 入力をエコーバックしない、バッファリングしない */
	settings.c_cc[VTIME] = 0;
	settings.c_cc[VMIN] = 1;
	tcsetattr(0,TCSANOW,&settings);
	fcntl(0,F_SETFL,O_NONBLOCK);	/* 標準入力からの読み込むときブロックしないようにする */
	//--- ntp_section ---
	char cmd[32]="", cmd_no_ntp[] = "T202103022180000";
	strcat(cmd, cmd_no_ntp);
	func_ntp* ntp_cl;
	ntp_cl = new func_ntp();
	if(ntp_cl->ntpdate("time.asia.apple.com")==0)
	{
		sprintf(cmd, "T%04d%02d%02d%01d%02d%02d%02d",ntp_cl->ntp_time.year, ntp_cl->ntp_time.mon, ntp_cl->ntp_time.mday
			  ,ntp_cl->ntp_time.wday ,ntp_cl->ntp_time.hour ,ntp_cl->ntp_time.min ,ntp_cl->ntp_time.sec);
		printf("T%04d%02d%02d%01d%02d%02d%02d\n",ntp_cl->ntp_time.year, ntp_cl->ntp_time.mon, ntp_cl->ntp_time.mday ,ntp_cl->ntp_time.wday ,ntp_cl->ntp_time.hour ,ntp_cl->ntp_time.min ,ntp_cl->ntp_time.sec);
		//printf("%d\n", 109);
	}
	//--- i2c_section ---
	struct ts t;
	//--- bcm2835ライブラリの初期化 ---
    if (! bcm2835_init()) {
        printf("BCM2835 Library initialization failed. Exiting.");
        return(-1);
    }
	//--- I2Cの初期化＆コンストラクト ---
	func_rtc3231* rtc_cl;
	rtc_cl = new func_rtc3231(400000);
	//--- RTC時刻の読込 ---
	rtc_cl->rtc_read();
	printf("rtc_set:%s", cmd);
	t.year = rtc_cl->inp2toi(cmd, 1) * 100 + rtc_cl->inp2toi(cmd, 3);
	t.mon = rtc_cl->inp2toi(cmd, 5);
	t.mday = rtc_cl->inp2toi(cmd, 7);
	t.wday = cmd[9] - 48;
	t.hour = rtc_cl->inp2toi(cmd, 10);
	t.min = rtc_cl->inp2toi(cmd, 12);
	t.sec = rtc_cl->inp2toi(cmd, 14);
	//---
	rtc_cl->rtc_set(t.year, t.mon, t.mday, t.hour, t.min, t.sec, t.wday);
	printf("OK\n");
	//---
	//--- Uhhmmss U27101520
	char cmd2[] = "U04023600";
	t.mday = rtc_cl->inp2toi(cmd2, 1);
	t.hour = rtc_cl->inp2toi(cmd2, 3);
	t.min = rtc_cl->inp2toi(cmd2, 5);
	t.sec = rtc_cl->inp2toi(cmd2, 7);
	//--- 秒でalarm ---
	//uint8_t flags[5] = { 0, 1, 1, 1, 0 };
	//--- 分、秒でalarm ---
	uint8_t flags[5] = { 0, 0, 1, 1, 0 };
	//--- 時、分、秒でalarm ---
	//uint8_t flags[5] = { 0, 0, 0, 1, 0 };
	//--- 日、時、分、秒でalarm ---
	//uint8_t flags[5] = { 0, 0, 0, 0, 0 };
	rtc_cl->rtc_alarm1_cycle_set(CYC_MIN, t, flags);
	//---
	char input_str[16];
	uint8_t input_len=0;
	uint8_t exit_flag =1;
	while(exit_flag)
	{
		/*--------------------------------------------------
		c = getchar()を含めると、以下エラーが発生する模様。
		Program received signal SIGSEGV, Segmentation fault.
		0x00011888 in func_rtc3231::rtc_read (this=0xffffff) at func_rtc3231.cpp:80
		80          rtc_time.sec = TimeDate[0];
		(gdb) bt
		#0  0x00011888 in func_rtc3231::rtc_read (this=0xffffff) at func_rtc3231.cpp:80
		#1  0x0001149c in main () at rtc_test.cpp:111
		(gdb) 
		---------------------------------------------------*/
		//printf("_debug::main0:%08x\n", rtc_cl);
		c=getchar();
		//printf("_debug::main1:%08x\n", rtc_cl);
		//*
		switch(c)
		{
			case 0x0a:
				input_str[input_len] = 0x00;
				printf("input_com:%s\n",input_str);
				//---
				t.mday = rtc_cl->inp2toi(input_str, 1);
				t.hour = rtc_cl->inp2toi(input_str, 3);
				t.min = rtc_cl->inp2toi(input_str, 5);
				t.sec = rtc_cl->inp2toi(input_str, 7);
				//---
				rtc_cl->rtc_alarm1_cycle_set(CYC_MIN, t, flags);
				break;
			case '1':
				printf("1\n");
				break;
			case 'q':
				printf("quit\n");
				exit_flag =0;
				break;
			default:
				input_str[input_len] = c;
				input_len++;
				break;
		}
		//*/
		usleep(1000000);	//0.5s
		rtc_cl->rtc_read();
		//printf("_debug::main2:%08x\n", rtc_cl);
		if(rtc_cl->rtc_alarm1_check()==1)
			printf("rtc_alarm1 interrupt occurd\n");
		//---
		printf("%04d/%02d/%02d %02d:%02d:%02d(%d)\n", rtc_cl->rtc_time.year, rtc_cl->rtc_time.mon, rtc_cl->rtc_time.mday, rtc_cl->rtc_time.hour, rtc_cl->rtc_time.min, rtc_cl->rtc_time.sec, rtc_cl->rtc_time.unixtime);
	}
	//--- デストラクト ---
	tcsetattr(0,TCSANOW,&save_settings);
	delete rtc_cl;
	//--- i2ctools用に再度開く
	bcm2835_i2c_begin();
	//---
	bcm2835_close();
	//---
	return 0;
}


