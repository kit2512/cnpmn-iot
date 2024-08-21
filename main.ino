#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32_Servo.h>


Servo myservo;

const char* ssid = "Phong 107";
const char* password = "12345679";

#define SS_PIN 21   //
#define RST_PIN 22  //

// Your Domain name with URL path or IP address with path
const char* serverName = "http://43.204.114.190/checkin/create";
int servoPin = 5;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instance of the class
unsigned long previousMillis = 5000;
unsigned long previousMillis2 = 0;
unsigned long lastTime = 0;
String OldCardID = "";
void setup() {
  myservo.attach(servoPin, 500, 2400);
  Serial.begin(115200);
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522
  Serial.println();
  Serial.print(F("Reader :"));
  mfrc522.PCD_DumpVersionToSerial();
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  //---------------------------------------------
  if (millis() - previousMillis2 >= 5000) {
    previousMillis2 = millis();
    OldCardID = "";
  }
  //---------------------------------------------
  // look for new card
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;  // go to start of loop if there is no card present
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;  // if read card serial(0) returns 1, the uid struct contains the ID of the read card.
  }
  String CardID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }
  //---------------------------------------------
  if (CardID == OldCardID) {
    return;
  } else {
    OldCardID = CardID;
  }

  // Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > previousMillis) {
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      http.begin(client, serverName);

      // Populate the JSON object with your data
      DynamicJsonDocument jsonDocument(200);
      jsonDocument["rfid_machine_id"] = 1;
      jsonDocument["card_id"] = CardID;

      String jsonString;
      serializeJson(jsonDocument, jsonString);

      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(jsonString);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("CardID: ");
      Serial.println(CardID);

      if (httpResponseCode == 200) {
        delay(900);
        myservo.write(180);
        delay(3000);
        myservo.write(0);
        delay(1000);
      }
      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  //---------------------------------------------
}
