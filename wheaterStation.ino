#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <WiFiManager.h>

#define _TASK_SCHEDULING_OPTIONS
#include <TaskScheduler.h>

void startHTTPRequest();
void updateScreenStart();

Scheduler schedule;
// 10 min
Task httpRequestTask(10 * 60 * 1000, TASK_FOREVER, &startHTTPRequest); // min * sec * ms
Task updateScreenTask(5 * 1000, TASK_FOREVER, &updateScreenStart);

//////////////////// Server config START ///////////////////
const char* serverName = "http://fafnir.xyz/wheater/esp-post-data.php";
String apiKeyValue = "fwE23Vcvhjdc"; 
//////////////////// Server config END ///////////////////

//////////////////// Delay timers START ///////////////////
// Screens
int displayScreenNum = 0;
int displayScreenNumMax = 2;
// Screen refresh timers
unsigned long lastTimer = 0;
unsigned long timerDelayScreen = 3 * 1000;  // 3secs       sec * miliSec=> so 3sec
//////////////////// Delay times END ///////////////////

//////////////////// NTP server config START ///////////////////
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//////////////////// NTP server config END ///////////////////

//////////////////// Declare variable for date and time START ///////////////////
// Declare variable for date and time
String formattedDate;
String formattedTime;
String dayStamp;
String timeStamp;
//////////////////// Declare variable for date and time END ///////////////////

//////////////////// Define the screen for the lib START ///////////////////
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);
//////////////////// Define the screen for the lib END ///////////////////

//////////////////// Sensors input START ///////////////////
// DHT Sensor input
#define DHTPIN 14     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// DS18B20 Sensor input
const int oneWireBus = 27;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
//////////////////// Sensors input END ///////////////////

/////////////////////////////////////// Wifi setup start//////////////////////////////////////////
 //Callback fn, when the wifi manager go to config mode, the screen show to connect that AP
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  String ssid = {myWiFiManager->getConfigPortalSSID()};
  int ssid_len = ssid.length() + 1;
  char ssidRes[ssid_len];
  ssid.toCharArray(ssidRes, ssid_len);
  
  u8g2.clearBuffer(); //Oled screen msg start
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 64);  // Draw a screen size box
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_helvB14_tf);
  u8g2.drawButtonUTF8(64, 20, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "Please");
  u8g2.drawButtonUTF8(64, 40, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "connect to");
  u8g2.setFont(u8g2_font_helvB12_tf);
  u8g2.drawButtonUTF8(64, 60, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, ssidRes);
  u8g2.sendBuffer();
  delay(2000);
}

void connectToWifi() {
    u8g2.clearBuffer(); //Oled screen msg start
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 64);  // Draw a screen size box
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_helvB14_tf);
    u8g2.drawButtonUTF8(64, 20, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "Please wait");
    u8g2.drawButtonUTF8(64, 40, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "for");
    u8g2.drawButtonUTF8(64, 60, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "WIFI!");
    u8g2.sendBuffer();
    delay(2000);

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();
   
    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    wm.setAPCallback(configModeCallback);
    res = wm.autoConnect("ESP_AP"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        u8g2.clearBuffer(); //Oled screen msg start
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, 0, 128, 64);  // Draw a screen size box
        u8g2.setDrawColor(0);
        u8g2.setFont(u8g2_font_helvB14_tf);
        u8g2.drawButtonUTF8(64, 60, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "Failed!"); //Draw a button with centered text
        u8g2.sendBuffer();
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("Connected to WIFI!");

        u8g2.clearBuffer(); //Oled screen msg start
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, 0, 128, 64);  // Draw a screen size box
        u8g2.setDrawColor(0);
        u8g2.setFont(u8g2_font_helvB14_tf);
        u8g2.drawButtonUTF8(64, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "Connected"); //Draw a button with centered text
        u8g2.drawButtonUTF8(64, 50, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "to the wifi!"); //Draw a button with centered text
        u8g2.sendBuffer();
        delay(2000);
    }
  
}
/////////////////////////////////////// Wifi setup function end//////////////////////////////////////////

/////////////////////////////////////// SCREEN NUMBER 0: Inside temp start//////////////////////////////////////////
void displayInsideTemp(){

  float dhtHum = dht.readHumidity();
  if (isnan(dhtHum)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  //////////////////// Convert humidity to a good string format
  char dhtHumidity[20];
  dtostrf(dhtHum, 2, 2, dhtHumidity);
  char percentSymb[20] = "%";
  char dhtHumRes[20];
  strcpy (dhtHumRes, dhtHumidity) ;
  strcat (dhtHumRes, percentSymb);

  float dhtTemp = dht.readTemperature();
  if (isnan(dhtTemp)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  //////////////////// Convert celsius to a good string format
  char dhtCelsius[20];
  dtostrf(dhtTemp, 2, 2, dhtCelsius);
  char degreeSymb[20] = "\u00b0C";
  char dhtTempRes[20];
  strcpy (dhtTempRes, dhtCelsius) ;
  strcat (dhtTempRes, degreeSymb);

  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 64);  // Draw a screen size box
  u8g2.setDrawColor(0);
  u8g2.drawDisc(54, 8, 3, U8G2_DRAW_ALL);
  u8g2.drawCircle(64, 8, 3, U8G2_DRAW_ALL);
  u8g2.drawCircle(74, 8, 3, U8G2_DRAW_ALL);
  u8g2.setFont(u8g2_font_helvB14_tf);
  u8g2.drawButtonUTF8(64, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, dhtHumRes); // Draw a button with centered text
  u8g2.setFont(u8g2_font_helvB24_tf);
  u8g2.drawButtonUTF8(64, 60, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, dhtTempRes); //Draw a button with centered text
  u8g2.sendBuffer();
}
/////////////////////////////////////// SCREEN NUMBER 0: Inside temp end//////////////////////////////////////////

/////////////////////////////////////// SCREEN NUMBER 1: Outside temp start//////////////////////////////////////////
void displayOutsideTemp(){

  sensors.requestTemperatures(); 
  float ds18 = sensors.getTempCByIndex(0);
  if (isnan(ds18)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  //////////////////// Convert humidity to a good string format
  char ds18Celsius[20];
  dtostrf(ds18, 2, 2, ds18Celsius);
  char degreeSymb[20] = "\u00b0C";
  char ds18Res[20];
  strcpy (ds18Res, ds18Celsius) ;
  strcat (ds18Res, degreeSymb);

  u8g2.clearBuffer();
  u8g2.setDrawColor(1); /* color 1 for the box */
  u8g2.drawBox(0, 0, 128, 64);
  u8g2.setDrawColor(0);
  u8g2.drawDisc(64, 8, 3, U8G2_DRAW_ALL);
  u8g2.drawCircle(54, 8, 3, U8G2_DRAW_ALL);
  u8g2.drawCircle(74, 8, 3, U8G2_DRAW_ALL);
  u8g2.setFont(u8g2_font_helvB14_tf);
  u8g2.drawButtonUTF8(64, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "Kint:"); //Draw a button with centered text
  u8g2.setFont(u8g2_font_helvB24_tf);//ez marad
  u8g2.drawButtonUTF8(64, 60, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, ds18Res);
  u8g2.sendBuffer();
}
/////////////////////////////////////// SCREEN NUMBER 1: Outside temp end//////////////////////////////////////////

/////////////////////////////////////// SCREEN NUMBER 2: Clock start//////////////////////////////////////////
void displayClock(){

  // Date formatting
  String formattedDate = timeClient.getFormattedDate().substring(0, 10);
  // Length (with one extra character for the null terminator)
  int date_len = formattedDate.length() + 1;
  // Prepare the character array (the buffer)
  char dateRes[date_len];
  // Copy it over
  formattedDate.toCharArray(dateRes, date_len);
 
  // Time formatting
  String formattedTime = timeClient.getFormattedTime().substring(0, 5);
  // Length (with one extra character for the null terminator)
  int str_len = formattedTime.length() + 1;
  // Prepare the character array (the buffer)
  char timeRes[str_len];
  // Copy it over
  formattedTime.toCharArray(timeRes, str_len);

  u8g2.clearBuffer();
  u8g2.setDrawColor(1); /* color 1 for the box */
  u8g2.drawBox(0, 0, 128, 64);
  u8g2.setDrawColor(0);
  u8g2.drawDisc(74, 8, 3, U8G2_DRAW_ALL);
  u8g2.drawCircle(64, 8, 3, U8G2_DRAW_ALL);
  u8g2.drawCircle(54, 8, 3, U8G2_DRAW_ALL);
  u8g2.setFont(u8g2_font_helvB14_tf);
  u8g2.drawButtonUTF8(64, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, dateRes); //Draw a button with centered text
  u8g2.setFont(u8g2_font_helvB24_tf);//ez marad
  u8g2.drawButtonUTF8(64, 60, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, timeRes);
  u8g2.sendBuffer();
}
/////////////////////////////////////// SCREEN NUMBER 2: Clock end//////////////////////////////////////////

/////////////////////////////////////// DISPLAY SCREEN function start//////////////////////////////////////////
// Display the right screen accordingly to the displayScreenNum
void updateScreenStart() {
  if (displayScreenNum == 0){
    displayInsideTemp();
  }
  else if (displayScreenNum == 1) {
    displayOutsideTemp();
  }
  else if (displayScreenNum == 2){
    displayClock();
  }

  if(displayScreenNum < displayScreenNumMax) {
      displayScreenNum++;
    }
    else {
      displayScreenNum = 0;
    }
}
/////////////////////////////////////// DISPLAY SCREEN function end//////////////////////////////////////////

///////////////////HTTP REQUEST START ///////////////////////////
void startHTTPRequest(){

    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      // Prepare your HTTP POST request data
      String httpRequestData = "api_key=" + apiKeyValue + "&temp_in=" + String(dht.readTemperature())
                            + "&hum_in=" + String(dht.readHumidity()) + "&temp_out=" + String(sensors.getTempCByIndex(0)) + "";
      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData);

      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      if (httpResponseCode >= 200 && httpResponseCode <= 299 ) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

          char httpResCode[16];
          itoa(httpResponseCode, httpResCode, 10); /// Format str to char[]

          u8g2.clearBuffer();
          u8g2.setDrawColor(1); /* color 1 for the box */
          u8g2.drawBox(0, 0, 128, 64);
          u8g2.setDrawColor(0);
          u8g2.setFont(u8g2_font_helvB14_tf);
          u8g2.drawButtonUTF8(64, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "OK!"); //Draw a button with centered text
          u8g2.setFont(u8g2_font_helvB24_tf);//ez marad
          u8g2.drawButtonUTF8(64, 60, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, httpResCode);
          u8g2.sendBuffer();
          delay(2000);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);

          char httpResCode[16];
          itoa(httpResponseCode, httpResCode, 10);

          u8g2.clearBuffer();
          u8g2.setDrawColor(1); /* color 1 for the box */
          u8g2.drawBox(0, 0, 128, 64);
          u8g2.setDrawColor(0);
          u8g2.setFont(u8g2_font_helvB14_tf);
          u8g2.drawButtonUTF8(64, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, "Hiba!!!"); //Draw a button with centered text
          u8g2.setFont(u8g2_font_helvB18_tf);//ez marad
          u8g2.drawButtonUTF8(64, 50, U8G2_BTN_HCENTER | U8G2_BTN_BW0, 128, 2, 2, httpResCode);
          u8g2.sendBuffer();
          delay(10000);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    
  
}
///////////////////////////////////////HTTP REQUEST END ///////////////

// Setup
void setup() {
  Serial.begin(115200); //Serial screen start
  u8g2.begin(); //Lib start
  u8g2.enableUTF8Print();
  dht.begin(); // DHT Sensor start
  sensors.begin(); // DS18B10 Sensor start
  connectToWifi();
  timeClient.begin();
  timeClient.setTimeOffset(3600);

  httpRequestTask.setSchedulingOption(TASK_SCHEDULE);
  updateScreenTask.setSchedulingOption(TASK_SCHEDULE);
  schedule.init();
  schedule.addTask(httpRequestTask);
  schedule.addTask(updateScreenTask);
  updateScreenTask.enable();
}



void loop() {
  schedule.execute();


  //This if start the 10 minutes interval, exactly every 10 minutes
  if (((timeClient.getEpochTime() + 1) %600)==0){
    Serial.println("egész perc");
     httpRequestTask.enableIfNot();
  }

//  Serial.println(timeClient.getEpochTime()	);
  delay(1000);

  while(!timeClient.update()) {
      timeClient.forceUpdate();
    }

}