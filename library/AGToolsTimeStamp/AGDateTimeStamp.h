#ifndef TIMEstampCLASS_H
#define TIMEstampCLASS_H 1

#include <stdio.h>
#include <string.h>
#include <math.h>
//---------------------------------------------------------------------------
class AGDateTimeStamp
{
public:
  enum e_DayOfWeek {
    MONDAY     = 0,
    TUESDAY    = 1,
    WEDNESDAY  = 2,
    THURSDAY   = 3,
    FRIDAY     = 4,
    SATURDAY   = 5,
    SUNDAY     = 6
  };
  enum e_Lang {
    EN =  0, // English
    DE =  1, // German
    FR =  2, // French
    IT =  3, // Italian
    ES =  4, // Spanish
    NL =  5, // Dutch
    DA =  6, // Danish
    SV =  7, // Swedish
    NO =  8, // Norwegian
    FI =  9, // Finnish
    RU = 10, // Russian
    PL = 11, // Polish
    CS = 12, // Czech
    SK = 13, // Slowakian
    HU = 14, // Hungarian
    ET = 15, // Estonian
    LV = 16, // Latvian
    LT = 17, // Lithuanian
    PT = 18, // Portuguese
    EO = 19  // Esperanto
  };
  AGDateTimeStamp();
  AGDateTimeStamp(int julian, int milliSecondsSinceMidnight = 0, bool sz = false);
  AGDateTimeStamp(int year, int month, int day, int secondsSinceMidnight = 0, int milliSecond = 0, bool sz = false);
  AGDateTimeStamp(int year, int month, int day, int hour, int minute, int second, int milliSecond = 0, bool sz = false);
  AGDateTimeStamp(const AGDateTimeStamp &DT);
  ~AGDateTimeStamp();
  void init(int JulianDay, int secondsSinceMidnight, int milliSecond = 0, bool sz = false);
  void set(int year, int month, int day, int secondsSinceMidnight = 0, int milliSecond = 0);
  void setSZ(bool SummerTime);
  bool getSZ();
  char *getDayOfWeek(e_Lang language);

  void swap(AGDateTimeStamp &A);
  AGDateTimeStamp&      operator=  (const AGDateTimeStamp &A);
  bool                  operator== (const AGDateTimeStamp &A);
  AGDateTimeStamp&      operator+= (const AGDateTimeStamp &A);
  AGDateTimeStamp&      operator+= (const int &mS);
  AGDateTimeStamp&      operator-= (const AGDateTimeStamp &A);
  AGDateTimeStamp&      operator-= (const int &mS);
  const AGDateTimeStamp operator+ (const AGDateTimeStamp &A) const;
  const AGDateTimeStamp operator+ (const int &mS) const;
  const AGDateTimeStamp operator- (const AGDateTimeStamp &A) const;
  const AGDateTimeStamp operator- (const int &mS) const;
  bool                  operator>= (const AGDateTimeStamp &A);
  bool                  operator<= (const AGDateTimeStamp &A);

  void addMilliSeconds(int milliSeconds);
  void addSeconds(int seconds);
  void addDays(int days);
  void adjust();
  void setTime(int secondsSinceMidnight);
  void setTimeMs(int miSecondsSinceMidnight);
  int getJulian();
  int getYear();
  int getMonth();
  int getDay();
  int getHour();
  int getMinute();
  int getSecond();
  int getMilliSecond();
  int getSecondsSinceMidnight();
  int getMilliSecondsSinceMidnight();
  AGDateTimeStamp getLast(e_DayOfWeek day, unsigned int n = 1, bool countFrom0 = true);
  AGDateTimeStamp getNext(e_DayOfWeek day, unsigned int n = 1, bool countFrom0 = true);
  AGDateTimeStamp getEndOfMonth(bool resetTimeTo0 = false);
  void setEndOfMonth(bool resetTimeTo0 = false);
  int JulianToDatum(int julian,int *tag,int *monat,int *jahr);
  int DatumToJulian(int tag,int monat,int jahr);
  int DateCheck(int tag,int monat,int jahr);
  int TagNrMo(int julian);
  int LastDayOfMonth(int Jahr, int Monat);
  int isSchaltJahr(int jahr);
private:
  int julianDay;
  int milliSecondsSinceMidnight;
  bool SZ;
};
#endif
