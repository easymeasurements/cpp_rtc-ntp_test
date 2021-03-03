#include "func_ntp.h"
// *** constructor **************************************************************
func_ntp::func_ntp(void)
// ******************************************************************************
{
	//set_addr(DS3231_CONTROL_ADDR, DS3231_INTCN);
}
// *** destructor ***************************************************************
func_ntp::~func_ntp(void)
// ******************************************************************************
{

}
// ******************************************************************************
void func_ntp::send_packet(int usd)
// ******************************************************************************
{
    uint32_t data[12];
    struct timeval now;
    static const int LI = 0, VN = 3, MODE = 3, STRATUM = 0, POLL = 4, PREC = -6;
    bzero((char *) data,sizeof(data));
    data[0] = htonl ((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
    data[1] = htonl(1<<16); // Root Delay (seconds)
    data[2] = htonl(1<<16); // Root Dispersion (seconds)
    gettimeofday(&now,NULL);
    data[10] = htonl(now.tv_sec + JAN_1970); // Transmit Timestamp coarse
    data[11] = htonl(NTPFRAC(now.tv_usec));  // Transmit Timestamp fine
    send(usd,data,48,0);
}
// ******************************************************************************
void func_ntp::rfc1305(uint32_t *data)
// ******************************************************************************
{
    time_t sec = ntohl(((uint32_t *)data)[10]) - JAN_1970;
    struct tm ttm;
    localtime_r(&sec,&ttm);
    printf("GMT: %04d-%02d-%02d %02d:%02d:%02d +0900\n",ttm.tm_year+1900,ttm.tm_mon+1,ttm.tm_mday,ttm.tm_hour,ttm.tm_min,ttm.tm_sec);
    //---
    ntp_time.year = ttm.tm_year+1900;
    ntp_time.mon  = ttm.tm_mon+1;
    ntp_time.mday = ttm.tm_mday;
    ntp_time.hour= ttm.tm_hour;
    ntp_time.min = ttm.tm_min;
    ntp_time.sec = ttm.tm_sec;
    ntp_time.wday = ttm.tm_wday;
    ntp_time.unixtime = sec + JAN_1970;
}
// ******************************************************************************
int func_ntp::ntpdate(const char* server)
// ******************************************************************************
{
    int usd;
    struct hostent *he;
    if ( (usd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ) { perror("socket"); return -1; }
    if ( (he = gethostbyname(server)) == NULL ) return -1;

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    memcpy(&sa.sin_addr, he->h_addr_list[0], sizeof(sa.sin_addr));
    sa.sin_port = htons(NTP_PORT);
    sa.sin_family = AF_INET;
    if ( connect(usd, (struct sockaddr*)&sa, sizeof(sa)) == -1 ) return -1;
    // send ntp packet
    send_packet(usd);
    // recv ntp packet
    struct timeval tv = {3,0};
    fd_set fds; FD_ZERO(&fds); FD_SET(usd, &fds);
    if ( select(usd + 1, &fds, NULL, NULL, &tv) == 1 ) {
        uint32_t packet[12];
        int len = recv(usd, packet, sizeof(packet), 0);
        if ( len == sizeof(packet) ) { rfc1305(packet); close(usd); return 0; }
    }
    return -1;
}