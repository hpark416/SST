#include <FirebaseClient.h>
#include <WiFi.h>
#include <Adafruit_SleepyDog.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include <movingAvg.h>
#define ONE_WIRE_BUS 4  // NOTE THIS IS THAT BUS WIRE
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS);        // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.
DeviceAddress s1, s2, s3, s4, s5;     // arrays to hold device addresses

#define WIFI_SSID "ross"
#define WIFI_PASSWORD "123456789"
#define past_arr_size 5
#define past_arr_size_float 5.0
// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyCRE3X98T5ZSZO4DEhOKFEBO9wInWUiUEs"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "murphyclaire91@gmail.com"
#define USER_PASSWORD "senior_shield5"
#define DATABASE_URL "http://uti-guard.firebaseapp.com/"

#define FIREBASE_PROJECT_ID "uti-guard"

void asyncCB(AsyncResult &aResult);

//state list
//1 - green
//2 - yellow
//3 - red
//4 = undeterminate

void printResult(AsyncResult &aResult);
DefaultNetwork network;  // initilize with boolean parameter to enable/disable network reconnection
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
WiFiClientSecure ssl_client;
// In case the keyword AsyncClient using in this example was ambigous and used by other library, you can change
// it with other name with keyword "using" or use the class name AsyncClientClass directly.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
Firestore::Documents Docs;
AsyncResult aResult_no_callback;
int counter = 0;
unsigned long dataMillis = 0;
bool changeNeeded = false;
bool active = false;
word timeElapsed = 0;
word startTime = 0;
word len = 1;
int wifiMessage = 4;
int past[5] = { 0, 0, 0, 0, 0 };  //hold past Max int temp values

//moving averages for max tmps
movingAvg avgTempMax(10);


void initSensors() {
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

void initWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  Serial.println("Initializing app...");
  ssl_client.setInsecure();
  app.setCallback(asyncCB);
  initializeApp(aClient, app, getAuth(user_auth));
  // Waits for app to be authenticated.
  // For asynchronous operation, this blocking wait can be ignored by calling app.loop() in loop().
  ms = millis();
  while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
    ;

  app.getApp<Firestore::Documents>(Docs);
}

void setup(void) {
  Serial.begin(9600);
  initSensors();
  initWifi();
  //movingAvg initialization
  avgTempMax.begin();
}
void printAddress(DeviceAddress deviceAddress)  // function to print a device address
{
  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

float getTemperature(DeviceAddress deviceAddress) {

  //implement a simple moving average across all sensors
  //TODO maybe we'll have enough memory for this?

  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
    return -1;
  }
  return tempC;
}
// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress) {
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

void asyncCB(AsyncResult &aResult) {
  // To get the UID (string) from async result
  // aResult.uid();
  printResult(aResult);
}

void printResult(AsyncResult &aResult) {
  if (aResult.appEvent().code() > 0) {
    Firebase.printf("Event msg: %s, code: %d\n", aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug()) {
    Firebase.printf("Debug msg: %s\n", aResult.debug().c_str());
  }

  if (aResult.isError()) {
    Firebase.printf("Error msg: %s, code: %d\n", aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available()) {
    Firebase.printf("payload: %s\n", aResult.c_str());
  }
}

/***********
   Data Processing
   Steps: establish baseline
    To test if it's on a human, it must be within a (large) range (TEST)
   Continue reading and shifting code
   If a slope of 1/75 (TEST THIS) within a range (TEST)
   signal, start counting time
   If button is pressed, then start over
   If button is pressed outside of alert, do nothing
 ***********/


float calculateSlope() {
  len++;
  float slope = -99.0;
  // slope = slope * ((len - 1) / len) + (int(temp) / len);
  if (past[0] != 0) {
    slope = (past[past_arr_size - 1] - past[0]) / past_arr_size_float;
  } else {
    slope = 0;
  }
  return slope;
}

void updatePast(int temp) {
  // if (past[0] == 0 || temp != past[sizeof(past) - 1]) {
  for (int i = 0; i < past_arr_size - 1; i++) {
    past[i] = past[i + 1];
  }
  past[past_arr_size - 1] = temp;
  // }
}

int getTime() {
  if (changeNeeded) {
    timeElapsed = millis() - startTime;
  }
  return timeElapsed;
}

void printSerialPlotter(float tmp1, float tmp2, float tmp3, float tmp4, float tmp5, float slope, int maxAvg) {
  Serial.print("S1:");
  Serial.print(tmp1);
  Serial.print(',');

  Serial.print("S2:");
  Serial.print(tmp2);
  Serial.print(',');

  Serial.print("S3:");
  Serial.print(tmp3);
  Serial.print(',');

  Serial.print("S4:");
  Serial.print(tmp4);
  Serial.print(',');

  Serial.print("S5:");
  Serial.print(tmp5);
  Serial.print(',');

  Serial.print("Slope:");
  Serial.print(slope);
  Serial.print(',');

  Serial.print("Avg");
  Serial.println(maxAvg);
}

// float getAverageTemperature(float tmp1, float tmp2, float tmp3, float tmp4, float tmp5) {
//   return (tmp1 + tmp2 + tmp3 + tmp4 + tmp5) / (5.0);
// }

void printToLog() {
  //used for data collection
  
}

float getMaxTemperature(float tmp1, float tmp2, float tmp3, float tmp4, float tmp5) {
  if (tmp1 > tmp2 && tmp1 > tmp3 && tmp1 > tmp4 && tmp1 > tmp5) {
    return getTemperature(s1);
  } else if (tmp2 > tmp1 && tmp2 > tmp3 && tmp2 > tmp4 && tmp2 > tmp5) {
    return getTemperature(s2);
  } else if (tmp3 > tmp2 && tmp3 > tmp1 && tmp3 > tmp4 && tmp3 > tmp5) {
    return getTemperature(s3);
  } else if (tmp4 > tmp2 && tmp4 > tmp3 && tmp4 > tmp1 && tmp4 > tmp5) {
    return getTemperature(s4);
  } else if (tmp5 > tmp2 && tmp5 > tmp3 && tmp5 > tmp4 && tmp5 > tmp1) {
    return getTemperature(s5);
  } else {
    return getTemperature(s3);
  }
}

bool checkStateOne(float tmpSlope, int avgMaxTmp) {
  // State 1: It is established to be on a human
  //  else if (changeNeeded && !(past[sizeof(past) - 1] > 25 && past[sizeof(past) - 1] < 40)) {
  //    Serial.println("Reset to base state");
  //    active = false;
  //    changeNeeded = false;
  //    wifiMessage = 0;
  //    //    if (app.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
  //    //    {
  //    //      write.setContent(0);
  //    //    }
  //  }

  //device is on a human if:
  //the device is not already active,
  // -0.5 < slope < 0.5 AND
  // 29 < most recent MAX temperature gathered < 35
  return (!active && tmpSlope >= 1.5 && avgMaxTmp >= 30 && avgMaxTmp <= 33);

  //return (!active && tmpSlope > -0.5 && tmpSlope < 0.5 && past[past_arr_size - 1] > 29 && past[past_arr_size - 1] < 35);
}

bool checkStateTwo(float tmpSlope) {
  //return (active && !changeNeeded && tmpSlope > 0.1);
  return (active && !changeNeeded&& tmpSlope >= 3.5);
}

bool checkStateThree() {
  return (changeNeeded && timeElapsed > 5000);  //TEST: change values to larger later
}

/*
   Main function, calls the temperatures in a loop.
*/
void loop(void) {
  sensors.requestTemperatures();
  float tmp1 = getTemperature(s1);
  float tmp2 = getTemperature(s2);
  float tmp3 = getTemperature(s3);
  float tmp4 = getTemperature(s4);
  float tmp5 = getTemperature(s5);

  float maxTmp = getMaxTemperature(tmp1, tmp2, tmp3, tmp4, tmp5);
  //Serial.print("Max Sensor Tmp:");
  //Serial.println(maxTmp);

  int maxTmpAverage = avgTempMax.reading(maxTmp);  // calculate the moving average
  updatePast(maxTmpAverage);//store the last 5 max temperature values and use to complete the slope
  float slope = calculateSlope() * 10.0;  //scale value for comparison
  printSerialPlotter(tmp1, tmp2, tmp3, tmp4, tmp5, slope, maxTmpAverage);

  // This function is required for handling async operations and maintaining the authentication tasks.
  app.loop();
  // This required when different AsyncClients than used in FirebaseApp assigned to the Firestore functions.
  Docs.loop();

  // To get anyc result without callback
  // printResult(aResult_no_callback)

  //Order state by importance
  // State 2: The temparature is rising
  if (checkStateTwo(slope)) {
    Serial.println("State 2: The temperature is rising");
    changeNeeded = true;
    // Start timer
    startTime = millis();
    wifiMessage = 2;
    //    if (app.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    //    {
    //      write.setContent(2);
    //    }
  } else if (checkStateOne(slope, maxTmpAverage)) {
    Serial.println("State 1: established to be on a human");
    active = true;
    wifiMessage = 1;
    //    if (app.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    //    {
    //      write.setContent(Values::IntegerValue(1));
    //    }

  } else if (checkStateThree()) {
    //    if (app.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    //    {
    //      write.setContent(3);
    //    }
    //TODO make this change to 3 later on
    wifiMessage = 3;
  } else {
    //state remains 0 unless changed?
  }

  //-------------------------Sending Data---------------------------------------------------///

  if (app.ready() && (millis() - dataMillis > 5000 || dataMillis == 0)) {
    dataMillis = millis();
    counter++;
    //Serial.println("Commit a document (set server value, update document)... ");
    String documentPath = "temperature/temperature";
    String fieldPath = "temp";

    // See https://firebase.google.com/docs/firestore/reference/rest/v1/Write#servervalue
    FieldTransform::SetToServerValue setValue(FieldTransform::REQUEST_TIME);  // set timestamp to "test_collection/test_document/server_time"
    FieldTransform::FieldTransform fieldTransforms(fieldPath, setValue);
    DocumentTransform transform(documentPath, fieldTransforms);

    Writes writes(Write(transform, Precondition() /* currentDocument precondition */));
    //////////////////////////////
    // Add another write for update

    //        documentPath = "test_collection/d" + String(counter);
    //        Values::MapValue mapV("temp", Values::IntegerValue(wifiMessage));
    //        mapV.add("random", Values::IntegerValue(rand()));
    //        mapV.add("status", Values::BooleanValue(counter % 2 == 0));
    //        Document<Values::Value> updateDoc;
    //        updateDoc.setName(documentPath);
    //        updateDoc.add("myMap", Values::Value(mapV));
    //        mapV.add("random", Values::IntegerValue(rand()));
    //        mapV.add("status", Values::BooleanValue(counter % 2 == 0));

    //        Writes writes(Write(updateDoc, Precondition() /* currentDocument precondition */));

    documentPath = "Residents/states";
    Serial.println("Sending State!");
    Serial.println(wifiMessage);
    Values::MapValue mapV("state", Values::IntegerValue(wifiMessage));
    Document<Values::Value> updateDoc;
    updateDoc.setName(documentPath);
    updateDoc.add("myMap", Values::Value(mapV));

    writes.add(Write(DocumentMask(), updateDoc, Precondition()));
    //////////////////////////////
    // Add another write for delete
    /*
      documentPath = "test_collection/d" + String(counter);

      // set only documentPath to delete in the Write class constructor.
      writes.add(Write(documentPath, Precondition()));
    */

    // All Writes, DocumentTransform and Values::xxxx objects can be printed on Serial port

    // You can set the content of write and writes objects directly with write.setContent("your content") and writes.setContent("your content")

    Docs.commit(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), writes, asyncCB);
    // To assign UID for async result
    // Docs.commit(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), writes, asyncCB, "myUID");

    // To get anyc result without callback
    // Docs.commit(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), writes, aResult_no_callback);
  }

  //sleep if possible
  // if (!Serial) {
  //    Watchdog.sleep(3000);
  //  } else {
  //    delay(3000);
  //  }
}