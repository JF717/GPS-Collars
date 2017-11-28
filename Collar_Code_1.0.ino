//Code for arduino GPS collars using Adafruit GPS and accelerometer
//Author - James Foley, Univesity of Oxford
//Originally for Ethiopian Wolf Conservation Programme
//Date 11/05/2017

//include all the packages you need
#include <SPI.h>
#include <Adafruit_GPS.h>
#include <TinyGPS.h>
#include <SD.h>
//#include <SoftwareSerial.h>

//package for making the microcontroller sleep, might be useful if accelerometer data is not required just used for smart collars
//#include <avr/sleep.h>

#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// define the pin that will turn on and off the GPS to save power High is off Low is on
// #define Enable 6

//not sure these are strictly necessary as with this wiring the accelerometer is running I2C but if wired different will need one of these
// Used for software SPI
//#define LIS3DH_CLK 13
//#define LIS3DH_MISO 12
//#define LIS3DH_MOSI 11
// Used for hardware & software SPI
//#define LIS3DH_CS 10

//assign the accelerometer
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  true
/* set to true to only log to SD when GPS has a fix, for debugging, keep it false */
#define LOG_FIXONLY false  

// define the GPS (with the serial)
Adafruit_GPS GPS(&Serial1);

// set up the files that will be used, one to save accelerometer data one for gps data
File accellog; 
File GPSlog;

//Assign time variable currently just tracking miliseconds since it turned on
unsigned long time;

//Assign File Name Variables
char filename1[15];
char filename2[15];

//create a class for Accelerometer readings so multiple can be stored across different times and then logged as a batch to save battery
class Accelerometer
  {
    //class member variables

    // read is an array which means it has a length of this much, so you will store it into the array, once the array is full log it,
    // curre ntly set to 100 which is 10 seconds
    public:
    float xRead[100];
    float yRead[100];
    float zRead[100]; 
    
    long interval;
    unsigned long previoustime;
    unsigned long Time[100];
    float x0;
    float x100;
  //the public values of this
 
  Accelerometer(long Interval)
  {
    Interval = interval;

    memset(xRead,0,sizeof(xRead));
    memset(yRead,0,sizeof(yRead));
    memset(zRead,0,sizeof(zRead));
    memset(Time,0,sizeof(Time));

    previoustime = 0;

    x0 = xRead[0];
    x100 = xRead[100];

    
  }

//create a function to read the accelerometer
void readaccelerometer()
{
  unsigned long currentMillis = millis();
  sensors_event_t event; 
  lis.getEvent(&event);
   int i = 0;
  if(currentMillis - previoustime >= interval)
  { 
    xRead[i] = event.acceleration.x;
    previoustime = currentMillis;
  
    yRead[i] = event.acceleration.y;
    previoustime = currentMillis;

    
    zRead[i] = event.acceleration.z;
    previoustime = currentMillis;
    
    Time[i] = millis();

    i = i+1;
  }

   if(i = 100)
   {
        accellog = SD.open(filename1, FILE_WRITE);
    for (int j = 0; j<100; j++)
    // writing the accelerometer data to the SD card
    {
      if (accellog) {
    accellog.print("Time: "); accellog.print(Time[j]);accellog.print("  ");
    accellog.print("X: "); accellog.print(xRead[j]); accellog.print("  ");
    accellog.print("Y: "); accellog.print(yRead[j]); accellog.print("  ");
    accellog.print("Z: " ); accellog.print(zRead[j]); accellog.print("  ");
    }
    } 
    accellog.close();
    i=0; 
   }
    
  }
 
};

//A class for the GPS
class GPSchip
{
  //class member variables
  int Offpin;
  long interval;
  int PinState;
  int attempts;
  long fixinterval;
  unsigned long previoustime;
  boolean On;
  
  public:
  // the variables entered into the GPS when constructed, interval is time between fixes in minutes (alter equation if you want it in seconds),
  // Enable is the pin number that the enable pin is soldered to ,
  // Checkfix is how many minutes you want it to search for a fix before it turns off
  GPSchip(int Interval, int Enable, int Checkfix)
  {
    interval = ((Interval * 60) / 100);
    Enable = Offpin;
    PinState = HIGH;
    attempts = 0;
    previoustime = 0;
    fixinterval = ((Checkfix * 60) / 1000);
    On = false;
    pinMode(Enable, OUTPUT);
    
  }

void GPSread()
{ 
  
  unsigned long currentMillis = millis();
  unsigned long Starttime;
  if(currentMillis - previoustime >= interval)
  {
    digitalWrite(Offpin,LOW);
    
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPS.sendCommand(PGCMD_NOANTENNA);

    Starttime = currentMillis;
    
    On = true;
    previoustime = currentMillis;
  }
    
  if (On && LOG_FIXONLY && GPS.fix)
    {
      if (GPSlog) {
    GPSlog.print("Time: "); GPSlog.print(time); GPSlog.print("  ");
    GPSlog.print("GPS: "); GPSlog.println(GPS.read());
    digitalWrite(Offpin, HIGH);
    On = false;
    }
    }
   if (currentMillis - Starttime >= fixinterval && !GPS.fix)
    {
    digitalWrite(Offpin, HIGH);
    On = false;
    }
  }

};

void setup() {
// put your setup code here, to run once:

// this sets the filename and checks if it already exists on the card, if it does it changes the name  to something else maybe?
strcpy(filename1, "ACCLOG00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename1[6] = '0' + i/10;
    filename1[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename1)) {
      break;}}
      
strcpy(filename2, "GPSLOG00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename2[6] = '0' + i/10;
    filename2[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename2)) {
      break;}}


  }
void loop() {
  // put your main code here, to run repeatedly:

Accelerometer Accel(100);
GPSchip GPS1(5,6,2);
  
Accel.readaccelerometer();

if ((Accel.x0 - Accel.x100) != 0)
  {
    GPS1.GPSread();
  }


}
// end code
