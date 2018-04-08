/*
NTP
  AGSimpleNTPtime for ESP8266
  This class gets date and time from the argument's ntp server.
  See https://tools.ietf.org/html/rfc5905 for reference.
  https://www.meinberg.de/german/info/ntp-packet.htm
  Author: Armin Gross

*/

#include "AGSimpleNTPESP.h"

//---------------------------------------------------------------------------------------
// AGSimpleNTPtime keeps track of the time by frequently call to an NTP server. It
// containes a time stamp that can be queried by calling the update() method.
// NTPServer: the name of the NTP server.
// TZH and TZM: Hours and Minutes of the time zone relative to GMT.
// frequency: Intervall (minutes) to compare the internal timestamp with the NTP servers time.
//            Depending on the RTC of the ESP, a frequency of 30 to 60 minutes should
//            be ok.
// region: The region is EU ot US, needed for calculation of daylight saving time.
// daylightShift: the amount of minutes that is added to the time when daylight saving is true.
//
AGSimpleNTPtime::AGSimpleNTPtime(String NTPServer, int TZH, int TZM, unsigned int frequency,
                                 Region region, unsigned int daylightShift) {
  IPAddress ServerIP;
  WiFi.hostByName(NTPServer.c_str(), ServerIP);
  init(ServerIP, TZH, TZM, frequency, region, daylightShift);
}
//---------------------------------------------------------------------------------------
// AGSimpleNTPtime keeps track of the time by frequently call to an NTP server. It
// containes a time stamp that can be queried by calling the update() method.
// NTPServer: the IPAddress of the NTP server.
// for all other parameters, see first constructor.
//
AGSimpleNTPtime::AGSimpleNTPtime(IPAddress NTPServer, int TZH, int TZM, unsigned int frequency,
                                 Region region, unsigned int daylightShift) {
  init(NTPServer, TZH, TZM, frequency, region, daylightShift);
}
//---------------------------------------------------------------------------------------
AGSimpleNTPtime::~AGSimpleNTPtime() {
}
//---------------------------------------------------------------------------------------
// initializes the instance with parameters passed by one of the constructors.
//
void AGSimpleNTPtime::init(IPAddress NTPServer, int TZH, int TZM, unsigned int frequency,
                           Region region, unsigned int daylightShift) {
  timeServerIP = NTPServer;
  TZh = TZH;
  TZm = TZM;
  Freq = frequency * 60000; // frequency is tracked in milliseconds
  DLregion = region;
  DLshift = daylightShift;
  udp.begin(4712);

  unsigned int duration;
  // first query of the NTP server
  timeStamp = getTime(500, AGDateTimeStamp(1970,1,1), &duration, &NTPOffset, &NTPDelay);
  if (timeStamp.getYear() == 1900) {
    // first query failed, so retry in 5 seconds within main loop (call to update())
    loopCount = 5 * 1000;
  }
  else loopCount = Freq;
  timeStamp += duration;
  // timeStart queries ESP's internal counter to keep track of the time
  timeStart = millis();
  justify = 0;
  NTPOffset = 0;
  NTPDelay = 0;
  errors = 0;
  failed = false;
}
//---------------------------------------------------------------------------------------
bool AGSimpleNTPtime::isFailed() {
  return failed;
}
//---------------------------------------------------------------------------------------
void AGSimpleNTPtime::resetFailed() {
  failed = false;
}
//---------------------------------------------------------------------------------------
// set a new frequency in minutes. The new frequency is active starting at the next
// intervall. For immediate activation, call update(true) once(!).
//
void AGSimpleNTPtime::setFrequency(unsigned int frequency) {
  Freq = frequency * 60000;
}
//------------------------------------------------------------------
IPAddress AGSimpleNTPtime::getServerIP() {
  return timeServerIP;
}
//------------------------------------------------------------------
//  send an NTP request to the time server at the given address
//
void AGSimpleNTPtime::sendNTPMessage(IPAddress& address) {
  // reset all values of ntp_message to 0
  memset(&ntp_message, 0, sizeof(ntp_message));
  // Initialize values needed to form NTP request
  // Leap = bits 7 6 = 0b11 = 3 = unknown
  // Version = bits 5 4 3 = 0b100 = 4 = Version 4
  // Mode = bits 2 1 0 = 0b011 = 3 = client
  ntp_message.lvm         = 0b11100011;
  // Stratum, 0 = unspecified
  ntp_message.stratum     = 0;
  // Poll: Maximum intervall = 6
  ntp_message.poll        = 6;
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write((uint8_t*)&ntp_message, sizeof(ntp_message));
  udp.endPacket();
}
//---------------------------------------------------------------------------------------
// getNTPTime() returns AGDateTimeStamp if the NTP server's transmit time.
// if the year of the transmit time is 1970, the function fails because the
// reply of the NTP server was not received within the timout. The timeout
// value is calculated by the caller to guarantee that the function returns
// in time to update the display of the next second.
//
AGDateTimeStamp AGSimpleNTPtime::getNTPTime(int timeout, AGDateTimeStamp origin, unsigned int *duration, int *NTPOffset, int *NTPDelay) {
  int delayTime = 50;
  unsigned int timeStart = millis();
  AGDateTimeStamp receive;
  AGDateTimeStamp transmit;
  // don't try if the timeout is smaller than delayTime
  if (timeout >= delayTime) {
    // send an NTP packet to a time server
    sendNTPMessage(timeServerIP);
    // try to receive the answer
    int cb;
    while (true) {
      cb = udp.parsePacket();
      if (!cb) {
        timeout -= delayTime;
        if (timeout < 0) break;
        delay(delayTime);
      }
      else break;
    }
    if (cb) {
      // read the NTS server's reply
      udp.read((uint8_t*)&ntp_message, sizeof(ntp_message));
      // the timestamp where the ntp server receives the query
      receive = getAGDateTimeStampFromNTPTimeStamp(ntp_message.receive);
      // the timestamp where the ntp server transmits the reply
      transmit = getAGDateTimeStampFromNTPTimeStamp(ntp_message.transmit);
    }
  }
  // calculate processing time for ntp query since sending the query packet
  unsigned int timeEnd = millis();
  *duration = timeEnd - timeStart;
  *NTPDelay = -1;
  *NTPOffset = 0;
  if ((origin.getYear() != 1970) && (transmit.getYear() != 1970)) {
    AGDateTimeStamp destination = origin + *duration;
    AGDateTimeStamp offsetTS = (receive - origin) + (transmit - destination);
    *NTPOffset = (offsetTS.getJulian() * 86400000 + offsetTS.getMilliSecondsSinceMidnight()) / 2;
    AGDateTimeStamp delayTS = (destination - origin) - (transmit - receive);
    *NTPDelay = delayTS.getJulian() * 86400000 + delayTS.getMilliSecondsSinceMidnight();
//    printf("     offset: %d\n", *NTPOffset);
//    printf("      delay: %d\n", *NTPDelay);
    transmit += (*NTPDelay / 2);
  }
  return transmit;
}
//---------------------------------------------------------------------------------------
// getTime() returns AGDateTimeStamp including summer time correction
//
AGDateTimeStamp AGSimpleNTPtime::getTime(int timeout, AGDateTimeStamp TimeStampNow, unsigned int *duration, int *NTPOffset, int *NTPDelay) {
  // while summer time, subtract 1 hour to process correct NTP calculations
  if ((SZStart <= TimeStampNow) && (SZEnd >= TimeStampNow)) {
    TimeStampNow.addSeconds(-3600);
  }
  // subtract time zone to process correct NTP calculations
  TimeStampNow.addSeconds(-(TZm*60 + TZh*3600));
  // Query time
  AGDateTimeStamp NTPTime = getNTPTime(timeout, TimeStampNow, duration, NTPOffset, NTPDelay);
  // check if time is valid
  if (NTPTime.getYear() > 1970) {
    // add time zone
    NTPTime += (TZm*60000 + TZh*3600000);
    // if a new year is reached, calculate start and end of summer time.
    if (SZStart.getYear() != NTPTime.getYear()) {
      switch (DLregion) {
        case (EU) :
          // Summertime for Western Europe
          // start is at last sunday in march 2am
          SZStart.set(NTPTime.getYear(), 3, 31, 2. * 3600);
          SZStart = SZStart.getLast(AGDateTimeStamp::SUNDAY, 0, true);
          // end is at last sunday in october, 3am
          SZEnd.set(NTPTime.getYear(), 10, 31, 3. * 3600);
          SZEnd = SZEnd.getLast(AGDateTimeStamp::SUNDAY, 0, true);
          break;
        case (US) :
          // Summertime (daylight saving time) for US
          // start is at second sunday in march 2am
          SZStart.set(NTPTime.getYear(), 3, 1, 2. * 3600);
          SZStart = SZStart.getNext(AGDateTimeStamp::SUNDAY, 1, true);
          // end is at first sunday in november, 3am
          SZEnd.set(NTPTime.getYear(), 11, 1, 3. * 3600);
          SZEnd = SZEnd.getNext(AGDateTimeStamp::SUNDAY, 0, true);
          break;
        default :   
          // no daylight time;
          SZStart.set(1900, 1, 1, 0);
          SZEnd.set(1900, 1, 1, 0);
          break;
      }
    }
    // while summer time, add 1 hour
    if ((SZStart <= NTPTime) && (SZEnd >= NTPTime)) {
      NTPTime.addSeconds(DLshift * 60);
      NTPTime.setSZ(true);
    }
    else {
      NTPTime.setSZ(false);
    }
  }
  return NTPTime;
}
//---------------------------------------------------------------------------------------
// update() sets the clock forward and frequently compares with NTP time.
// update() must be called at small intervalls << 1 second (tested with 50ms intervalls,
// that is calling update() 20 times per second).
// It calculates the time passed since the last call and adds that time to the tima stamp.
// At defined intervalls it compares with NTP time and fastens or slows the
// advance of the timestamp if there is a difference, except the difference
// is larger than 5 seconds. then the time stamp is reset to the NTP time.
// If you do not want to wait for the next intervall to get NTP time, set
// forceUpdate = true. This is only needed for testing.
// See the frequency parameter in destructor call.
//
AGDateTimeStamp AGSimpleNTPtime::update(bool *forceUpdate) {
  // get the current RTC counter of the ESP
  timeEnd = millis();
  // calculate the milliseconds passed since last call to millis().
  uint32_t mSecsAdd = timeEnd - timeStart;
  // set timeStart to the last millis() result.
  timeStart = timeEnd;
  // add the passed millis and justification to the time stamp.
  timeStamp += (mSecsAdd - justify);
  // add justification to calculated NTP Offest to see when the time stamp should be correct.
  NTPOffset += justify;
  // if NTPoffset is 0, justification can stop
  if (NTPOffset == 0) justify = 0;
  // decrease loop-counter. when it reaches 0, the NTP server is queried.
  loopCount -= mSecsAdd;
  // when switch to summer time we will force a new query to get things right
  if ((SZStart <= timeStamp) && (SZEnd >= timeStamp) && !timeStamp.getSZ()) {
    loopCount = 0;
  }
  // when switch to normal time we will force a new query to get things right
  if (!((SZStart <= timeStamp) && (SZEnd >= timeStamp)) && timeStamp.getSZ()) {
    loopCount = 0;
  }
  // Query NTP server when:
  // no more than 5 errors in sequence happened AND
  // loop count < 5 OR forceUpdate is true.
  if ((!failed) && (loopCount <= 0 || *forceUpdate)) {
    *forceUpdate = false;
    unsigned int duration;
    // calculate the timeout for the NTP query. The timeout is the time in milliseconds from now,
    // when the user display's second-digits must be updated. This is to avoid ugly jumps of
    // the second-digits at the display.
    int timeout = 1000 - timeStamp.getMilliSecond();
    // get the current time from the NTP server, corrected by time zone and daylight saving.
    AGDateTimeStamp newTimeStamp = getTime(timeout, timeStamp, &duration, &NTPOffset, &NTPDelay);
    // if the resulting time stamp is not 1900, the query succeeded.
    if (newTimeStamp.getYear() != 1900) {
      // reset the loopCount to the frequency passed to the constuctor or setFrequency() mathod.
      loopCount = Freq;
      // reset error counter.
      errors = 0;
      // optional: print local timestamp and ntp timestamp.
      printf("cur Timestamp  %d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%3.3d\n", timeStamp.getYear(),
                 timeStamp.getMonth(), timeStamp.getDay(), timeStamp.getHour(),
                 timeStamp.getMinute(), timeStamp.getSecond(), timeStamp.getMilliSecond());
      printf("ntp Timestamp  %d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%3.3d\n", newTimeStamp.getYear(),
                 newTimeStamp.getMonth(), newTimeStamp.getDay(), newTimeStamp.getHour(),
                 newTimeStamp.getMinute(), newTimeStamp.getSecond(), newTimeStamp.getMilliSecond());
      // add the duration of the call to NTP to the time (experimental)
      timeStamp += duration;
      // if the offset between NTP and local timestamp is larger than 5 seconds:
      if (abs(NTPOffset) > 5000) {
        // this will become true at least at switch to or from daylight saving
        timeStamp = newTimeStamp;
      }
      else {
        // if the offset of the NTP server is not 0, adjust the clock by 1 ms per update call.
        // If you call update() 20 times a second, this adds/subtracts 20ms to the time within a second.
        // if you call update() less frequently, increase the below justification values to -2 and +2.
        if (NTPOffset > 0) justify = -1;
        else if (NTPOffset < 0) justify = +1;
        else justify = 0;
      }
    }
    else {
      printf("Error\n");
      errors ++;
      if (errors > 5) failed = true;
      // retry in 5 seconds;
      loopCount = 5 * 1000;
    }
  }
  return timeStamp;
}
//---------------------------------------------------------------------------------------
// getSZStart() returns start of summer time
//
AGDateTimeStamp AGSimpleNTPtime::getSZStart() {
  return SZStart;
}
//---------------------------------------------------------------------------------------
// getSZEnd() returns end of summer time
//
AGDateTimeStamp AGSimpleNTPtime::getSZEnd() {
  return SZEnd;
}
//---------------------------------------------------------------------------------------
// Convert AGDateTimeStamp to a NTP Time Stamp containing Seconds since 1900-1-1 and Fraction
//
AGSimpleNTPtime::s_ntpTimeStamp AGSimpleNTPtime::getNTPTimeStampFromAGDateTimeStamp(AGDateTimeStamp TimeStamp) {
  s_ntpTimeStamp NTPStamp;
  AGDateTimeStamp Epoch1900(1900, 1, 1);
  unsigned long Seconds = (unsigned long)(TimeStamp.getJulian() - Epoch1900.getJulian()) * 86400 + TimeStamp.getMilliSecondsSinceMidnight() / 1000;
  unsigned long MilliSeconds = TimeStamp.getMilliSecond();
  NTPStamp = getNTPTimeStamp(Seconds, MilliSeconds);
  return NTPStamp;
}
//---------------------------------------------------------------------------------------
// Convert NTP Time Stamp to an AGDateTimeStamp
//
AGDateTimeStamp AGSimpleNTPtime::getAGDateTimeStampFromNTPTimeStamp(s_ntpTimeStamp NTPStamp) {
  AGDateTimeStamp TimeStamp(1970, 1, 1);
  unsigned long Seconds;
  unsigned long MilliSeconds;
  getSecondsMillis(NTPStamp, &Seconds, &MilliSeconds);
  Seconds -= 2208988800UL;
  TimeStamp.addSeconds(Seconds);
  TimeStamp += MilliSeconds;
  return TimeStamp;
}
//---------------------------------------------------------------------------------------
// create a NTP Time Stamp from Seconds and Milliseconds
//
AGSimpleNTPtime::s_ntpTimeStamp AGSimpleNTPtime::getNTPTimeStamp(unsigned long Seconds, unsigned long MilliSeconds) {
  s_ntpTimeStamp NTPStamp;
  unsigned long Fraction;
  Fraction = (int)(((double)MilliSeconds / 1000.) * (double)0xFFFFFFFFL);
  NTPStamp.Second = HTONL(Seconds);
  NTPStamp.Fraction = HTONL(Fraction);
  return NTPStamp;
}
//---------------------------------------------------------------------------------------
// Read Seconds and Milliseconds from a NTP Time Stamp  containing Seconds since 1900-1-1 and Fraction
//
void AGSimpleNTPtime::getSecondsMillis(s_ntpTimeStamp NTPStamp, unsigned long *Seconds, unsigned long *MilliSeconds) {
  unsigned long Fraction;
  *Seconds = NTOHL(NTPStamp.Second);
  Fraction = NTOHL(NTPStamp.Fraction);
  *MilliSeconds = (int)(((double)Fraction / (double)0xFFFFFFFFL) * 1000.);
}
//---------------------------------------------------------------------------------------
