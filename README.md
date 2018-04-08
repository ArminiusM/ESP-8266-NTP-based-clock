# ESP-8266-NTP-based-clock
C++ code including class to keep track of time using NTP, including time zone and dst

This code runs on an ESP8266 and connects to an access point you have to add into the code. When connection to the access point fails, the ESP configures itself to an accesspoint you can connect to and start the web page http://10.10.10.1 in your browser to see the available access points, select one and type the passcode to ask the ESP to connect to that.

When connection to the AP is established, connection to an NTP server (also configurable in the source code) is established and a time stamp is created printing the time to the serial console every second. An instance of the class AGSimpleNTPtime keeps track of the time while AGSimpleNTPtime.update() is called in the loop() of the ESP. The RTC (millis()) of the ESP is used to calculate the time stamp at every call to update(). At frequent intervals the AGSimpleNTPtime instance is comparing the timestamp to the NTP time and adjusts the time by a few milliseconds at every update() call. How often the NTP server is called can be configured. The default is 15 minutes.

Look for these lines to change:
```
String NTPde("de.pool.ntp.org");  // NTP server used when connection to primary accesspoint fails  
IPAddress NTPhome(192,168,1,1);   // local NTP server adress used when access to primary accesspoint works.  
                                  // Insert an official one if you do not have one at home.  
String Hssid = "Home Accesspoint SSID";  
String Hpassword = "Password for Home Accesspoint";  
```
Optional changes:  
```
#define LANGUAGE      AGDateTimeStamp::DE  
#define REGION        AGSimpleNTPtime::EU  
#define DAYLIGHTSHIFT 60  
```
The directories containing the two classes AGSimpleNTPESP and AGDateTimeStamp must be copied to the Arduino's library folder.

When everything is correct, after compiling and uploading the serial console shoud show:
```
Connecting to YOURACCESSPOINTNAME  
WiFi Verbindung aufgebaut  
Eigene IP des ESP-Modul: 192.168.2.7  
Free Network Space ... 0  
... done  
So, 2018-04-08 19:48:01 +  
So, 2018-04-08 19:48:02 +  
So, 2018-04-08 19:48:03 +  
So, 2018-04-08 19:48:04 +  
So, 2018-04-08 19:48:05 +  
So, 2018-04-08 19:48:06 +  
So, 2018-04-08 19:48:07 +  
So, 2018-04-08 19:48:08 +  
So, 2018-04-08 19:48:09 +  
So, 2018-04-08 19:48:10 +  
```
where the + sign indicates daylight saving time. The language of the two digit day can be changed (see AGDateTimeStamp.h). Daylight saving time is calculated for mean european summer time or US daylight saving time.

Within the main loop it should be easy to set external devices like 7-segment displays etc to show the time. Just use TimeStamp->getHour() etc. to get the values you need. See printTimeToSerial() in the main program for example.

Enjoy
