#include <Adafruit_SleepyDog.h>

#include <OneWire.h>
#include <DallasTemperature.h>


#define ONE_WIRE_BUS 4  // NOTE THIS IS THAT BUS WIRE
#define TEMPERATURE_PRECISION 9
OneWire oneWire(ONE_WIRE_BUS);        // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.
DeviceAddress s1, s2, s3, s4, s5;     // arrays to hold device addresses


void setup(void) {
  Serial.begin(9600);
  sensors.begin();  // Start up the library

  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);  // locate devices on the bus
  Serial.println(" devices.");
  // report parasite power requirements (in Sensors_test)

  // method 1: by index
  if (!sensors.getAddress(s1, 0)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(s2, 1)) Serial.println("Unable to find address for Device 2");
  if (!sensors.getAddress(s3, 2)) Serial.println("Unable to find address for Device 3");
  if (!sensors.getAddress(s4, 3)) Serial.println("Unable to find address for Device 4");
  if (!sensors.getAddress(s5, 4)) Serial.println("Unable to find address for Device 5");

  Serial.print("Device 1 Address: ");
  printAddress(s1);
  Serial.println();
   Serial.print("Device 2 Address: ");
   printAddress(s2);
   Serial.println();
   Serial.print("Device 3 Address: ");
   printAddress(s3);
   Serial.println();
   Serial.print("Device 4 Address: ");
   printAddress(s4);
   Serial.println();
   Serial.print("Device 5 Address: ");
   printAddress(s5);
   Serial.println();
  // set the resolution to 9 bit per device
  sensors.setResolution(s1, TEMPERATURE_PRECISION);
  sensors.setResolution(s2, TEMPERATURE_PRECISION);
  sensors.setResolution(s3, TEMPERATURE_PRECISION);
  sensors.setResolution(s4, TEMPERATURE_PRECISION);
  sensors.setResolution(s5, TEMPERATURE_PRECISION);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(s1), DEC);
  Serial.println();
  //  Serial.print("Device 1 Resolution: ");
  //  Serial.print(sensors.getResolution(s2), DEC);
  //  Serial.println();
}


void printAddress(DeviceAddress deviceAddress)  // function to print a device address
{
  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
float getTemperature(DeviceAddress deviceAddress)  // function to print the temperature for a device
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
    return -1;
  }
  //Serial.print("Temp C: ");
 // Serial.print(tempC);
  //Serial.print(" Temp F: ");
  //  Serial.print(DallasTemperature::toFahrenheit(tempC));
  return tempC;
}
// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
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
bool active = false;
word timeElapsed = 0;
word startTime = 0;
float slope = 0;
word len = 1;
uint8_t past[5] = {0, 0, 0, 0, 0};

float updateSlope() {
  len++;
  // slope = slope * ((len - 1) / len) + (int(temp) / len);
  if (past[0] != 0) {
    slope = (past[-1] - past[0]) / sizeof(past);
  } else {
    slope = 0;
  }
  return slope;
}

void updatePast(uint8_t temp) {
  // if (past[0] == 0 || temp != past[-1]) {
    for (int i = 0; i < sizeof(past) - 1; i++) {
      past[i] = past[i+1];
    }
    past[-1] = temp;
  // }
}

float getSlope() {
  return slope;
}

int getTime() {
  if (changeNeeded) {
    timeElapsed = millis() - startTime;
  }
  return timeElapsed;
}

/*
   Main function, calls the temperatures in a loop.
*/
void loop(void) {
  sensors.requestTemperatures();
  float tmp1 = getTemperature(s1);
  Serial.print(tmp1);
  Serial.print(',');
  float tmp2 = getTemperature(s2);
  Serial.print(tmp2);
  Serial.print(',');
  float tmp3 = getTemperature(s3);
  Serial.print(tmp3);
  Serial.print(',');
  float tmp4 = getTemperature(s4);
  Serial.print(tmp4);
  Serial.print(',');
  float tmp5 = getTemperature(s5);
  Serial.print(tmp5);
  Serial.print(',');
  Serial.println();

  Serial.println();
  Serial.print("Past data: ");
  Serial.print(past[0]);
  Serial.print(", ");
  Serial.print(past[1]);
  Serial.print(", ");
  Serial.print(past[2]);
  Serial.print(", ");
  Serial.print(past[3]);
  Serial.print(", ");
  Serial.print(past[4]);
  Serial.print(", ");
  Serial.println();

  Serial.print("current slope:");
  if (tmp1 > tmp2 && tmp1 > tmp3 && tmp1 > tmp4 && tmp1 > tmp5) {
    updatePast(getTemperature(s1));
  }
  else if (tmp2 > tmp1 && tmp2 > tmp3 && tmp2 > tmp4 && tmp2 > tmp5) {
    updatePast(getTemperature(s2));
  } 
  else if (tmp3 > tmp2 && tmp3 > tmp1 && tmp3 > tmp4 && tmp3 > tmp5) {
    updatePast(getTemperature(s3));
  }
  else if (tmp4 > tmp2 && tmp4 > tmp3 && tmp4 > tmp1 && tmp4 > tmp5) {
    updatePast(getTemperature(s4));
  }
  else if (tmp5 > tmp2 && tmp5 > tmp3 && tmp5 > tmp4 && tmp5 > tmp1) {
    updatePast(getTemperature(s5));
  }
  Serial.println(updateSlope());
  //Serial.println("Finding slope...");

  // Please don't change state order, it is ordered by importance
  // State 2: The temparature is rising
  if (active && !changeNeeded && getSlope() > 0.1) {
    Serial.println("State 2: The temperature is rising");
    changeNeeded = true;
    // Start timer
    startTime = millis();
    Serial.println("Change this line to push state 2 to server");
  }
  // State 1: It is established to be on a human
  else if (!active && getSlope() > -0.5 && getSlope() < 0.5 && past[-1] > 25 && past[-1] < 35) {
    Serial.println("State 1: established to be on a human");
    active = true;
    Serial.println("Change this line to push state 1 to server");
  }
  else if (changeNeeded && !(past[-1] > 25 && past[-1] < 40)) {
    Serial.println("Reset to base state");
    active = false;
    changeNeeded = false;
    Serial.println("Change this line to push state 0 to server");
  }
  else if (changeNeeded && timeElapsed > 5000) { //TEST: change values to larger later
    Serial.println("change this line to push state 3 to server aka red");
  }
  if (!Serial) {
    Watchdog.sleep(2000);
  } else {
    delay(2000);
  }
}