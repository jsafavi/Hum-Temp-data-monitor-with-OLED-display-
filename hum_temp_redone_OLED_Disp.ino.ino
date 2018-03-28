
/************************ Adafruit IO Configuration *******************************/

// Adafruit info
#define IO_USERNAME    "jsafavi"
#define IO_KEY         "1975f53624e2433a992dbe59abc8b73a"

/******************************* WIFI Configuration *******************************/

#define WIFI_SSID       "agiwifi"
#define WIFI_PASS       "wireless"

#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

/************************ List of Libraries Included ****************************/

#include <ESP8266WiFi.h>       //related to IoT
#include <AdafruitIO.h>
#include <Adafruit_MQTT.h>
#include <ArduinoHttpClient.h>

#include <Arduino.h>            //related to sensor 
#include <Wire.h>
#include "Adafruit_SHT31.h"

//  I2C address for display 
#define I2C_ADDRESS 0x3C
#include "SSD1306Ascii.h"   //related to OLED display
#include "SSD1306AsciiWire.h"

/************************* Initiating feeds and variables ***********************/

// set up the feeds to adafruit io
AdafruitIO_Feed *hum_test       = io.feed("hum_test");
AdafruitIO_Feed *temp_test      = io.feed("temp_test");
AdafruitIO_Feed *calib_test     = io.feed("calib_test");
AdafruitIO_Feed *calib_test_t   = io.feed("calib_test_t");
AdafruitIO_Feed *sample_time    = io.feed("sample_time");
  
// initiating sensor 
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// initiating constatns and variables 
unsigned long previousMillis = 0;           // used to count time in the loop (sampling)
unsigned long previousMillis_red = 0;       // used to count time in the loop (red LED blink)
unsigned long interval = 20000;             // default sampling interval of sensor in milli seconds
unsigned long interval_led = 2000;          // blinking interval for red LED
int offset_h  = 0;                          // calibration value for humidity
int offset_t  = 0;                          // calibration value for temperature  


// Used by minmax function
float minTemp = 100;                        // initiating min temp value for minmaxFunction
float maxTemp = -100;                       // initiating max temp value for minmaxFunction
float minHum  = 100;
float maxHum  = -100;
int n = 0;
unsigned long func_interval_mins = 5;                 // interval time which data is written to arrays (min max sampling time)
int samples_per_24hour = 60*24/func_interval_mins;
float maxTemp24array[288];
float minTemp24array[288];
float maxTemp24 = -100;
float minTemp24 = 100;
float maxHum24array[288];
float minHum24array[288];
float maxHum24 = -100;
float minHum24 = 100;
unsigned long time_before = 0;


// initiating display
SSD1306AsciiWire oled;
unsigned long oledtime_previous = 0;    // used to count time to change frames of oled display
int f = 1;                              // number of frame being displayed 

unsigned long frame_transition_interval = 5000;       // transition time between each frame on display
unsigned long oledtime_now;


/********************************************************************************/

void setup() {

  // Display
  Wire.begin();                
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.set400kHz(); 
  oled.setFont(System5x7);
  oled.clear();
  oled.println("--Connecting to io--");
  oled.println();
  //oled.setFont(Arial_bold_14);
  oled.println(io.statusText());

  // start the serial connection
  Serial.begin(115200);
  pinMode(0, OUTPUT);                       // red LED 
  digitalWrite(0, HIGH);                    // turning off the red red LED
  pinMode(2, OUTPUT);                       // blue LED

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // set up a message handler for the 'temp_test' feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  calib_test->onMessage(handleMessage2);
  calib_test_t->onMessage(handleMessage4);
  sample_time->onMessage(handleMessage8);

  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  if (!sht31.begin(0x44)) {
    Serial.print("Could not find SHT31");
    while (1) delay(1);
  }
  
  // initializing arrays for min and max temp values 
  for (int j=0 ; j<samples_per_24hour ; j++) {
    maxTemp24array[j] = maxTemp24;
    minTemp24array[j] = minTemp24;
    maxHum24array[j]  = maxHum24;
    minHum24array[j]  = minHum24;
  }
  
  // Display 
  delay(1000);
  oled.clear();
  oled.setFont(System5x7);
  oled.clear();
  oled.println("--Connecting to io--");
  oled.println();
  //oled.setFont(Arial_bold_14);
  oled.println(io.statusText());

}

/********************************************************************************/

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  
  // reading sensor info
  float t = sht31.readTemperature() + offset_t;
  float h = sht31.readHumidity() + offset_h;
  float tf = t * 1.8 + 32.0;                  // Celsius to Fahrenheit conversion 
  //minmaxFunction(t,h);  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    
    digitalWrite(2, LOW);                 // turning on blue LED (sampling indicator)
    previousMillis = currentMillis;

    // printing and uploading sensor info to feeds on Adafruit
    Serial.println("Sending data --> ");
    Serial.print("Sampling at (sec) = ");
    Serial.println(interval/1000);
    Serial.print("Offset  (Temp *C) = ");
    Serial.println(offset_t);
    Serial.print("Offset (Humidity) = ");
    Serial.println(offset_h);
    Serial.print("Max Temp in 24hrs = ");
    Serial.println(maxTemp24);
    Serial.print("Min Temp in 24hrs = ");
    Serial.println(minTemp24);
    Serial.print("Max Hum. in 24hrs = ");
    Serial.println(maxHum24);
    Serial.print("Min Hum. in 24hrs = ");
    Serial.println(minHum24);
    Serial.print("ON Time: ");
    Serial.println(millis());
    Serial.print("ON Time (hr): ");
    Serial.println(millis()/(3600000UL));


    if (!isnan(t)) {
      Serial.print("Temperature in *C = "); Serial.println(t);
      Serial.print("Temperature in *F = "); Serial.println(tf);
      temp_test->save(t);
    }
    else {
      Serial.println("Failed to read Tempreture");
    }

    if (!isnan(h)) {
    Serial.print("Rel. Humidity  %  = "); Serial.println(h);
    hum_test->save(h);
    }
    else {
    Serial.println("Failed to read Humidity");
    }

    Serial.println();
    
  }
  
  else {
    digitalWrite(2, HIGH);            //turning off blue LED 
  }

  if (currentMillis - previousMillis_red >= interval_led) {
    previousMillis_red = currentMillis;
    digitalWrite(0, LOW);
    delay(500);  
    digitalWrite(0, HIGH);
  }
  
  // OLED display 
  oledtime_now = millis();
  if ((oledtime_now - oledtime_previous) > frame_transition_interval){

    oledtime_previous = oledtime_now;
    oled.clear();
    oled.setFont(System5x7);
    oled.clear();

    // frame conditions 
    if (f==1){                // frame 1: current temp and hum info 
//      oled.setFont(Arial14);
      oled.println("R. Humidity:"); oled.set2X();
      oled.print("  "); oled.print(h); oled.println(" %"); oled.set1X();
//      oled.println();
      oled.println("Temperature:"); oled.set2X();
      oled.print("  "); oled.print(t); oled.println(" *C"); 
      oled.print("  "); oled.print(tf); oled.println(" *F");
      oled.set1X();
    }
    if (f==2){                // frame 2: hum info in last 24hrs
      oled.println("-R. Humidity (24hrs)-");
      oled.println();
//      oled.println();
//      oled.setFont(Arial_bold_14);
      oled.println("Min R.H.: "); oled.set2X(); oled.print("  "); oled.print(minHum24); oled.println(" %"); oled.set1X();
      oled.println("Max R.H.: "); oled.set2X(); oled.print("  "); oled.print(maxHum24); oled.println(" %"); oled.set1X();
    }
    if (f==3){                // frame 3: temp info in last 24hrs
      oled.println("-Temperature (24hrs)-");
      oled.println();
//      oled.println();
//      oled.setFont(Arial_bold_14);
      oled.println("Min Temp: "); oled.set2X(); oled.print("  "); oled.print(minTemp24); oled.println(" *C"); oled.set1X();
      oled.println("Max Temp: "); oled.set2X(); oled.print("  "); oled.print(maxTemp24); oled.println(" *C"); oled.set1X();
      }
    if (f==4){                // frame 4: misc information  
      oled.println("-----Misc. Info-----");
      oled.println();
      oled.println(">> Sampling to io: "); oled.print(interval/1000); oled.println(" secs"); 
      oled.println(">> IP  Address: "); oled.println(WiFi.localIP());
      oled.println(">> Connection Status: "); oled.println(io.statusText());
    }
    if (f==5){
      oled.println("-----Misc. Info-----");
      oled.println();
      //unsigned long  on_time = millis();
      unsigned long  hrs_on  = millis()/(3600000UL);                   // milli seconds in one hour
      unsigned long  mins_on = (millis()/(60000UL))%(60UL);            // remainder gives minutes left to hour
      oled.println(">> ON time: "); oled.print(hrs_on); oled.print(" hrs "); oled.print(mins_on); oled.println(" mins");
      oled.println(">> Max/Min sampling:"); oled.print(func_interval_mins); oled.println(" mins");
      oled.println(">> ON time: "); oled.println(millis());
    }
    
    f++;                  // reset frame number 
    if (f>5){
      f=1;
    }
  }
    
  
}


/********************************************************************************/

// This function is called whenever data is received
// from calib_test feed on Adafruit IO. it can be 
// found in setup() function above.

void handleMessage2(AdafruitIO_Data *data2) {

  offset_h = data2->toInt();
  //Serial.print("received new hum Calib value <-- ");
  //Serial.println(offset_h);
  //Serial.println();
}

/********************************************************************************/

// This function is called whenever data is recieved 
// from sampling_test feed on Adafruit IO. It can be 
// found in setup() function above.

void handleMessage8(AdafruitIO_Data *data8) {

  interval = (data8->toLong())*1000;
  Serial.print("received new Sampling frequency <-- ");
  Serial.println(interval/1000);
  Serial.println();
}

/********************************************************************************/

// This function is called whenever data is received
// from calib_test_t feed on Adafruit IO. it can be 
// found in setup() function above.

void handleMessage4(AdafruitIO_Data *data4) {

  offset_t = data4->toInt();
  //Serial.print("received new temp Calib value <-- ");
  //Serial.println(offset_t);
  //Serial.println();
}

/********************************************************************************/

// This function is tasked with keeping the MIN and MAX value of
// data given to it over a specified period of time.
  //  It takes in input data (such as temp or humidity)
  //  and it outputs (saves) min and max of the inputted data.
  
//void minmaxFunction(float input_temp , float input_hum) {
//  
//  
//  unsigned long time_now = millis(); 
//  unsigned long func_Interval = func_interval_mins * 60000UL;               // converting minutes to millis         
//  
//  // saving the lowest temp
//  if (input_temp < minTemp) {
//    minTemp = input_temp;
//  }
//  
//  // saving the highest temp
//  if (input_temp > maxTemp) {
//    maxTemp = input_temp;
//  }
//  
//  // saving the lowest humidity 
//  if (input_hum < minHum) {
//    minHum = input_hum;
//  }
//  
//  // saving the highest Humidity 
//  if (input_hum > maxHum) {
//    maxHum = input_hum;
//  }
//
//      
//  // if one minute (or func_Interval) has passed, the following if statement will happen
//  if (time_now - time_before > func_Interval) {
//    time_before = time_now;
//    maxTemp24array[n] = maxTemp;
//    minTemp24array[n] = minTemp;
//    maxHum24array[n]  = maxHum;
//    minHum24array[n]  = minHum;
//    n++;
//    minTemp = 100;
//    maxTemp = -100;
//    minHum  = 100;
//    maxHum  = -100;
//    if (n == (samples_per_24hour-1)) {
//      n = 0;
//    } 
//  }
//  
//  maxTemp24 = maxTemp24array[0];
//  minTemp24 = minTemp24array[0];
//  maxHum24  = maxHum24array[0];
//  minHum24  = minHum24array[0];
//  // Finding the max and min in their respective arrays 
//  for (int i = 0; i<samples_per_24hour; i++) {
//    
//    if (maxTemp24 < maxTemp24array[i]) {
//    maxTemp24 = maxTemp24array[i];
//    }
//    
//    if (minTemp24 > minTemp24array[i]) {
//    minTemp24 = minTemp24array[i];
//    }
//
//    if (maxHum24 < maxHum24array[i]) {
//    maxHum24 = maxHum24array[i];  
//    }
//    
//    if (minHum24 > minHum24array[i]) {
//    minHum24 = minHum24array[i];  
//    }
//  }
//
//    if (time_now>0 && time_now<func_Interval){                // Instead of showing -100 and 100 on the display for min max until the first
//    maxTemp24 = maxTemp;                                      // samples are taken, this if statement will show min max until 
//    minTemp24 = minTemp;                                      // first sample is taken 
//    maxHum24  = maxHum;
//    minHum24  = minHum;
//  }
//}



/********************************************************************************/















