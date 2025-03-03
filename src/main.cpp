#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>
// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <ESP32Time.h>

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0
SensirionI2cScd4x sensor; // Co2 sensor object
static char errorMessage[64]; // error buffer
static int16_t error; // error type

ESP32Time rtc(0);//RTC class   

//main display class for Weact 4.2 inch e-paper screen
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_DRIVER_CLASS(/*CS=5*/ 5, /*DC=*/ 1, /*RST=*/ 0, /*BUSY=*/ 6)); // ESP32-C3 Super Mini

int count = 0; //test for display
int64_t sec = 0; //current uptime second
int page_mem = 0,page_current = 0; //page
bool page_change = false; //page state change
bool dataReady = false;
uint16_t co2Concentration = 0;
float temperature = 0.0;
float relativeHumidity = 0.0;


int by8(int x){
  return 8*x;
}


void getSensorData(){
   error = sensor.getDataReadyStatus(dataReady);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute getDataReadyStatus(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    while (!dataReady) {
        delay(100);
        error = sensor.getDataReadyStatus(dataReady);
        if (error != NO_ERROR) {
            Serial.print("Error trying to execute getDataReadyStatus(): ");
            errorToString(error, errorMessage, sizeof errorMessage);
            Serial.println(errorMessage);
            return;
        }
    }
    //
    // If ambient pressure compensation during measurement
    // is required, you should call the respective functions here.
    // Check out the header file for the function definition.
    error =
        sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}

//Makes sure the sensor is fully functioning 
void co2Init(){
  uint64_t serialNumber = 0;
    delay(30);
    // Ensure sensor is in clean state
    error = sensor.wakeUp();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute wakeUp(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
    error = sensor.stopPeriodicMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
    error = sensor.reinit();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute reinit(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
    // Read out information about the sensor
    error = sensor.getSerialNumber(serialNumber);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    // If temperature offset and/or sensor altitude compensation
    // is required, you should call the respective functions here.
    // Check out the header file for the function definitions.
    // Start periodic measurements (5sec interval)
    error = sensor.startPeriodicMeasurement();
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}

void page1(){
  if(page_change){
    display.setCursor(by8(1),by8(6));
    display.printf("Temp: ");
    display.setCursor(by8(12), by8(6));
    display.printf("%f C",temperature);
    
    display.setCursor(by8(1),by8(6*2));
    display.printf("Humidity: ");
    display.setCursor(by8(12), by8(6*2));
    display.printf("%f %",relativeHumidity);

    display.setCursor(by8(1),by8(6*3));
    display.printf("CO2");
    display.setCursor(by8(12), by8(6*2));
    display.printf("%d ppm",co2Concentration);

    display.display();
    //locks in page after change
    page_change = false;
  }else{
    display.fillScreen(GxEPD_WHITE);

    display.setPartialWindow(by8(12), by8(6), display.width()-by8(12), by8(6));
    display.setCursor(by8(12), by8(6));
    display.printf("%f C",temperature);

    display.setPartialWindow(by8(12), by8(6*2), display.width()-by8(12), by8(6));
    display.setCursor(by8(12), by8(6*2));
    display.printf("%f %",relativeHumidity);

    display.setPartialWindow(by8(12), by8(6*3), display.width()-by8(12), by8(6));
    display.setCursor(by8(12), by8(6*3));
    display.printf("%d ppm",co2Concentration);

    display.nextPage();
    
  }
}

void page2(){
  if(page_change){
    display.setCursor(by8(1),by8(6));
    display.printf("Page 2");
    //locks in page after change
    page_change = false;
  }else{
    
  }
}

void page3(){
  if(page_change){
    display.setCursor(by8(1),by8(6));
    display.printf("Page 3");
    //locks in page after change
    page_change = false;
  }else{
    
  }
}

void pages(int num){
  switch (num){
    case 1:
      page1();
      break;
    case 2:
      page2();
      break;
    case 3:
      page3();
      break;
  }

}


void setup()
{
  Serial.begin(115200);

  //Co2 sensor
  Wire.begin();
  sensor.begin(Wire, 0x77);

  //wait for Serial to start
  while(Serial.available()){
    delay(100);
  }  

  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setFont(&FreeMonoBoldOblique24pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.clearScreen();
  display.fillScreen(GxEPD_WHITE); 
  display.setCursor(8,24);
 display.printf("%Hello World");
 display.display();
  delay(2000);
};



void loop() {

  if(page_current!=page_mem){
    page_change = true;
    pages(page_current); 
  }
  
  //runs per a second 
  if(sec!=rtc.getSecond())
  {
    sec=rtc.getSecond();
  }

  //runs every 15 seconds
  if(sec%15==0){
    error = sensor.getDataReadyStatus(dataReady);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute getDataReadyStatus(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    page_current++;
  }
  
  //runs every 30 seconds
  if(sec%30==0){
   
  }

};
