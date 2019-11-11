// Libraries
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <DHT.h>

// Firebase Configuration
#define FIREBASE_HOST "smart-irrigation-system-3483d.firebaseio.com"
#define FIREBASE_AUTH "LlVDqZCt3suDnJuRHwBXvCatuuHUFG256hqGLsBi"

// Wifi Configuration
#define WIFI_SSID "NISHANT"
#define WIFI_PASSWORD "12345786"

// DHT Sensore Configuration
#define DHTTYPE DHT11
#define DHTPin D1
DHT dht(DHTPin, DHTTYPE);

// Relay pin Define 
#define RelayPin D2

//Soil Sensor Pin Cofiguer
#define Soil_Pin A0

// Setup function
void setup() {
  Serial.begin(9600);
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Step 1: Connecting to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  // Step 2: Connecting Firebase DB
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Step 3: Connecting Sensores
  dht.begin();
  pinMode(RelayPin, OUTPUT);
  //pinMode(Soil_Pin, INPUT);

  digitalWrite(LED_BUILTIN, LOW);
}


int pres = 0;
void loop() {
  // Step 4: Sensores Read Data
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();
  float soil = analogRead(Soil_Pin);
  soil=map(soil,0,982,148,0);
  pres = soil;
  if(soil > 100)
  {   soil=100;  }
  else if(soil < 0)  
  { soil = 0;}

  int Local_Motor_Status;

  // Step 5: Chechking Sensore Read Data or not
  if (isnan(humidity) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
    }
  else if (isnan(soil)) {
    Serial.println(F("Failed to read from Soil sensor!"));
    return;
    }
    
  // Step 6 : Sensore Readed Data Send on Firebase 
  Firebase.setFloat("ESP8266/Humidity",humidity);
  Firebase.setFloat("ESP8266/Temp",temp);
  Firebase.setFloat("ESP8266/Soil_moisture",soil);
  Firebase.setString("ESP8266/Wifi_Setup/Wifi_Name",WIFI_SSID);
  
  
  // Step 7 : Firebase Data Check Data Send Or not
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
    
  // Step 8: Menually Motor ON/OFF
  int Web_Motor_Status = Firebase.getInt("ESP8266/Motor_state"); 
  
  if(Web_Motor_Status == 0)
  {
    digitalWrite(RelayPin, LOW);
    Local_Motor_Status = 0;
  }
  if(Web_Motor_Status == 1)
  {
    digitalWrite(RelayPin, HIGH);
    Local_Motor_Status = 1;
  }

  //Step 9 : Condition For Auto Mode Motor 
  if(Web_Motor_Status == 2)
  {
    if(((humidity <= 60)&&(humidity >= 140)) || ((temp <= 5)&&(temp >= 40)) || (soil <= 10))
    {
      digitalWrite(RelayPin, HIGH);
      Local_Motor_Status = 1;
    }
    else
     {
      digitalWrite(RelayPin, LOW);
        Local_Motor_Status = 0;
     }
  }

  Firebase.setInt("ESP8266/Local_Motor_Status",Local_Motor_Status);
  delay(500);
  

}
