//Defining the variables that will be used to connect the Arduino to the Blynk
#define BLYNK_TEMPLATE_ID "TMPL2QjHyzHH2"
#define BLYNK_TEMPLATE_NAME "Greenhouse System"
#define BLYNK_AUTH_TOKEN "VI9bsrUKgS2oHM7xQ_EMbMuCB8XF3820"


//including the DHT11 library
#include <DHT.h>
#include <DHT_U.h>

//including the Blynk Library
#include <BlynkSimpleEsp32.h>


//including the WiFi Library
#include <WiFi.h>
#include <WiFiClient.h>


//WiFi details
char ssid[] = "Artemis";
char pass[] = "Mt0lymp3";
char auth[] = BLYNK_AUTH_TOKEN;

//defining the pin for DHT11
#define DHTpin 23
#define DHTtype DHT11
DHT dht(DHTpin, DHTtype);

//defining the pins for the motor drivers and for the water level and soil humidity sensor
#define motorIn1 27
#define motorIn2 26
#define motorIn3 18
#define motorIn4 19
#define motorENA 14
#define motorENB 13
#define waterPin 17
#define soilPin 35

//declaring variables for water and soil sensor
float water;
float soil;

//defining variables for pwm
const int freq = 30000;
const int resolution = 8;
int speed = 255;  //to be kept between 0 and 255

//defining the virtual buttons
#define Vhumidity V1
#define Vtemp V2
#define VWaterButton V3
#define VSoilButton V4
#define VWater V5
#define VSoil V6
#define Vplants V10

//defining the variables for temperature and humidity
float temperature;
float humidity;
char plants[] = "There is no plants that work in these conditions.";

//defining the variables for soil humidity and water level buttons
int BoolSoil;
int BoolWater;
int SoilButton;
BLYNK_WRITE(VSoilButton){
  SoilButton = param.asInt();
}
int WaterButton;
BLYNK_WRITE(VWaterButton){
  WaterButton = param.asInt();
}

//define timer for Blynk
BlynkTimer timer;

//defining the pins and variables for the water level sensor
#define PowerPin 22
#define SignalPin 35
float WaterLevel = 0;

//defining soil moisture pin and variables
#define SoilPin 34

//connecting virtual pins to blynk
BLYNK_CONNECTED() {
  Blynk.syncVirtual(Vhumidity);
  Blynk.syncVirtual(Vtemp);
  Blynk.syncVirtual(VWaterButton);
  Blynk.syncVirtual(VSoilButton);
  Blynk.syncVirtual(VWater);
  Blynk.syncVirtual(VSoil);
}

//function for masuring temp and humidity
void DHTmeasure() {
  //reading temperature and humidity
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  //writing the humidity and temperature to Blynk
  Blynk.virtualWrite (Vhumidity, humidity);
  Blynk.virtualWrite (Vtemp, temperature);

  //delay for 100 milliseconds (0.1 seconds)
  delay(100);
}

//function for measuring water level
void measurewater() {
  //making the scissors lift go down
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);
  ledcWrite(motorENA, speed);
  delay(3000);

  //making the scissors lift stop moving
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);

  ///checking the water level
  digitalWrite(PowerPin, HIGH);
  delay(10);
  WaterLevel = analogRead(SignalPin);
  digitalWrite(PowerPin, LOW);
  Serial.println(WaterLevel);

  //sending the water level to Blynk
  float level = map(WaterLevel, 0, 521, 0, 100);
  Blynk.virtualWrite(VWater, level);
  delay(500);

  //making the scissors lift go back up
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, HIGH);
  ledcWrite(motorENA, speed);
  delay(3000);

  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);

  WaterButton = 0;
}

//function for measuring soil level
void measuresoil() {
  //making the scissors lift go down
  digitalWrite(motorIn3, HIGH);
  digitalWrite(motorIn4, LOW);
  ledcWrite(motorENB, speed);
  delay(3000);

  //making the scissors lift stop moving
  digitalWrite(motorIn3, LOW);
  digitalWrite(motorIn4, LOW);

  ///checking the soil moisture
  float moisture = analogRead(SoilPin);
  Serial.println (moisture);
  float soil = map(moisture, 0, 4095, 0, 100);
  Blynk.virtualWrite(VSoil, soil);
  delay(500);

  //making the scissors lift go back up
  digitalWrite(motorIn3, LOW);
  digitalWrite(motorIn4, HIGH);
  ledcWrite(motorENB, speed);
  delay(3000);

  digitalWrite(motorIn3, LOW);
  digitalWrite(motorIn4, LOW);

  SoilButton = 0;
}

//setting up serial, Blynk, Wifi and dht
void setup() {
  Blynk.begin(auth, ssid, pass);
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, pass);
  Blynk.config(auth);
  
  //setting up the pin modes for the motors
  pinMode (motorIn1, OUTPUT);
  pinMode (motorIn2, OUTPUT);
  pinMode (motorIn3, OUTPUT);
  pinMode (motorIn4, OUTPUT);
  pinMode (motorENA, OUTPUT);
  pinMode (motorENB, OUTPUT);


  //setting up the pin modes for the water level sensor and the soil humidity sensor
  pinMode (waterPin, INPUT);

//setting up the motor enable pins
  ledcAttach(motorENA, freq, resolution);
  ledcAttach(motorENB, freq, resolution);
  
  //Configuring ADC
  analogSetAttenuation(ADC_11db);

  //setting up the pin modes for the water level sensor
  pinMode(PowerPin, OUTPUT);
  digitalWrite(PowerPin, LOW);

  //setting up the soil moisture sensor
  pinMode(SoilPin, INPUT);
}

//main loop
void loop() {
  DHTmeasure();
  Blynk.run();
  timer.run();

  Serial.println(WaterButton);

  if (WaterButton == 1) {
    measurewater() ;
    Blynk.virtualWrite(VWaterButton, 0);
  }
  if (SoilButton == 1) {
    measuresoil() ;
    Blynk.virtualWrite(VSoilButton, 0);
  }
  delay (1000);
}