// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2    // NOTE THIS IS THAT BUS WIRE
#define TEMPERATURE_PRECISION 12
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
// arrays to hold device addresses
DeviceAddress s1,s2,s3,s4,s5;
// Assign address manually. The addresses below will need to be changed
// to valid device addresses on your bus. Device address can be retrieved
// by using either oneWire.search(deviceAddress) or individually via
// sensors.getAddress(deviceAddress, index)
// DeviceAddress s1 = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };
// DeviceAddress s2   = { 0x28, 0x3F, 0x1C, 0x31, 0x2, 0x0, 0x0, 0x2 };
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
  // Start up the library
  sensors.begin();
  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  //
  // method 1: by index
  if (!sensors.getAddress(s1, 0)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(s2, 1)) Serial.println("Unable to find address for Device 2");
  if (!sensors.getAddress(s3, 2)) Serial.println("Unable to find address for Device 3");
  if (!sensors.getAddress(s4, 3)) Serial.println("Unable to find address for Device 4");
  if (!sensors.getAddress(s5, 4)) Serial.println("Unable to find address for Device 5");

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to s1
  //if (!oneWire.search(s1)) Serial.println("Unable to find address for s1");
  // assigns the seconds address found to s2
  //if (!oneWire.search(s2)) Serial.println("Unable to find address for s2");
  // show the addresses we found on the bus
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
  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(s2), DEC);
  Serial.println();
}
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  //Serial.print("Temp C: ");
  //Serial.print(tempC);
  //Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
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
int sensorData[5] = {-1, -1, -1, -1, -1};
int total[5][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
int t = 1;
//float average[2][5] = {{0.0, 0.0, 0.0, 0.0, 0.0}, {-5.0, -5.0, -5.0, -5.0, -5.0}};
float maxSlope = -1.0;
time_t timer;

void baselineSlope(){
  maxSlope = -1.0;
  for (int i = 0; i < sizeof(total[0]); i++) {
    if ((total[4][i] - total[0][i]) / t > maxSlope) {
      maxSlope = (total[4][i] - total[0][i]) / t;
    }
  }
}

void updateRunningAverage(){
  //Move values down one column
  for (int i = 1; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      total[i][j] = total[i-1][j];
    }
  }
  for (int i = 0; i < sizeof(sensorData); i++) {
    total[0][i] = sensorData[i];
    t += 1;
  }
}



/*
   Main function, calls the temperatures in a loop.
*/
void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  //Serial.println("DONE");
  // print the device information
  printData(s1);
  Serial.print(',');
  printData(s2);
  Serial.print(',');
  printData(s3);
  Serial.print(',');
  printData(s4);
  Serial.print(',');
  printData(s5);
  Serial.println();

  
  updateRunningAverage();
  baselineSlope();
  // State 1: It is established to be on a human
  if (maxSlope > -0.5 && maxSlope < 0.5) {
    active = true;
  }
  // State 2: The temparature is rising
  if (active && maxSlope > 0.1) {
    changeNeeded = true;
    // Indicate light
    // Start timer
  }
  if (changeNeeded && button) {
    active = false;
    changeNeeded = false;
  }
  

  
  delay(50);
}