#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
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
int page_mem = 0,page_num = 1; //page
bool page_change = false; //page state change
bool dataReady = false;
uint16_t co2 = 0;
float raw_temp = 0.0;
float raw_humidity = 0.0;
int temp = 0;
int humidity = 0;
int width_center = (int)(display.width()/2);
int height_center = (int)(display.height()/2);

int by8(int x){
  return 8*x;
}

float toFahrenheit(float c){
  return (c * 9/5) + 32; 
}
void clearPage(){
  display.fillScreen(GxEPD_WHITE);
  display.nextPage();
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
        sensor.readMeasurement(co2, raw_temp, raw_humidity);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    temp =round(toFahrenheit(raw_temp)); //convert and round to the nearest int
    humidity = round(raw_humidity); 
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

    // If temp offset and/or sensor altitude compensation
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

void centerJustifiedText(int x, int y, char text[], int font_size = 12){
  int text_width = (strlen(text)+1) * font_size;
  x = (int)(x-(text_width/2));
  display.setCursor(x-3,y);
  //Serial.printf("%d %d %d\n" , text_width, text_width , strlen(text) );
  display.printf("%s",text);
}

void centerJustifiedRect(int x ,int y, int width = 0, int height = 0, int16_t color = GxEPD_BLACK){
  display.fillRoundRect(x-(int)round(width/2), y, width, height, by8(1), color);
}


void pageTurnBar(int turn_num){

}


void pageBorder(char title[], char name_left[], char name_right[]){
  int title_size =0,name_left_size =0,name_right_size=0;
  title_size = strlen(title);
  name_left_size = strlen(name_left);
  name_right_size = strlen(name_right);
  int font_size = 12; //default 12
  
  int border_padding = 1;
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);

  //TOP
  //page title
  centerJustifiedText(width_center,by8(2),title);

  //BOTTOM
  centerJustifiedText(50,display.height() - by8(1), name_left);
  centerJustifiedText(350,display.height() - by8(1), name_right);
  
  //reset to normal text mode
  display.setFont(&FreeMonoBold24pt7b);
  display.setTextColor(GxEPD_BLACK);
}

void page1(){
  Serial.printf("temp:%d\n",temp);

  if(page_change){
    clearPage();
    display.fillRect(width_center,0,1,display.height(),GxEPD_BLACK);

    pageBorder("Inside","Puzzle","Outside");
    display.setCursor(by8(1),by8(8));
    display.printf("Temp");
    display.setCursor(by8(1),by8(16));
    display.printf("Temp:%dF",temp);
    
  }else{
    pageBorder("Inside","Puzzle","Outside");
    display.fillRect(0,by8(9),display.width(),by8(9),GxEPD_WHITE);
    display.setCursor(by8(1),by8(16));
    display.printf("Temp:%dF",temp);
    
  }
}

void page2(){
  Serial.printf("Humidity: %d%\n",humidity);
  if(page_change){
    clearPage();
    display.setCursor(by8(1),by8(6));
    display.printf("Page 2");
    display.setCursor(by8(1),by8(6*2));
    display.printf("Humidity:%d %",humidity);
    
  }else{
    display.fillRect(0,by8(8),display.width(),by8(7),GxEPD_WHITE);
    display.setCursor(by8(1),by8(6*2));
    display.printf("Humidity:%d %",humidity);
  }
}

void page3(){
  Serial.printf("CO2:%d\n",co2);
  if(page_change){
    clearPage();
    display.setCursor(by8(1),by8(6));
    display.printf("Page 3");
    display.setCursor(by8(1),by8(6*2));
    display.printf("CO2:%d",co2);
    
  }else{
    display.fillRect(0,by8(8),display.width(),by8(7),GxEPD_WHITE);
    display.setCursor(by8(1),by8(6*2));
    display.printf("CO2:%d",co2);
  }
}

void pages(int num){
  //first check if the page number is valid if not then adjust it
  if(page_num > 3){
    page_num = 1;
  }

  if(page_num < 1){
    page_num = 3;
  }

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
    default:
      page1();
      break;
  }
  //locks in page after change
  page_change = false;
  display.nextPage();
  page_mem = page_num;
}


void setup()
{
  Serial.begin(115200);

  //Co2 sensor
  Wire.begin();
  sensor.begin(Wire, SCD41_I2C_ADDR_62);
  co2Init();

  //wait for Serial to start
  while(Serial.available()){
    delay(100);
  }  

  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setFont(&FreeMonoBold24pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setTextWrap(false);
  display.clearScreen();
  display.fillScreen(GxEPD_WHITE); 
  
  display.display();
  display.display();
  display.setPartialWindow(0,0,display.width(),display.height());

  delay(2000);
};


void loop() {
  if(page_num!=page_mem){
    page_change = true;
  }

  //runs per a second 
  if(sec!=rtc.getSecond())
  {
    sec=rtc.getSecond();
  }
  
  //runs every 15 seconds
  if(sec%5==0||page_change){
    getSensorData();
     pages(page_num);
  
  }    
    
  //runs every 30 seconds
  if(sec%30==0){
     //page_num++;
  }
};
