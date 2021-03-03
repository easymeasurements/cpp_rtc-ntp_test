#ifndef func_ntp_h
#define func_ntp_h
//---
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <netdb.h>
#include "time.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "func_time.h"
//---
#define _BSD_SOURCE
//----------------------------------------------------------------------------------------------------
#define JAN_1970    0x83aa7e80 // 2208988800 1970 - 1900 in seconds
#define NTP_PORT    (123)
#define NTPFRAC(x)  (4294*(x) + ((1981*(x))>>11))
//----------------------------------------------------------------------------------------------------
class func_ntp
{
	//-----------------------------
	public:
        //---
        struct ts ntp_time;
        //---
        func_ntp(void);
        ~func_ntp(void);
        void send_packet(int usd);
        void rfc1305(uint32_t *data);
        int ntpdate(const char* server);
	//-----------------------------
	private:

};
#endif