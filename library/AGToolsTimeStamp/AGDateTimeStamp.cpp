#include "AGDateTimeStamp.h"

//---------------------------------------------------------------------------
AGDateTimeStamp::AGDateTimeStamp()
{
  init(DatumToJulian(1900, 1, 1), 0);
}
//---------------------------------------------------------------------------
AGDateTimeStamp::AGDateTimeStamp(int julian, int milliSecondsSinceMidnight, bool sz)
{
  init(julian, 0, milliSecondsSinceMidnight, sz);
}
//---------------------------------------------------------------------------
AGDateTimeStamp::AGDateTimeStamp(int year, int month, int day,
                                 int secondsSinceMidnight, int milliSecond, bool sz)
{
  init(DatumToJulian(year, month, day), secondsSinceMidnight, milliSecond, sz);
}
//---------------------------------------------------------------------------
AGDateTimeStamp::AGDateTimeStamp(int year, int month, int day, int hour, int minute, int second, int milliSecond, bool sz)
{
  init(DatumToJulian(year, month, day), hour * 3600 + minute * 60 + second, milliSecond, sz);
}
//---------------------------------------------------------------------------
AGDateTimeStamp::AGDateTimeStamp(const AGDateTimeStamp &DT)
{
  init(DT.julianDay, 0, DT.milliSecondsSinceMidnight, DT.SZ);
}
//---------------------------------------------------------------------------
AGDateTimeStamp::~AGDateTimeStamp()
{
}
//---------------------------------------------------------------------------
void AGDateTimeStamp::init(int JulianDay, int secondsSinceMidnight, int milliSecond, bool sz)
{
  julianDay                 = JulianDay;
  milliSecondsSinceMidnight = secondsSinceMidnight * 1000 + milliSecond;
  SZ = sz;
}
//---------------------------------------------------------------------------
void AGDateTimeStamp::set(int year, int month, int day, int secondsSinceMidnight, int milliSecond)
{
  init(DatumToJulian(year, month, day), secondsSinceMidnight, milliSecond);
}
//---------------------------------------------------------------------------
void AGDateTimeStamp::setSZ(bool SummerTime)
{
  SZ = SummerTime;
}
//---------------------------------------------------------------------------
bool AGDateTimeStamp::getSZ()
{
  return SZ; 
}
//---------------------------------------------------------------------------
// Tausch mit dem AGDateTimeStamp A
//
void AGDateTimeStamp::swap(AGDateTimeStamp &A)
{
  int temp;
  temp = julianDay;
  julianDay = A.julianDay;
  A.julianDay = temp;
  double msTemp;
  msTemp = milliSecondsSinceMidnight;
  milliSecondsSinceMidnight = A.milliSecondsSinceMidnight;
  A.milliSecondsSinceMidnight = msTemp;
  bool szTemp;
  szTemp = SZ;
  SZ = A.SZ;
  A.SZ = szTemp;
}
//---------------------------------------------------------------------------
// Zuweisung der Form a = b
// Reallokiert Speicher falls die Dimension der Zielmatrix ungleich
// der Dimension der Quellmatrix ist
//
AGDateTimeStamp& AGDateTimeStamp::operator=(const AGDateTimeStamp &A)
{
  AGDateTimeStamp Temp(A);
  swap(Temp);

  return *this ;
}
//---------------------------------------------------------------------------
// Vergleich a == b, wobei die Unterschiede den Wert epsilon
// nicht überstein dürfen. Dieser sollte nicht 0.0 sein da
// bei Fließkommaberechnungen immer Ungenauigkeiten entstehen.
//
bool AGDateTimeStamp::operator==(const AGDateTimeStamp &A)
{
  bool rc = true;
  int diffj = fabs(julianDay - A.julianDay);
  double diffs = fabs(milliSecondsSinceMidnight - A.milliSecondsSinceMidnight);
  if ((diffj != 0) || (diffs != 0)) {
    rc = false;
  }
  return rc;
}
//---------------------------------------------------------------------------
// Adds a timestamp to another timestamp. Years and Seconds are added
//
AGDateTimeStamp& AGDateTimeStamp::operator+=(const AGDateTimeStamp &A)
{
  julianDay += A.julianDay;
  milliSecondsSinceMidnight += A.milliSecondsSinceMidnight;
  adjust();
  return *this;
}
//---------------------------------------------------------------------------
// Adds seconds to the timestamp
//
AGDateTimeStamp& AGDateTimeStamp::operator+=(const int &mS)
{
  milliSecondsSinceMidnight += mS;
  adjust();
  return *this;
}
//---------------------------------------------------------------------------
//
const AGDateTimeStamp AGDateTimeStamp::operator+(const AGDateTimeStamp& A) const
{
  AGDateTimeStamp R(*this);
  R += A;
  return R;
}
//---------------------------------------------------------------------------
//
const AGDateTimeStamp AGDateTimeStamp::operator+(const int &mS) const
{
  AGDateTimeStamp R(*this);
  R += mS;
  return R;
}
//---------------------------------------------------------------------------
AGDateTimeStamp& AGDateTimeStamp::operator-=(const AGDateTimeStamp &A)
{
  julianDay -= A.julianDay;
  milliSecondsSinceMidnight -= A.milliSecondsSinceMidnight;
  adjust();
  return *this;
}
//---------------------------------------------------------------------------
AGDateTimeStamp& AGDateTimeStamp::operator-=(const int &mS)
{
  milliSecondsSinceMidnight -= mS;
  adjust();
  return *this;
}
//---------------------------------------------------------------------------
const AGDateTimeStamp AGDateTimeStamp::operator-(const AGDateTimeStamp &A) const
{
  AGDateTimeStamp R(*this);
  R -= A;
  return R;
}
//---------------------------------------------------------------------------
const AGDateTimeStamp AGDateTimeStamp::operator-(const int &mS) const
{
  AGDateTimeStamp R(*this);
  R -= mS;
  return R;
}
//---------------------------------------------------------------------------
bool AGDateTimeStamp::operator>= (const AGDateTimeStamp &A) {
  bool rc = false;
  if (julianDay > A.julianDay) rc = true;
  else if ((julianDay == A.julianDay) && (milliSecondsSinceMidnight >= A.milliSecondsSinceMidnight)) rc = true;
  return rc;
}
//---------------------------------------------------------------------------
bool AGDateTimeStamp::operator<= (const AGDateTimeStamp &A) {
  bool rc = false;
  if (julianDay < A.julianDay) rc = true;
  else if ((julianDay == A.julianDay) && (milliSecondsSinceMidnight <= A.milliSecondsSinceMidnight)) rc = true;
  return rc;
}
//---------------------------------------------------------------------------
// set the time by passing seconds since midnight
void AGDateTimeStamp::setTime(int secondsSinceMidnight)
{
  milliSecondsSinceMidnight = secondsSinceMidnight * 1000;
  adjust();
}
//---------------------------------------------------------------------------
// set the time by passing milliseconds since midnight
void AGDateTimeStamp::setTimeMs(int miSecondsSinceMidnight)
{
  milliSecondsSinceMidnight = miSecondsSinceMidnight;
  adjust();
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getJulian()
{
  return julianDay;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getYear()
{
  int y, m, d;
  if (JulianToDatum(julianDay, &y, &m, &d)) {
    y = 0;
  }
  return y;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getMonth()
{
  int y, m, d;
  if (JulianToDatum(julianDay, &y, &m, &d)) {
    m = 0;
  }
  return m;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getDay()
{
  int y, m, d;
  if (JulianToDatum(julianDay, &y, &m, &d)) {
    d = 0;
  }
  return d;
}
//-----------------------------)----------------------------------------------
int AGDateTimeStamp::getHour() {
  return milliSecondsSinceMidnight / 3600000;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getMinute() {
  return (milliSecondsSinceMidnight / 60000) % 60;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getSecond() {
  return (milliSecondsSinceMidnight / 1000) % 60;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getMilliSecond() {
  return milliSecondsSinceMidnight % 1000;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getSecondsSinceMidnight() {
  return milliSecondsSinceMidnight / 1000;
}
//---------------------------------------------------------------------------
int AGDateTimeStamp::getMilliSecondsSinceMidnight() {
  return milliSecondsSinceMidnight;
}
//---------------------------------------------------------------------------
// add seconds to the current date
void AGDateTimeStamp::addSeconds(int seconds)
{
  int days = seconds / 86400;
  int secs = seconds % 86400;
  julianDay += days;
  milliSecondsSinceMidnight += secs * 1000;
  adjust();
}
//---------------------------------------------------------------------------
// add days to the current date
void AGDateTimeStamp::addDays(int days)
{
  julianDay += days;
}
//---------------------------------------------------------------------------
void AGDateTimeStamp::adjust()
{
  while (milliSecondsSinceMidnight < 0) {
    milliSecondsSinceMidnight += 86400000;
    julianDay--;
  }
  while (milliSecondsSinceMidnight >= 86400000) {
    milliSecondsSinceMidnight -= 86400000;
    julianDay++;
  }
}
//---------------------------------------------------------------------------
// get the first day (Monday) of the week of the current date
//
AGDateTimeStamp AGDateTimeStamp::getEndOfMonth(bool resetTimeTo0)
{
  AGDateTimeStamp result(julianDay, milliSecondsSinceMidnight);
  result.setEndOfMonth(resetTimeTo0);
  return result;
}
//---------------------------------------------------------------------------
// get the first day (Monday) of the week of the current date
//
void AGDateTimeStamp::setEndOfMonth(bool resetTimeTo0)
{
  int y, m, d;
  if (resetTimeTo0) setTime(0.);
  JulianToDatum(julianDay, &y, &m, &d);
  d = LastDayOfMonth(y, m);
  julianDay = DatumToJulian(y, m, d);
}
//---------------------------------------------------------------------------
// calculates the previous named weekday before the actUal day.
// if the actual weekday is the same as the named weekday the
// result is one week back.
// if n > 1 then the 2nd, 3rd etc previous named weekday is calculated.
AGDateTimeStamp AGDateTimeStamp::getLast(e_DayOfWeek day, unsigned int n, bool countFrom0)
{
  AGDateTimeStamp result(julianDay, milliSecondsSinceMidnight);
  int daynum = TagNrMo(julianDay);
//  printf("Daynum = %d\n", daynum);
  int diff = day - daynum;
  if (diff > 0) diff -= 7;
//  printf("Diff = %d\n", diff);
  if (!countFrom0) n++;
  result.addDays(diff - n * 7);

  return result;
}
//---------------------------------------------------------------------------
// calculates the next named weekday before the actial day.
// if the actual weekday is the same as the named weekday the
// result is one week into the future.
// if n > 1 then the 2nd, 3rd etc next named weekday is calculated.
AGDateTimeStamp AGDateTimeStamp::getNext(e_DayOfWeek day, unsigned n, bool countFrom0)
{
  AGDateTimeStamp result(julianDay, milliSecondsSinceMidnight);
  int daynum = TagNrMo(julianDay);
//  printf("Daynum = %d\n", daynum);
  int diff = day - daynum;
  if (diff < 0) diff += 7;
//  printf("Diff = %d\n", diff);
  if (!countFrom0) n++;
  result.addDays(diff + n * 7);

  return result;
}
//---------------------------------------------------------------------------
/*----------------------------------------------------A. Grosss; Nov. 88--
int JulToDatum(julian,&tag,&monat,&jahr)
Berechnet das gregorianische Datum aus dem Julianischen Datum
(Tagesnummer ab 0 (= 1.1.4713 v.Chr.) gltig ab 2299161 = Freitag,
15.10.1582).

Rueckgabe:

0 : alles ok
1 : Tagesnummer ist ungltig

I/O    Typ       Name    Beschreibung
---------------------------------------------------------
I  int      julian     Tagesnummer
O  int       tag     Tag
O  int       monat      Monat
O  int       jahr    Jahr
---------------------------------------------------------
Quelle:   Heinz Zemanek
  Kalender und Chronologie
  4. Auflage
  Oldenbourg Verlag 1987
----------------------------------------------------------------------*/
int AGDateTimeStamp::JulianToDatum(int julian,int *year,int *month,int *day)
{
  int rc;
  int t,m,j;

  rc=0;

  if (julian < 2299161L) {
    rc = 1;
  }
  else {
    julian -= 1721119L;
    j = (4 * julian - 1) / 146097L;
    t = (julian = 4 * julian - 1 - 146097L * j) / 4;
    t = 4 * t + 3 - 1461 * (julian = (4 * t + 3) / 1461);
    t = (t + 4) / 4;
    m = (5 * t - 3) / 153;
    t = 5 * t - 3 - 153 * m;
    t = (t + 5) / 5;
    j = 100 * j + julian;
    if (m < 10)
      m += 3;
    else {
      m -= 9;
      j += 1;
    }
    *day = (int)t;
    *month = (int)m;
    *year = (int)j;
  }
  return rc;
}
/*----------------------------------------------------A. Gross; Nov. 88--
int DatumToJulian(tag,monat,jahr)
Berechnet das Julianische Datum (Tagesnummer ab 1.1.4713 v.Chr.)
fuer alle gueltigen Gregorianischen Daten (ab 15.10.1582).

Rueckgabe:

0 : Eingegebenes Datum ist ungueltig
<> 0 : Julianisches Datum

I/O    Typ       Name    Beschreibung
---------------------------------------------------------
I  int       tag     Tag
I  int       monat      Monat
I  int       jahr    Jahr
---------------------------------------------------------
Quelle:  Heinz Zemanek
        Kalender und Chronologie
        4. Auflage
        Oldenbourg Verlag 1987

----------------------------------------------------------------------*/
int AGDateTimeStamp::DatumToJulian(int year,int month,int day)
{
  int c,ya,julday;
  int check_d,check_m,check_y,mm,yy;

  mm = month;
  yy = year;
  if (month > 2)
    month -= 3;
  else {
    month += 9;
    year--;
  }
  c = year/100;
  ya = year - 100 * c;
  julday = (146097L * c)/4 + (1461 * ya)/4 + (153 * month+2)/5 + day+1721119L;
  JulianToDatum(julday,&check_y,&check_m,&check_d);
  if (day != check_d) julday=0;
  if (mm != check_m) julday=0;
  if (yy != check_y) julday=0;
  if (julday < 2299161L) julday=0;
  return julday;
}

/*----------------------------------------------------A. Gross; Nov. 88--
int DateCheck(tag,monat,jahr)
Prueft, ob das eingegebene Datum(tag,monat,jahr) ein gueltiges

gregorianisches Datum ist (ab 15.10.1582)

Ruckgabe:

0 : alles ok

1 : Datum ungueltig

I/O    Typ       Name    Beschreibung
---------------------------------------------------------
I  int       tag     Tag

I  int       monat      Monat
I  int       jahr    Jahr
---------------------------------------------------------
----------------------------------------------------------------------*/
int AGDateTimeStamp::DateCheck(int year, int month, int day)
{
  int rc;

  if (!DatumToJulian(year, month, day)) {
    rc = 1;
  }
  else {
    rc = 0;
  }
  return rc;
}
/*----------------------------------------------------A. Gross; Nov. 88--
  TagNrMo(julian)
  Berechnet den Wochentag aus einem beliebigen Julianischen Datum
  groesser 2299161. Bei ungltiger Tagesnummer wird -1 zurckgegeben.
  Rueckgabewert:
//   -1 : Julian Date before 15.10.1582
//    0 : Monday
//    1 : Tuesday
//    2 : Wednesday
//    3 : Thursday
//    4 : Fryday
//    5 : Saturday
//    6 : Sunday

  I/O    Typ       Name    Beschreibung
  ---------------------------------------------------------
   I  int      julian     Tagesnummer
----------------------------------------------------------------------*/
int AGDateTimeStamp::TagNrMo(int julian)
{
  int rc;
  rc = 0;
  if (julian < 2299161L) {
    rc = -1;
  }
  else {
    rc = (int)(julian%7);
  }
  return rc;
}
//---------------------------------------------------------------------------
// char *getDayOfWeek(e_Lang language)
// Gibt den Wochentag als Zahl und, wenn "name" nicht NULL ist,
// die ersten zwei Buchstaben des Wochentags an:
//   -1 : Julian Date before 15.10.1582
//    0 : Monday
//    1 : Tuesday
//    2 : Wednesday
//    3 : Thursday
//    4 : Fryday
//    5 : Saturday
//    6 : Sunday
 char* AGDateTimeStamp::getDayOfWeek(AGDateTimeStamp::e_Lang language)
 {
  static char *dayNames[]=
  { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su", // English
    "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So", // German
    "Lu", "Ma", "Me", "Je", "Ve", "Sa", "Di", // French
    "Lu", "Ma", "Me", "Gi", "Ve", "Sa", "Do", // Italian
    "Lu", "Ma", "Mi", "Ju", "Vi", "Sa", "Do", // Spanish
    "Ma", "Di", "Wo", "Do", "Vr", "Za", "Zo", // Dutch
    "Ma", "Ti", "On", "To", "Fr", "Lo", "So", // Danish
    "Ma", "Ti", "On", "To", "Fr", "Lo", "So", // Swedish
    "Ma", "Ti", "On", "To", "Fr", "Lo", "So", // Norwegian
    "Ma", "Ti", "Ke", "To", "Pe", "La", "Su", // Finnish
    "NO", "BT", "CP", "UT", "NT", "Cb", "BC", // Russian (not exactly due to missing characters)
    "Po", "Wt", "Sr", "Cz", "Pi", "So", "Ni", // Polish
    "Po", "Ut", "St", "Ct", "Pa", "So", "Ne", // Czech
    "Po", "Ut", "St", "Sv", "Pi", "So", "Ne", // Slowakian
    "He", "Ke", "Sz", "Cs", "Pe", "So", "Va", // Hungarian
    "Es", "Te", "Ko", "Ne", "Re", "La", "Pu", // Estonian
    "Pi", "Ot", "Tr", "Ce", "Pi", "Se", "Sv", // Latvian
    "Pi", "An", "Tr", "Ke", "Pe", "Se", "Se", // Lithuanian
    "Se", "Te", "Qu", "Qi", "Se", "Sa", "Do", // Portuguese
    "Lu", "Ma", "Me", "Jh", "Ve", "Sa", "Di"  // Esperanto
  };

  char *rc;
  int today = TagNrMo(julianDay);
  if (today >= 0) rc = dayNames[language * 7 + today];
  else rc = NULL;
  return rc;
}
//---------------------------------------------------------------------------
/*----------------------------------------------------A. Gross; Mar. 08--
  int LastDayOfMonth(int Jahr, int Monat)
  Gibt den letzen Tag des Monats Monat im Jahr Jahr aus.

  Rueckgabe:
     1-31 : ok
     0    : ungueltiger Monat oder Jahr (wenn Jahr < 1583)

  I/O    Typ       Name    Beschreibung
  ---------------------------------------------------------
   I  int          Jahr    Jahr
   I  int          Monat   Monat
  ---------------------------------------------------------
----------------------------------------------------------------------*/
int AGDateTimeStamp::LastDayOfMonth(int year, int month)
{
  int MonLast[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int rc;

  if (year < 1583) {
    rc = 0;
  }
  else if ((month < 1) || (month > 12)) {
    rc = 0;
  }
  else {
    rc = MonLast[month-1];
    if (month == 2) {
      rc += isSchaltJahr(year);
    }
  }
  return (rc);
}
//---------------------------------------------------------------------------
  /*----------------------------------------------------A. Grosss; Nov. 88--
    int IsSchaltJahr(int jahr)
    Prueft, ob jahr ein Schaltjahr ist

    Rueckgabe:
    0 : kein Schaltjahr
    1 : Schaltjahr
  ----------------------------------------------------------------------*/
int AGDateTimeStamp::isSchaltJahr(int jahr)
{
  int rc;

  if (!(jahr%4)) { // ist durch 4 Teilbar
    if (!(jahr%100)) { // ist durch 100 teilbar
      if (!(jahr%400)) rc = 1;
      else rc = 0;
    }
    else rc = 1;
  }
  else rc = 0;
  return(rc);
}
//---------------------------------------------------------------------------

