#ifndef func_rtc3231_h
#define func_rtc3231_h
//---
#include <bcm2835.h>
#include "func_time.h"
//--- define UNIX_TIME ---
// D:\source\program\arduino\libraries\ds3231FS\config.h
//--- 
#define BUFF_MAX 128
#define SECONDS_FROM_1970_TO_2000 946684800
// i2c slave address of the DS3231 chip
#define DS3231_I2C_ADDR             0x68
// timekeeping registers
#define DS3231_TIME_CAL_ADDR        0x00
#define DS3231_ALARM1_ADDR          0x07
#define DS3231_ALARM2_ADDR          0x0B
#define DS3231_CONTROL_ADDR         0x0E
#define DS3231_STATUS_ADDR          0x0F
#define DS3231_AGING_OFFSET_ADDR    0x10
#define DS3231_TEMPERATURE_ADDR     0x11
// control register bits
#define DS3231_A1IE     0x1
#define DS3231_A2IE     0x2
#define DS3231_INTCN    0x4
// status register bits
#define DS3231_A1F      0x1
#define DS3231_A2F      0x2
#define DS3231_OSF      0x80
// ******************************************************************************EM

// ******************************************************************************EM
enum type_cycle
{
	CYC_OFF,
	CYC_DAY,
	CYC_HOUR,
	CYC_MIN,
	CYC_SEC
};
class func_rtc3231
{
	//-----------------------------
	public:
		struct ts rtc_time;
		uint8_t alarm_n[4];				
		uint8_t alarm_t[4];               //second,minute,hour,day
		uint8_t alarm_f[5];               // flags
		//--- function
		func_rtc3231(uint32_t _baud_rate);
		~func_rtc3231(void);
		uint8_t rtc_read();
		uint8_t rtc_set(uint16_t _year, uint8_t _month, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec, uint8_t _wday);
		uint8_t rtc_alarm1_cycle_set(int _type_cycle, struct ts _trig, uint8_t * flags);
		uint8_t rtc_alarm1_check(void);
		//---
		void set_addr(uint8_t _addr, uint8_t _val);
		uint8_t get_addr(uint8_t _addr);
        //---
        uint8_t i2c_write(uint8_t _addr_sl, uint8_t _addr, char* _data, uint8_t _len );
        uint8_t i2c_read(uint8_t _addr_sl, uint8_t _addr, char* _data, uint8_t _len );
		//---
		uint32_t get_unixtime(struct ts _t);
		uint8_t dectobcd(uint8_t _val);
		uint8_t bcdtodec(uint8_t _val);
		uint8_t inp2toi(char *_cmd, const uint16_t _seek);
	private:
		uint32_t baud_rate =100000;
		struct ts t;
		struct ts t_alarm;
		char buff[BUFF_MAX];
		//--- scaduler
		struct ts time_sc[10];
		//--- function
		void set_a1(uint8_t _s, uint8_t _mi, uint8_t _h, uint8_t _d, uint8_t * _flags);
		void get_a1(uint8_t * _t, uint8_t * _f, uint8_t * _n);
		void set_a2(uint8_t _mi, uint8_t _h, uint8_t _d, uint8_t * _flags);
		void get_a2(uint8_t * _t, uint8_t * _f, uint8_t * _n);
		//---
};
#endif
