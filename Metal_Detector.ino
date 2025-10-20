#define BLYNK_TEMPLATE_ID "TMPL6CDFGHUrm"
#define BLYNK_TEMPLATE_NAME "Embedded and IOT Project"
#define BLYNK_AUTH_TOKEN "D_tysTydcSHnd6382dcRaWHwnu-iWCH_"

#include <ESP32Servo.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// WiFi
const char* WIFI_SSID = "AIM08";
const char* WIFI_PASS = "fysl2001ran";

// Firebase
const char* FIREBASE_DB_URL = "https://embedded-project-9c56a-default-rtdb.firebaseio.com";
WiFiClientSecure client;

#define TRIG_PIN 5
#define ECHO_PIN 18
#define MOTOR_PIN 4
#define PROXIMITY_PIN 27
#define SERVO_PIN 32

// Blynk Virtual Pins
#define VPIN_TODAY_COUNT   V1
#define VPIN_LAST_DETECT   V2
#define VPIN_TOTAL_COUNT   V4
#define VPIN_SYSTEM_SWITCH V3

// Global Variables
long todayCount = 0;
bool SYSTEM_ACTIVE = false; // initially OFF
bool sensorPrevState = HIGH; // sensor previous state

// Motor timing
unsigned long motorStopStart = 0;
bool motorStopped = false;

// Non-blocking timers
unsigned long lastSensorCheck = 0;
const unsigned long SENSOR_INTERVAL = 50; // ms

Servo myServo;

// Calibration and system state variables
float baselineDistance = 2;
bool objectDetected = false;
bool systemBusy = false;
unsigned long motorStopTime = 0;
const int MOTOR_DELAY = 50; // Delay in ms for object to reach proximity sensor

// Helper Functions
void initTime(){
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    struct tm timeinfo;
    int tries = 0;
    while(!getLocalTime(&timeinfo) && tries < 20){
        delay(500);
        tries++;
    }
}

void getTimestamp(String &dateStr, String &isoTs){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        time_t now = time(nullptr);
        struct tm *t = gmtime(&now);
        timeinfo = *t;
    }
    char bufDate[16];
    char bufISO[32];
    strftime(bufDate, sizeof(bufDate), "%Y-%m-%d", &timeinfo);
    strftime(bufISO, sizeof(bufISO), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    dateStr = String(bufDate);
    isoTs = String(bufISO);
}

// Firebase Functions
long firebaseGetLong(String path){
    String url = String(FIREBASE_DB_URL) + path + ".json";
    HTTPClient https;
    https.begin(client, url);
    int code = https.GET();
    if(code == 200){
        String payload = https.getString();
        https.end();
        payload.trim();
        if(payload == "null") return -1;
        return payload.toInt();
    }
    https.end();
    return -1;
}

bool firebasePut(String path, String json){
    String url = String(FIREBASE_DB_URL) + path + ".json";
    HTTPClient https;
    https.begin(client, url);
    https.addHeader("Content-Type","application/json");
    int code = https.PUT(json);
    https.end();
    return (code >=200 && code<300);
}

bool firebasePost(String path, String json){
    String url = String(FIREBASE_DB_URL) + path + ".json";
    HTTPClient https;
    https.begin(client, url);
    https.addHeader("Content-Type","application/json");
    int code = https.POST(json);
    https.end();
    return (code >=200 && code<300);
}

long firebaseGetTotalCount(){
    String url = String(FIREBASE_DB_URL) + "/detections.json";
    HTTPClient https;
    https.begin(client, url);
    int code = https.GET();
    long total = 0;
    if(code==200){
        String payload = https.getString();
        https.end();
        int idx = 0;
        while(true){
            int countPos = payload.indexOf("\"count\":", idx);
            if(countPos==-1) 
              break;
            int commaPos = payload.indexOf(",", countPos);
            if(commaPos==-1) 
              commaPos = payload.indexOf("}", countPos);
            String cstr = payload.substring(countPos+8, commaPos);
            cstr.trim();
            total += cstr.toInt();
            idx = commaPos+1;
        }
    } 
    else 
      https.end();
    return total;
}

// Detection Handler
void handleDetection(){
    String dateStr, isoTs;
    getTimestamp(dateStr, isoTs);

    // Push timestamp
    String entriesPath = "/detections/" + dateStr + "/entries";
    String json = "{\"ts\":\""+isoTs+"\"}";
    firebasePost(entriesPath, json);

    // Update today's count
    String countPath = "/detections/" + dateStr + "/count";
    long cur = firebaseGetLong(countPath);
    if(cur<0) 
      cur=0;
    cur++;
    firebasePut(countPath, String(cur));
    todayCount = cur;

    // Updating Blynk
    long total = firebaseGetTotalCount();
    Blynk.virtualWrite(VPIN_TODAY_COUNT, todayCount);
    Blynk.virtualWrite(VPIN_LAST_DETECT, isoTs);
    Blynk.virtualWrite(VPIN_TOTAL_COUNT, total);

    Serial.printf("Metal Detected at %s | Today: %ld | Total: %ld\n", isoTs.c_str(), todayCount, total);
}

// Blynk Button
BLYNK_WRITE(VPIN_SYSTEM_SWITCH){
    SYSTEM_ACTIVE = param.asInt();
    if(!SYSTEM_ACTIVE){
      Serial.println("System stopped! Motor OFF, detection paused.");
      digitalWrite(MOTOR_PIN,LOW);
    } 
    else
      Serial.println("System started!");
}

void setup(){
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(PROXIMITY_PIN, INPUT);

  // Connecting WiFi
  Serial.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while(WiFi.status()!=WL_CONNECTED){
      delay(500);
      Serial.print(".");
  }
  Serial.println("Connected");

  // Init NTP + Firebase
  client.setInsecure();
  initTime();

  // Init Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

  // Init today's count
  String dateStr, isoTs;
  getTimestamp(dateStr, isoTs);
  long cur = firebaseGetLong("/detections/"+dateStr+"/count");
  if(cur<0) 
    cur=0;
  todayCount = cur;
  Blynk.virtualWrite(VPIN_TODAY_COUNT, todayCount);
  Blynk.virtualWrite(VPIN_TOTAL_COUNT, firebaseGetTotalCount());

  myServo.attach(SERVO_PIN);
  myServo.write(90); // Start at 90° (neutral position)
  
  Serial.println("=== System Started ===");
  Serial.println("Calibrating baseline distance...");
  Serial.println("System ready. Motor running.");
}

void loop(){
  Blynk.run();

  if(!SYSTEM_ACTIVE){
    digitalWrite(MOTOR_PIN, LOW); // Ensure motor is OFF
    return; // system paused
  }

  digitalWrite(MOTOR_PIN, HIGH);
  delay(100);
  digitalWrite(MOTOR_PIN, LOW);

  float distance = getDistance();
  
  // If system is busy processing, don't check for new objects
  if(distance >0 && distance <baselineDistance){
    Serial.print("OBJECT DETECTED: ");
    Serial.print(distance);
    Serial.println(" cm  ");
    Serial.println("Motor continuing briefly...    ");

    digitalWrite(MOTOR_PIN, HIGH); // Stop motor
    delay(100); // Stabilize
    digitalWrite(MOTOR_PIN, LOW); 
    
    // Check proximity sensor for metal detection
    int proximityState = digitalRead(PROXIMITY_PIN);
    
    if(proximityState == LOW){
      // Metal detected
      Serial.println("METAL DETECTED after soner sensor- Pushing LEFT (0°)");
      handleDetection(); // Log to Firebase and Blynk
      myServo.write(0);
      delay(1000);
      myServo.write(90);
      delay(500);
    } 
    else{
      // Non-metal detected
      Serial.println("NON-METAL DETECTED - Pushing RIGHT (180°)");
      myServo.write(180);
      delay(1000);
      myServo.write(90);
      delay(500);
    }
  }
  
  int proximityState = digitalRead(PROXIMITY_PIN);
    
  if(proximityState == LOW){
    // Metal detected
    Serial.println("METAL DETECTED without sensor - Pushing LEFT (0°)");
    handleDetection(); // Log to Firebase and Blynk
    digitalWrite(MOTOR_PIN, LOW);
    delay(100);        
    myServo.write(0);
    delay(1000);
    myServo.write(90);
    delay(500);
  }
  
  delay(50); // Loop delay for stability
}

// Function to measure distance in cm
float getDistance(){
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30 ms timeout
 
  if(duration == 0) 
    return -1; // No echo received
  return (duration * 0.0343) / 2;
}
