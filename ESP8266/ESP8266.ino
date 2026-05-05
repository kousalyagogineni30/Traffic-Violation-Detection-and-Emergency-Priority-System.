#include <WiFi.h>

String apiKey = "69VBMFNI8EKF6GX3";  // Enter your Write API key from ThingSpeak

const char* ssid = "project";    // Replace with your Wi-Fi SSID
const char* pass = "123456789";      // Replace with your Wi-Fi password
const char* server = "api.thingspeak.com";

WiFiClient client;

void setup() {
  Serial.begin(9600);
  delay(10);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    if (data.indexOf('$') != -1 && data.length() > 1) {
      Serial.println(data);

      int indexA = data.indexOf("$") + 1;
      int indexB = data.indexOf("b") + 1;
      int indexC = data.indexOf("c") + 1;
      int indexD = data.indexOf("d") + 1;
      int indexE = data.indexOf("e") + 1;
      int indexF = data.indexOf("f") + 1;
      int indexG = data.indexOf("g") + 1;
      int indexH = data.indexOf("h") + 1;

      String valueA = data.substring(indexA, indexB - 1);
      String valueB = data.substring(indexB, indexC - 1);
      String valueC = data.substring(indexC, indexD - 1);
      String valueD = data.substring(indexD, indexE - 1);
      String valueE = data.substring(indexE, indexF - 1);
      String valueF = data.substring(indexF, indexG - 1);
      String valueG = data.substring(indexG, indexH - 1);
      String valueH = data.substring(indexH);  // Assuming it goes to the end of the string

      Serial.println("Value a: " + valueA);
      Serial.println("Value b: " + valueB);
      Serial.println("Value c: " + valueC);
      Serial.println("Value d: " + valueD);
      Serial.println("Value e: " + valueE);
      Serial.println("Value f: " + valueF);
      Serial.println("Value g: " + valueG);
      Serial.println("Value h: " + valueH);

      if (client.connect(server, 80)) {
        String postStr = "api_key=" + apiKey;
        // String postStr = apiKey;
        postStr += "&field1=" + valueA;
        postStr += "&field2=" + valueB;
        postStr += "&field3=" + valueC;
        postStr += "&field4=" + valueD;
        postStr += "&field5=" + valueE;
        postStr += "&field6=" + valueF;
        postStr += "&field7=" + valueG;

        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        // client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(postStr.length());
        client.print("\n\n");
        client.print(postStr);
        // client.stop();

        // Thingspeak requires a minimum 15-second delay between updates
        delay(1000);  // Allow time for the server to process the request

        //   if (client.connected()) {
        //     Serial.println("Data sent to ThingSpeak successfully");
        //     client.stop();
        //   } else {
        //     Serial.println("Failed to send data to ThingSpeak");
        //   }
      }
      // else {
      //   Serial.println("Failed to connect to ThingSpeak server");
      // }

      delay(15000);  // Minimum delay between updates for ThingSpeak
    }
  }
}