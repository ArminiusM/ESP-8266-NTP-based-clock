#include <AGSimpleNTPESP.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
// IP-Address of ESP AP when needed
IPAddress staticIP(10,10,10,1);
IPAddress gateway(10,10,10,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

//-------------------------------------------------------------------
// Set DEBUG to 1 to get more information on serial out
int DEBUG = 1;
//-------------------------------------------------------------------
// define the language for day of the week and region to calculate daylight time
#define LANGUAGE      AGDateTimeStamp::DE
#define REGION        AGSimpleNTPtime::EU
#define DAYLIGHTSHIFT 60
//-------------------------------------------------------------------
String NTPde("de.pool.ntp.org");  // NTP server
IPAddress NTPhome(192,168,1,1);   // local NTP server adress for home use (eg Raspberry)
bool atHome = true;

const char *myHostname = "ESPTime";
char* ssid = NULL;
char* password = NULL;
String Hssid = "Home Accesspoint SSID";
String Hpassword = "Password for Home Accesspoint";
const char* Assid = ""; // Alternative Home SSID
const char* Apassword = "";

bool IsConnected = false;

String WebPage;
char** WiFiList;
int WiFiCount = 0; // essential to set this to 0
//-------------------------------------------------------------------
// Time Zone
char HH = +1;
char MM = 0;
int waitTime = 50; // delay time in main loop
int oldSecond = 0;
int updateFrequency = 15; // NTP query every x minutes
//------------------------------------------------------------------
AGSimpleNTPtime *ntp;
bool forceUpdate;
//-------------------------------------------------------------------
void flashBuiltInLED(int duration) {
  digitalWrite(LED_BUILTIN, LOW);   // LED an
  delay(duration);                  // warten
  digitalWrite(LED_BUILTIN, HIGH);  // LED aus
}
//-------------------------------------------------------------------
void printTimeToSerial(AGDateTimeStamp TS, bool linefeed, bool printMillis) {
  char timeString[20] = {""};
  sprintf(timeString, "%s, %4d-%02d-%02d %02d:%02d:%02d", TS.getDayOfWeek(LANGUAGE), TS.getYear(), TS.getMonth(), TS.getDay(), TS.getHour(), TS.getMinute(), TS.getSecond());
  Serial.print(timeString);
  if (printMillis) {
    sprintf(timeString, ".%03d", TS.getMilliSecond());
    Serial.print(timeString);
  }
  // print summer time indicator
  if (TS.getSZ()) Serial.print(" +");
  if (linefeed) Serial.println("");
}
//-------------------------------------------------------------------
// scan for access points
//
int getNetworks(char ***WiFilist)
{
  if (DEBUG) Serial.println("Scanning Network...");

  int count = WiFi.scanNetworks();
  *WiFilist = new char* [count];

  for (int i = 0; i < count; i++) {
    (*WiFilist)[i] = new char [WiFi.SSID(i).length() + 2];
    WiFi.SSID(i).toCharArray((*WiFilist)[i], WiFi.SSID(i).length() + 1);
  }
  WiFi.scanDelete();

  if (DEBUG) Serial.println("... done");
  return count;
}
//-------------------------------------------------------------------
// delete list of access points
//
int deleteNetworkList(char ***WiFilist, int WiFicount)
{
  if (DEBUG) Serial.print("Free Network Space ... ");
  if (DEBUG) Serial.println(WiFiCount, DEC); 
  if (WiFicount > 0) {
    for (int i=0; i<WiFicount; i++) {
      delete [] ((*WiFilist)[i]);
    }
    delete [] *WiFilist;
  }
  if (DEBUG) Serial.println("... done");
  return 0;
}
//------------------------------------------------------------------
void handleSubmit()
{
  String button;
  String password;
  String APNo;

  if (server.hasArg("button")) {
    button = server.arg("button");
  }
  if (server.hasArg("pass")) {
    password = server.arg("pass");
  }
  if (server.hasArg("selected")) {
    APNo = server.arg("selected");
  }
  server.send(200, "text/html", WebPage);
  
  if (DEBUG) Serial.print("Button: ");
  if (DEBUG) Serial.println(button);
  if (DEBUG) Serial.print("Password: ");
//  if (DEBUG) Serial.println(password);
  if (DEBUG) Serial.println("***");
  if (DEBUG) Serial.print("AP: ");
  if (DEBUG) Serial.println(WiFiList[APNo.toInt()]);
  if (button == "Rescan") {
    WebPage = CreateWebPage();
    server.send(200, "text/html", WebPage);
  }
  if (server.hasArg("selected") && (button == "Connect")) {
    // Disable ESP acces point
    WiFi.disconnect();
    // copy WiFi dredentials
    copyCredentials(WiFiList[APNo.toInt()], password);
    // connect to selected ssid and if that fails, open accesspoint again
    connectToWiFiOrCreateAP();
  }
}
//------------------------------------------------------------------
void handleRootPath() {
  if (server.hasArg("button")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", WebPage);
  }
}
//------------------------------------------------------------------
String CreateWebPage() {
  String rc;
  
  WiFiCount = deleteNetworkList(&WiFiList, WiFiCount);
  WiFiCount = getNetworks(&WiFiList);

  if (DEBUG) {
    for (int i=0; i<WiFiCount; i++) {
      Serial.print(i, DEC);
      Serial.print(" : ");
      Serial.println(WiFiList[i]);
    }
  }
  
  rc = "<html>";
  rc += "<meta http-equiv=\"cache-control\" content=\"no-cache, must-revalidate, post-check=0, pre-check=0\" />";
  rc += "<meta http-equiv=\"cache-control\" content=\"max-age=0\" />";
  rc += "<meta http-equiv=\"expires\" content=\"0\" />";
  rc += "<meta http-equiv=\"expires\" content=\"Tue, 01 Jan 1980 1:00:00 GMT\" />";
  rc += "<meta http-equiv=\"pragma\" content=\"no-cache\" />";  
  rc += "<title>ESP Uhr</title><H2>Liste der Accesspoints</H2><H3><form>";
  for (int i=0; i<WiFiCount; i++) {
    rc += "<input type=\"radio\" name=\"selected\" value=\"";
    rc += i;
    rc += "\"><label>";
    rc += WiFiList[i];
    rc += "</label><BR><BR>";
  }
  rc += "<label>Password: <input type=\"password\" name=\"pass\"></label><BR><BR>";
  rc += "<input type=\"submit\" value=\"Connect\" name=\"button\"><BR><BR>";
  rc += "<input type=\"submit\" value=\"Rescan\" name=\"button\"><BR><BR>";
  rc += "<input type=\"submit\" value=\"Refresh\" name=\"button\">";
  rc += "</form></html>";

  return rc;
}
//-------------------------------------------------------------------
void copyCredentials(String mySSID, String myPassword) {
  if (DEBUG) Serial.print("Copy Credentials ... ");
  if (ssid) delete [] ssid;
  ssid = new char [mySSID.length() + 2];
  mySSID.toCharArray(ssid, mySSID.length()+1);

  if (password) delete [] password;
  password = new char [myPassword.length() + 2];
  myPassword.toCharArray(password, myPassword.length()+1);
  if (DEBUG) Serial.println("... done");
}
//-------------------------------------------------------------------
bool WifiStart() {
  bool rc = false;
  int timeout = 5;

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  digitalWrite(LED_BUILTIN, LOW);   // LED an
  while (WiFi.status() != WL_CONNECTED) {
    flashBuiltInLED(100);
    delay(1500);
    timeout--;
    if (timeout < 0) break;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connection established"); 
    Serial.print("IP address of ESP: ");
    Serial.println(WiFi.localIP());
    WiFiCount = deleteNetworkList(&WiFiList, WiFiCount);
    rc = true;
  }
  if (!rc) {
    Serial.println("Connection failed");
    WiFi.disconnect();
  }
  return rc;
}
//------------------------------------------------------------------
void StartAP()
{
  WebPage = CreateWebPage();

  WiFi.softAP("ESPTIME","password");
  WiFi.softAPConfig(staticIP, gateway, subnet);
  Serial.print("Soft AP IP address: ");
  Serial.println(staticIP);
  server.on("/", handleRootPath);
  server.begin();
}
//------------------------------------------------------------------
void connectToWiFiOrCreateAP()
{
  IsConnected = WifiStart();
  if (IsConnected) {
    if (atHome) ntp = new AGSimpleNTPtime(NTPhome, HH, MM, updateFrequency, REGION, DAYLIGHTSHIFT);
    else        ntp = new AGSimpleNTPtime(NTPde,   HH, MM, updateFrequency, REGION, DAYLIGHTSHIFT);
  }
  else{
    atHome = false;
    // try alternative Accesspoint
    copyCredentials(Assid, Apassword);
    IsConnected = WifiStart();
    if (!IsConnected) {
      // open AP and ask for other AP via Webpage
      // connect to AP ESPTime using mobile phone and start web browser with http://10.10.10.1.
      // select AP to connect to. rescan and refresh shows new result, when needed.
      StartAP();
      if (!MDNS.begin("ESPTime", staticIP)) Serial.println("Error setting up MDNS responder!");
      else Serial.println("mDNS responder started");
    }
  }
}
//------------------------------------------------------------------
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off

  Serial.begin(115200);
  Serial.println("");

  copyCredentials(Hssid, Hpassword);
  connectToWiFiOrCreateAP();

  oldSecond = 0;
  forceUpdate = false;
}
//------------------------------------------------------------------
void loop()
{
  if (!IsConnected) {
    server.handleClient();
  }
  else {
    // frequent call to ntp->update gets new timestamp and processes
    // the clock. Within the ntp class, the NTP server is queried every
    // "updateFrequency" minutes (defined when ntp instance is created,
    // see connectToWiFiOrCreateAP()).
    // ntp->update() should be called at least once a second.
    AGDateTimeStamp TimeStamp = ntp->update(&forceUpdate);
    if (oldSecond != TimeStamp.getSecond()) {
      oldSecond = TimeStamp.getSecond();
      printTimeToSerial(TimeStamp, true, false);
    }
    // delay for some milliseconds. Select a value that is small enough
    // to process the clock every second.
    delay(waitTime);
  }
}
