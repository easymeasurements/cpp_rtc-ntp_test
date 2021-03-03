#include "stdio.h"
#include <string.h>
#include <time.h>
#include "func_rtc3231.h"
// *** constructor **************************************************************
func_rtc3231::func_rtc3231(uint32_t _baud_rate)
// ******************************************************************************
{
	//set_addr(DS3231_CONTROL_ADDR, DS3231_INTCN);
    baud_rate = _baud_rate;
    bcm2835_i2c_set_baudrate(_baud_rate);
}
// *** destructor ***************************************************************
func_rtc3231::~func_rtc3231(void)
// ******************************************************************************
{

}

// ******************************************************************************
uint8_t func_rtc3231::i2c_write(uint8_t _addr_sl, uint8_t _addr, char *_data, uint8_t _len )
// ******************************************************************************
{
    //char aaddr[1] = {(char)_addr};
    char data_send[16];
    data_send[0] = _addr;
    memcpy(data_send+1, _data, _len);
    bcm2835_i2c_begin();
    bcm2835_i2c_setSlaveAddress(_addr_sl);
    bcm2835_i2c_write(data_send, (uint32_t)_len+1);
    bcm2835_i2c_end();
    //---
    return 0;
}
// ******************************************************************************
uint8_t func_rtc3231::i2c_read(uint8_t _addr_sl, uint8_t _addr, char* _data, uint8_t _len )
// ******************************************************************************
{   
    //char aaddr[1] = {(char)_addr};
    char data_send[16];
    data_send[0] = _addr;
    memcpy(data_send+1, _data, _len);
    bcm2835_i2c_begin();
    bcm2835_i2c_setSlaveAddress(_addr_sl);
    bcm2835_i2c_write(data_send, 1);
    bcm2835_i2c_read(_data, _len);
    bcm2835_i2c_end();
    //---
    return 0;
}
//--- public ---
// ******************************************************************************
uint8_t func_rtc3231::rtc_read()
// ******************************************************************************
{
    uint8_t TimeDate[7];        //second,minute,hour,dow,day,month,year
    uint8_t century = 0;
    uint8_t i, n;
    uint16_t year_full;
    char data[8];
	//---
    //printf("_debug::rtc_read0:%x\n", this);
    i2c_read(DS3231_I2C_ADDR, DS3231_TIME_CAL_ADDR, data, 7);
    //printf("_debug::rtc_read1:%x\n", this);
	//---
    for (i = 0; i < 7; i++) {
        n = data[i];
        if (i == 5) {
            TimeDate[5] = bcdtodec(n & 0x1F);
            century = (n & 0x80) >> 7;
        } else
            TimeDate[i] = bcdtodec(n);
    }
	//---
    if (century == 1) {
        year_full = 2000 + TimeDate[6];
    } else {
        year_full = 1900 + TimeDate[6];
    }
    rtc_time.sec = TimeDate[0];
    rtc_time.min = TimeDate[1];
    rtc_time.hour = TimeDate[2];
    rtc_time.mday = TimeDate[4];
    rtc_time.mon = TimeDate[5];
    rtc_time.year = year_full;
    rtc_time.wday = TimeDate[3];
    rtc_time.year_s = TimeDate[6];
    rtc_time.unixtime = get_unixtime(t);
	//---
	//printf("%04d/%02d/%02d %02d:%02d:%02d(%d)\n", t.year, t.mon, t.mday, t.hour, t.min, t.sec, t.unixtime);
    //printf("_debug::rtc_read2:%x\n", this);
  	return 0;
}
// ******************************************************************************
uint8_t func_rtc3231::rtc_set(uint16_t _year, uint8_t _month, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec, uint8_t _wday)
// ******************************************************************************
{
	t.year = _year;
	t.mon = _month;
	t.mday = _day;
	t.wday = _wday;
	t.hour = _hour;
	t.min = _min;
	t.sec = _sec;
	//---
    uint8_t i, century;
	//---
    if (t.year > 1999) {
        century = 0x80;
        t.year_s = t.year - 2000;
    } else {
        century = 0;
        t.year_s = t.year - 1900;
    }
	//---
    uint8_t TimeDate[7] = { t.sec, t.min, t.hour, t.wday, t.mday, t.mon, t.year_s };
	//---
    for (i = 0; i < 7; i++) {
        TimeDate[i] = dectobcd(TimeDate[i]);
        if (i == 5)
            TimeDate[5] += century;
    }
    i2c_write(DS3231_I2C_ADDR, DS3231_TIME_CAL_ADDR, (char *)&TimeDate, 7);
  	return 0;
}
// ******************************************************************************
uint8_t func_rtc3231::rtc_alarm1_cycle_set(int _type_cycle, struct ts _trig, uint8_t * _flags)
// ******************************************************************************
{
    //---
    uint8_t * flags;
    //--- 秒でalarm ---
	uint8_t flag_sec[5] = { 0, 1, 1, 1, 0 };
	//--- 分、秒でalarm ---
	uint8_t flag_min[5] = { 0, 0, 1, 1, 0 };
	//--- 時、分、秒でalarm ---
	uint8_t flag_hour[5] = { 0, 0, 0, 1, 0 };
	//--- 日、時、分、秒でalarm ---
	uint8_t flag_day[5] = { 0, 0, 0, 0, 0 };
    //---
	switch (_type_cycle)
	{
		case CYC_OFF:
			set_addr(DS3231_CONTROL_ADDR, 0);
			break;
		case CYC_SEC:
            flags = flag_sec;
			break;
			break;
		case CYC_MIN:
            flags = flag_min;
			break;
			break;
		case CYC_HOUR:
            flags = flag_hour;
			break;
			break;
		case CYC_DAY:
            flags = flag_day;
			break;
		default:
			break;
	}
    set_a1(_trig.sec, _trig.min, _trig.hour, _trig.mday, flags);
    get_a1(alarm_t, alarm_f, alarm_n);
    set_addr(DS3231_STATUS_ADDR, 0);
    //printf("cyc_n:%03d %03d %03d %03d\n", alarm_n[0], alarm_n[1], alarm_n[2], alarm_n[3]);
    //printf("cyc_t:%03d %03d %03d %03d\n", alarm_t[0], alarm_t[1], alarm_t[2], alarm_t[3]);
    //printf("cyc_f:%03d %03d %03d %03d %03d\n", alarm_f[0], alarm_f[1], alarm_f[2], alarm_f[3], alarm_f[4]);
    set_addr(DS3231_CONTROL_ADDR, DS3231_INTCN | DS3231_A1IE);
	return 0;
}
// ******************************************************************************
uint8_t func_rtc3231::rtc_alarm1_check(void)
// ******************************************************************************
{
    //printf("_debug::rtc_alarm_check0:%x\n", this);
    uint8_t alarm_state = get_addr(DS3231_STATUS_ADDR) & 0x01;
    if(alarm_state!=0)
    {
        set_addr(DS3231_STATUS_ADDR, 0);
    }
    //printf("_debug::rtc_alarm_check1:%x\n", this);
    return alarm_state;
}
// ******************************************************************************
void func_rtc3231::set_addr(uint8_t _addr, uint8_t _val)
// ******************************************************************************
{
    i2c_write(DS3231_I2C_ADDR, _addr, (char *)&_val, 1);
}
// ******************************************************************************
uint8_t func_rtc3231::get_addr(uint8_t _addr)
// ******************************************************************************
{
    //printf("_debug::get_addr0:%x\n", this);
    uint8_t rv;
    i2c_read(DS3231_I2C_ADDR, _addr, (char *)&rv, 1);
    //printf("_debug::get_addr1:%x\n", this);
    return rv;
}
// ******************************************************************************
// flags are: A1M1 (seconds), A1M2 (minutes), A1M3 (hour),
// A1M4 (day) 0 to enable, 1 to disable, DY/DT (dayofweek == 1/dayofmonth == 0)
void func_rtc3231::set_a1(uint8_t _s, uint8_t _mi, uint8_t _h, uint8_t _d, uint8_t * _flags)
// ******************************************************************************
{
    uint8_t t[4] = { _s, _mi, _h, _d };
    uint8_t i;
    uint8_t data[8];
	//---q
    for (i = 0; i < 4; i++) {
        if (i == 3) {
            data[i] = dectobcd(t[3]) | (_flags[3] << 7) | (_flags[4] << 6);
            //Wire.write(dectobcd(t[3]) | (_flags[3] << 7) | (_flags[4] << 6));
        } else
            data[i] = dectobcd(t[i]) | (_flags[i] << 7);
            //Wire.write(dectobcd(t[i]) | (_flags[i] << 7));
    }
    i2c_write(DS3231_I2C_ADDR, DS3231_ALARM1_ADDR, (char *)&data, 4);
}
// ******************************************************************************
void func_rtc3231::get_a1(uint8_t * _t, uint8_t * _f, uint8_t * _n)
// ******************************************************************************
{
    uint8_t i;
    uint8_t data[8];
	//---
    i2c_read(DS3231_I2C_ADDR, DS3231_ALARM1_ADDR, (char *)&data, 4);
	//---
    for (i = 0; i < 4; i++) {
        _n[i] = data[i];
        _f[i] = (_n[i] & 0x80) >> 7;
        _t[i] = bcdtodec(_n[i] & 0x7F);
    }
    _f[4] = (_n[3] & 0x40) >> 6;
    _t[3] = bcdtodec(_n[3] & 0x3F);
}
/*
// ******************************************************************************
// flags are: A1M1 (seconds), A1M2 (minutes), A1M3 (hour),
// A1M4 (day) 0 to enable, 1 to disable, DY/DT (dayofweek == 1/dayofmonth == 0)
void func_rtc3231::set_a2(uint8_t mi, uint8_t h, uint8_t d, uint8_t * flags)
// ******************************************************************************
{
    uint8_t _t[3] = { mi, h, d };
    uint8_t i;
	//---
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_ALARM2_ADDR);
	//---
    for (i = 0; i < 3; i++) {
        if (i == 2) {
            Wire.write(dectobcd(_t[2]) | (flags[2] << 7) | (flags[3] << 6));
        } else
            Wire.write(dectobcd(_t[i]) | (flags[i] << 7));
    }
    Wire.endTransmission();
}
// ******************************************************************************
void func_rtc3231::get_a2(uint8_t * _t, uint8_t * _f, uint8_t * _n)
// ******************************************************************************
{
    uint8_t i;
    //---
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_ALARM2_ADDR);
    Wire.endTransmission();
    Wire.requestFrom(DS3231_I2C_ADDR, 3);
    //---
    for (i = 0; i < 3; i++) {
        _n[i] = Wire.read();
        _f[i] = (_n[i] & 0x80) >> 7;
        _t[i] = bcdtodec(_n[i] & 0x7F);
    }
    //---
    _f[3] = (_n[2] & 0x40) >> 6;
    _t[2] = bcdtodec(_n[2] & 0x3F);
}
*/



uint32_t func_rtc3231::get_unixtime(struct ts _t)
{
    uint16_t y;
    y = _t.year - 1600; // cause this is the first year < at 1970 where year % 400 = 0
    return (_t.year - 1970) * 31536000 + (_t.yday - 1 + (y / 4) - (y / 100) + (y / 400) - 89) * 86400 + _t.hour * 3600 + _t.min * 60 + _t.sec;
}

uint8_t func_rtc3231::dectobcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

uint8_t func_rtc3231::bcdtodec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

uint8_t func_rtc3231::inp2toi(char *cmd, const uint16_t seek)
{
    uint8_t rv;
    rv = (cmd[seek] - 48) * 10 + cmd[seek + 1] - 48;
    return rv;
}