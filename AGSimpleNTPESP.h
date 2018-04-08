/*
NTP
  AGSimpleNTPtime for ESP8266
  This class gets date and time from the argument's ntp server

  Author: Armin Gross

*/
#ifndef AGSimpleNTPtime_h
#define AGSimpleNTPtime_h

#include "AGDateTimeStamp.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define NTOHL(n) ((((n & 0xFF))       << 24) | \
                 (((n & 0xFF00))     << 8)  | \
                 (((n & 0xFF0000))   >> 8)  | \
                 (((n & 0xFF000000)) >> 24))
#define HTONL(n) ((((n & 0xFF))       << 24) | \
                 (((n & 0xFF00))     << 8)  | \
                 (((n & 0xFF0000))   >> 8)  | \
                 (((n & 0xFF000000)) >> 24))

//---------------------------------------------------------------------------------------
class AGSimpleNTPtime {
  public:
  struct s_ntpTimeStamp {
    uint32_t Second;
    uint32_t Fraction;
  };
  struct s_ntp_packet {
    uint8_t  lvm;
    uint8_t  stratum;
    uint8_t  poll;
    uint8_t  precision;
    uint32_t rootDelay;
    uint32_t rootDispersion;
    uint32_t referenceId;
    s_ntpTimeStamp reference;
    s_ntpTimeStamp origin;
    s_ntpTimeStamp receive;
    s_ntpTimeStamp transmit;
  };
  enum Region {
    EU = 0,
    US = 1
  };
  AGSimpleNTPtime(String NTPServer, int TZHH, int TZMM, unsigned int frequency,
                  Region region = Region::EU, unsigned int daylightShift = 60);
  AGSimpleNTPtime(IPAddress NTPserver, int TZHH, int TZMM, unsigned int frequency,
                  Region region = Region::EU, unsigned int daylightShift = 60);
  ~AGSimpleNTPtime();
  void setFrequency(unsigned int frequency);
  AGDateTimeStamp getNTPTime(int timeout, AGDateTimeStamp TimeStampNow, unsigned int *duration, int *offset, int *delay);
  AGDateTimeStamp getTime(int timeout, AGDateTimeStamp TimeStampNow, unsigned int *duration, int *offset, int *delay);
  AGDateTimeStamp getSZStart();
  AGDateTimeStamp getSZEnd();
  IPAddress getServerIP();
  AGDateTimeStamp update(bool *forceUpdate);
  bool isFailed(); // returns true if NTP server does not answer 5 times in sequence
  void resetFailed();
private:
  s_ntp_packet ntp_message;
  AGDateTimeStamp timeStamp;
  AGDateTimeStamp SZStart; // without argument, AGDateTimeStamp is initialized to 1970-01-01
  AGDateTimeStamp SZEnd; // without argument, AGDateTimeStamp is initialized to 1970-01-01
  void init(IPAddress NTPserver, int TZHH, int TZMM, unsigned int frequency,
            Region region, unsigned int daylightShift);
  IPAddress timeServerIP;
  void sendNTPMessage(IPAddress& address);
  int TZh, TZm; // Timezone in Hours and Minutes
  unsigned int Freq;
  int loopCount;
  int justify;
  int NTPOffset;
  int NTPDelay;
  int errors;
  bool failed;
  bool summerTime(unsigned long _timeStamp );
  Region DLregion;
  unsigned int DLshift;
  unsigned long adjustTimeZone(unsigned long _timeStamp, float _timeZone, int _DayLightSavingSaving);
  s_ntpTimeStamp getNTPTimeStampFromAGDateTimeStamp(AGDateTimeStamp TimeStamp);
  AGDateTimeStamp getAGDateTimeStampFromNTPTimeStamp(AGSimpleNTPtime::s_ntpTimeStamp NTPStamp);
  s_ntpTimeStamp getNTPTimeStamp(unsigned long Seconds, unsigned long MilliSeconds);
  void getSecondsMillis(s_ntpTimeStamp NTPStamp, unsigned long *Seconds, unsigned long *MilliSeconds);
  WiFiUDP udp;
  uint32_t timeStart, timeEnd;
};
//---------------------------------------------------------------------------------------
#endif
