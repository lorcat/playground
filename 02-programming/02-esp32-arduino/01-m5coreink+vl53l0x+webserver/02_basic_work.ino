// flag defining production state
#define PRODUCTION true

#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "M5CoreInk.h"
#include "esp_adc_cal.h"
#include "icon.h"

// local wifi variables - SSID + Password
#include "local_wifi.h"

// value to change in multithreaded approach
unsigned long value = 0;

// flag indicating first run at the device
bool bfirst_run = true;

// maximum value read at the battery
#define MAX_VOLTAGE 5.47

// core to run an additional thread
#define CORE_THREAD 1

// WiFi connection status converted to text
#define SWL_NO_SHIELD        "WiFi: No shield"
#define SWL_IDLE_STATUS      "WiFi: Idle"
#define SWL_NO_SSID_AVAIL    "WiFi: No SSID"
#define SWL_SCAN_COMPLETED   "WiFi: Scan done"
#define SWL_CONNECTED        "WiFi: Connected"
#define SWL_CONNECT_FAILED   "WiFi: Connect Failed"
#define SWL_CONNECTION_LOST  "WiFi: Connection lost"
#define SWL_DISCONNECTED     "WiFi: Disconnected"

// Wifi related information
const char* ssid = LOCAL_SSID;
const char* password = LOCAL_PASSWD;

// HTTP related things
#define HTTP_ROOT "/"
#define HTTP_SIGNAL "/signal"

// vl53l0x time of flight sensor

#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xc0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID      0xc2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS         0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14
#define VL53L0X_address                             0x29  // I2C address

// IP address to store
IPAddress ip;
uint8_t mac[6];
char macs[] ="00:00:00:00:00:00";

// Webserver running on the device
WebServer web_server(80);

// Ink screen to control
Ink_Sprite InkPageSprite(&M5.M5Ink);

// flags for internal loop control
bool bconfigured = false;

// VL53L0x code
byte gbuf[16];

uint16_t bswap(byte b[]) {
    // Big Endian unsigned short to little endian unsigned short
    uint16_t val = ((b[0] << 8) & b[1]);
    return val;
}

uint16_t makeuint16(int lsb, int msb) {
    return ((msb & 0xFF) << 8) | (lsb & 0xFF);
}

void write_byte_data(byte data) {
    Wire.beginTransmission(VL53L0X_address);
    Wire.write(data);
    Wire.endTransmission();
}

void write_byte_data_at(byte reg, byte data) {
    // write data word at VL53L0X_address and register
    Wire.beginTransmission(VL53L0X_address);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

void write_word_data_at(byte reg, uint16_t data) {
    // write data word at VL53L0X_address and register
    byte b0 = (data & 0xFF);
    byte b1 = ((data >> 8) && 0xFF);

    Wire.beginTransmission(VL53L0X_address);
    Wire.write(reg);
    Wire.write(b0);
    Wire.write(b1);
    Wire.endTransmission();
}

byte read_byte_data() {
    Wire.requestFrom(VL53L0X_address, 1);
    while (Wire.available() < 1) delay(1);
    byte b = Wire.read();
    return b;
}

byte read_byte_data_at(byte reg) {
    write_byte_data(reg);
    Wire.requestFrom(VL53L0X_address, 1);
    while (Wire.available() < 1) delay(1);
    byte b = Wire.read();
    return b;
}

uint16_t read_word_data_at(byte reg) {
    write_byte_data(reg);
    Wire.requestFrom(VL53L0X_address, 2);
    while (Wire.available() < 2) delay(1);
    gbuf[0] = Wire.read();
    gbuf[1] = Wire.read();
    return bswap(gbuf);
}

void read_block_data_at(byte reg, int sz) {
    int i = 0;
    write_byte_data(reg);
    Wire.requestFrom(VL53L0X_address, sz);
    for (i = 0; i < sz; i++) {
        while (Wire.available() < 1) delay(1);
        gbuf[i] = Wire.read();
    }
}

uint16_t VL53L0X_decode_vcsel_period(short vcsel_period_reg) {
    // Converts the encoded VCSEL period register value into the real
    // period in PLL clocks
    uint16_t vcsel_period_pclks = (vcsel_period_reg + 1) << 1;
    return vcsel_period_pclks;
}

// change green led indication
void enable_led(bool status=true){
  /*
  Sets the power led status LOW=ON; HIGH=OFF;
  */
  if(status)
    digitalWrite(LED_EXT_PIN, LOW);
  else
    digitalWrite(LED_EXT_PIN, HIGH);
}

// drawing functionality
void drawWarning(const char *str) {
    M5.M5Ink.clear();
    InkPageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
    drawImageToSprite(76, 40, &warningImage, &InkPageSprite);
    InkPageSprite.drawString(30, 100, str, &fonts::AsciiFont8x16);
    InkPageSprite.pushSprite();
}

void drawImageToSprite(int posX, int posY, image_t *imagePtr,
                       Ink_Sprite *sprite) {
    sprite->drawBuff(posX, posY, imagePtr->width, imagePtr->height,
                     imagePtr->ptr);
}

void drawInfoPage(bool printnet=false, unsigned long battery=200, float fbattery=0.){
  InkPageSprite.clear();

  char address[40];
  memset(address, 32, sizeof(address));

  sprintf(address, "IP : %s", ip.toString());
  InkPageSprite.drawString(15, 50, address);

  if(printnet)
    Serial.println(address);

  memset(address, 32, sizeof(address));
  sprintf(address, "MAC: %s", macs);
  InkPageSprite.drawString(15, 70, address);

  if(printnet)
    Serial.println(address);

  //if(battery){
    memset(address, 32, sizeof(address));
    sprintf(address, "Bat: %03d; %6.2f V;", battery, fbattery);
    InkPageSprite.drawString(15, 30, address);
  //}

  
  // adding connection status
  memset(address, 32, sizeof(address));

  switch(WiFi.status()){
    case WL_NO_SHIELD:
      sprintf(address, "%s", SWL_NO_SHIELD);
      break;
    case WL_IDLE_STATUS:
      sprintf(address, "%s", SWL_IDLE_STATUS);
      break;
    case WL_NO_SSID_AVAIL:
      sprintf(address, "%s", SWL_NO_SSID_AVAIL);
      break;
    case WL_SCAN_COMPLETED:
      sprintf(address, "%s", SWL_SCAN_COMPLETED);
      break;
    case WL_CONNECTED:
      sprintf(address, "%s", SWL_CONNECTED);
      break;
    case WL_CONNECT_FAILED:
      sprintf(address, "%s", SWL_CONNECT_FAILED);
      break;
    case WL_CONNECTION_LOST:
      sprintf(address, "%s", SWL_CONNECTION_LOST);
      break;
    case WL_DISCONNECTED:
      sprintf(address, "%s", SWL_DISCONNECTED);
      break;
  }

  InkPageSprite.drawString(15, 90, address);

  memset(address, 32, sizeof(address));
  sprintf(address, "mDNS: %s", mDNSName);
  InkPageSprite.drawString(15, 110, address);

  InkPageSprite.pushSprite();
}

// test battery voltage
float getBatVoltage() {
    analogSetPinAttenuation(35, ADC_11db);
    esp_adc_cal_characteristics_t *adc_chars =
        (esp_adc_cal_characteristics_t *)calloc(
            1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12,
                             3600, adc_chars);
    uint16_t ADCValue = analogRead(35);

    uint32_t BatVolmV = esp_adc_cal_raw_to_voltage(ADCValue, adc_chars);
    float BatVol      = float(BatVolmV) * 25.1 / 5.1 / 1000;
    free(adc_chars);
    return BatVol;
}

void checkBatteryVoltage(bool powerDownFlag=true) {
    float batVol = getBatVoltage();
    Serial.printf("Battery Voltage %.2f\r\n", batVol);

    if (batVol > 3.2){
      return;
    }

    const char* msg = "Battery voltage is low.";
    Serial.println(msg);
    drawWarning(msg);

    if (powerDownFlag == true) {
        M5.shutdown();
    }
}

// WiFi related
void mac2char(char* strmac) {
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, mac);

  if (ret == ESP_OK) {
    sprintf(strmac, "%02x:%02x:%02x:%02x:%02x:%02x",
                  mac[0], mac[1], mac[2],
                  mac[3], mac[4], mac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setupWiFi(){
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  Serial.println("");

  delay(500);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // notify user of the connection status
  M5.M5Ink.clear();
  InkPageSprite.drawString(35, 50, "Device has started.");
  InkPageSprite.drawString(31, 70, " WiFi is connected.");

  // prepare values
  ip = WiFi.localIP();
  mac2char(macs);

  delay(500);

  // show the info page
  drawInfoPage(true);
}

// http web server code
void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>%s</title>\
    <style>\
      body { background-color: #fff; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\
      h1 {font-size: 20; font-weight: bold;}\
    </style>\
  </head>\
  <body>\
    <h1>Hello from %s!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>Signal can be found <a href='/signal'>here</a>.</p>\
  </body>\
</html>", mDNSName, mDNSName,
           hr, min % 60, sec % 60
          );
          
  web_server.sendHeader("Cache-Control", "no-cache");
  web_server.send(200, "text/html", temp);
}

void handleSignal() {
  String message = "";
  message += "{\r\ndevice:\"";
  message += mDNSName;
  message += "\",\r\nvalue:";
  message += String(value, DEC) + ",\r\n";
  message += "}\r\n";

  web_server.sendHeader("Cache-Control", "no-cache");
  web_server.send(200, "text/plain", message);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += web_server.uri();
  message += "\nMethod: ";
  message += (web_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += web_server.args();
  message += "\n";

  for (uint8_t i = 0; i < web_server.args(); i++) {
    message += " " + web_server.argName(i) + ": " + web_server.arg(i) + "\n";
  }

  web_server.send(404, "text/plain", message);
}

// multithread target function, second core
void signal01(void* pvParameters) {  
    // server operation
    web_server.handleClient();

    delay(2);
}

// main setup
void setup() {
    // enable LED indication of the device power
    enable_led();

    // initialization of the M5CoreInk
    M5.begin();                
    if (!M5.M5Ink.isInit()) {  
                               
        Serial.printf("Ink Init failed");
        while (1) delay(100);
    }

    M5.M5Ink.clear();  
    delay(1000);

    if (InkPageSprite.creatSprite(0, 0, 200, 200, true) != 0) {
        Serial.printf("Ink Sprite create failed\n");
    }

    // check battery voltage
    checkBatteryVoltage(true);

    InkPageSprite.drawString(30, 50, "Device has started.");
    InkPageSprite.drawString(30, 70, "Connecting to WiFi.");
    InkPageSprite.pushSprite();
    delay(500);

    // begin wire communication
    // start i2c communication
    Serial.println("VLX53LOX I2C communication prepared.");
    Wire.begin();

    // setup wifi, set connection
    setupWiFi();

    // mDNS service for small networks without DNS server (multicast DNS)
    if (MDNS.begin(mDNSName)) {
      Serial.printf("mDNS responder started (%s)\r\n", mDNSName);
    }

    // HTTP web service
    web_server.on(HTTP_ROOT, handleRoot);
    web_server.on(HTTP_SIGNAL, handleSignal);
    web_server.onNotFound(handleNotFound);
    web_server.begin();
    
    Serial.println("Started HTTP web service\r\n");

    Serial.printf("Started a signal simulating thread at core (%d)\r\n", CORE_THREAD);

    // set configuration flag so that the loop can operate
    bconfigured = true;
  
}

// distance measurment
uint16_t measure(){
    write_byte_data_at(VL53L0X_REG_SYSRANGE_START, 0x01);

    byte val = 0;
    int cnt  = 0;

    while (cnt < 100) {  // 1 second waiting time max
        delay(10);
        val = read_byte_data_at(VL53L0X_REG_RESULT_RANGE_STATUS);
        if (val & 0x01) break;
        cnt++;
    }

    if (val & 0x01){
        if(!PRODUCTION) 
          Serial.println("ready");
    }else{
        if(!PRODUCTION) 
          Serial.println("not ready");
        return 0;
    }

    read_block_data_at(0x14, 12);

    uint16_t acnt                  = makeuint16(gbuf[7], gbuf[6]);
    uint16_t scnt                  = makeuint16(gbuf[9], gbuf[8]);
    uint16_t dist                  = makeuint16(gbuf[11], gbuf[10]);

    value = dist;

    byte DeviceRangeStatusInternal = ((gbuf[0] & 0x78) >> 3);

    if( (dist > 25 && dist < 1000) && !PRODUCTION){
      Serial.print("ambient count: ");
      Serial.println(acnt);
      Serial.print("signal count: ");
      Serial.println(scnt);
      Serial.print("distance: ");
      Serial.println(dist);
      Serial.print("status: ");
      Serial.println(DeviceRangeStatusInternal);
    }
    
    return dist;
}

// check points for parameter update and time tracking
unsigned long cp_250 = 0;     // 250 ms
unsigned long cp_5000 = 0;    // 5 second
unsigned long cp_30000 = 0;   // 30 second
unsigned long cp_300000 = 0;  // 300 second - 5 minutes

void loop() {
  // check time
  unsigned long curtime = millis();

  // take care of checkpoint value overflow. 
  // millis should be sufficient for 49 days
  if(cp_250 > curtime)
    cp_250 = curtime;

  if(cp_5000 > curtime)
    cp_5000 = curtime;

  if(cp_30000 > curtime)
    cp_30000 = curtime;

  if(cp_300000 > curtime)
    cp_300000 = curtime;

  // perform actions after configuration is done
  if(bconfigured){
    // optional code, just to check battery
    // check points checking information over predefined time base
    // updating the screen makes the device a slightly slower HTTP server
    if(curtime - cp_300000 > 300000 || bfirst_run){
      cp_300000 = curtime;
      float fbvolt = getBatVoltage();

      if(fbvolt > MAX_VOLTAGE)
        fbvolt = MAX_VOLTAGE;

      int ibvolt = uint(round(100 / (5.47-3.2) * (fbvolt-3.2)));

      /* Serial.printf("Tick Tack 1000: %d\r\n", cp_5000);
      Serial.printf("Battery Voltage: %d \%\t%6.4f\r\n", 
                      ibvolt,
                      fbvolt
                    );
      */
      
      drawInfoPage(false, ibvolt, fbvolt);


      // change the first run flag
    if(bfirst_run)
        bfirst_run = !bfirst_run;
    }

  }

  // check point for distance measurement, 4Hz
  curtime = millis();
  if(curtime - cp_250 > 250){
    cp_250 = curtime;
    measure();
  }

  web_server.handleClient();

  // Refresh buttons logic
  M5.update();

  // Shutdown on power press
  if( M5.BtnPWR.wasPressed() ){
    Serial.printf(
                  "Button %d was pressed.\r\n"\
                  "M5CoreInk is shutting down.\r\n", 
                  BUTTON_EXT_PIN
                  );
    enable_led(false);
    M5.shutdown();
  }
  
  delay(2);
}
