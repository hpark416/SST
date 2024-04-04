#include <DataTome.h>
#include <DataTomeAnalysis.h>
#include <DataTomeMvAvg.h>

#include <OneWire.h>
#include <DallasTemperature.h>


#define ONE_WIRE_BUS 2    // NOTE THIS IS THAT BUS WIRE
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DeviceAddress s1,s2,s3,s4,s5; // arrays to hold device addresses

DataTomeMvAvg<float> slope(4);


void setup(void)
{
  Serial.begin(9600);
  sensors.begin();  // Start up the library
  
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC); // locate devices on the bus
  Serial.println(" devices.");
  // report parasite power requirements (in Sensors_test)
  
  // method 1: by index
  if (!sensors.getAddress(s1, 0)) Serial.println("Unable to find address for Device 1");
//  if (!sensors.getAddress(s2, 1)) Serial.println("Unable to find address for Device 2");
//  if (!sensors.getAddress(s3, 2)) Serial.println("Unable to find address for Device 3");
//  if (!sensors.getAddress(s4, 3)) Serial.println("Unable to find address for Device 4");
//  if (!sensors.getAddress(s5, 4)) Serial.println("Unable to find address for Device 5");

  Serial.print("Device 1 Address: ");
  printAddress(s1);
  Serial.println();
//  Serial.print("Device 2 Address: ");
//  printAddress(s2);
//  Serial.println();
//  
//  Serial.print("Device 3 Address: ");
//  printAddress(s3);
//  Serial.println();
//  Serial.print("Device 4 Address: ");
//  printAddress(s4);
//  Serial.println();
//  Serial.print("Device 5 Address: ");
//  printAddress(s5);
//  Serial.println();
  // set the resolution to 9 bit per device
  sensors.setResolution(s1, TEMPERATURE_PRECISION);
//  sensors.setResolution(s2, TEMPERATURE_PRECISION);
//  sensors.setResolution(s3, TEMPERATURE_PRECISION);
//  sensors.setResolution(s4, TEMPERATURE_PRECISION);
//  sensors.setResolution(s5, TEMPERATURE_PRECISION);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(s1), DEC);
  Serial.println();
//  Serial.print("Device 1 Resolution: ");
//  Serial.print(sensors.getResolution(s2), DEC);
//  Serial.println();
}


void printAddress(DeviceAddress deviceAddress) // function to print a device address
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
float printTemperature(DeviceAddress deviceAddress) // function to print the temperature for a device
{
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return -1;
  }
  //Serial.print("Temp C: ");
  Serial.print(tempC);
  //Serial.print(" Temp F: ");
//  Serial.print(DallasTemperature::toFahrenheit(tempC));
  return tempC;
}
// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}
// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  //Serial.print("Device Address: ");
  //printAddress(deviceAddress);
  //Serial.print(" ");
  printTemperature(deviceAddress);
  //Serial.println();
}



/***********
 * Data Processing
 * Steps: establish baseline
 *  To test if it's on a human, it must be within a (large) range (TEST)
 * Continue reading and shifting code
 * If a slope of 1/75 (TEST THIS) within a range (TEST)
 * signal, start counting time
 * If button is pressed, then start over
 * If button is pressed outside of alert, do nothing
 ***********/

bool changeNeeded = false;
bool button = false;
bool active = false;
time_t timer;
int timeElapsed = 0;
int nowTime = 0;

float updateSlope(DeviceAddress deviceAddress){
  Serial.println("Finding slope...");
  slope.push(printTemperature(deviceAddress));
  return slope.get();
}

float getSlope() {
  return slope.get();
}

int getTime() {
  if (changeNeeded) {
    timeElapsed = timer.second() - nowTime;
  }
  return timeElapsed;
}

/*
   Main function, calls the temperatures in a loop.
*/
void loop(void)
{
  sensors.requestTemperatures();
  printData(s1);
  Serial.print(',');
  Serial.println();

  Serial.print("current slope:");
  Serial.println(updateSlope(s1));
  // State 1: It is established to be on a human
  if (getSlope() > -0.5 && getSlope() < 0.5) {
    Serial.println("State 1: established to be on a human");
    active = true;
  }
  // State 2: The temparature is rising
  if (active && getSlope() > 0.1 && !changeNeeded) {
    Serial.println("State 2: The temperature is rising");
    changeNeeded = true;
    // Start timer
    nowTime = timer.second();
  }
  if (changeNeeded && button) {
    Serial.println("Reset to base state");
    active = false;
    changeNeeded = false;
  }
  
  
  delay(50);
}
