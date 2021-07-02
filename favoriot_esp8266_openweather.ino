#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

const char* ssid = "wifi id";
const char* password = "wifi password";
const String myDevice = "deviceDefault@favoriot"; 
char server[] = "apiv2.favoriot.com";


int status = WL_IDLE_STATUS;
WiFiClient client;

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "open weather API key";
// Example:
//String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd4";

// Replace with your country code and city
String city = "location NAME based on Openweather website";
String countryCode = "country CODE based on openweather";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 20000;
 long temp3,humid,wspeed;// set new variable to be able show data on Favoriot Platform

String jsonBuffer;

void setup() {
 
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
}
void loop() {
  long temp3 = 0,wspeed = 0,humid=0;
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);
      Serial.print("Temperature: ");
      Serial.println(myObject["main"]["temp"]);
      const double temp = myObject["main"]["temp"];
      Serial.print("Pressure: ");
      Serial.println(myObject["main"]["pressure"]);
      const double pressure = myObject["main"]["pressure"];
      Serial.print("Humidity: ");
      Serial.println(myObject["main"]["humidity"]);
      humid = myObject["main"]["humidity"];
      Serial.print("Wind Speed: ");
      Serial.println(myObject["wind"]["speed"]);
      wspeed = myObject["wind"]["speed"];
      temp3 = temp - 273.15;//change temp value from Kalvin to Celsius
      Serial.print("Temperature Celsius :");
      Serial.print(temp3);
      Serial.println(" C");
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
// }
  String json = "{\"device_developer_id\":\""+myDevice+"\",\"data\":{\"Temperature:\":\""+temp3+"\",\"Humid:\":\""+humid+"\",\"Wind Speed:\":\""+wspeed+"\" }}";
  //String json = "Data" + temp3;
  Serial.println(json);
  if (client.connect(server, 80)) {
    client.println("POST /v2/streams HTTP/1.1");
    client.println("Host: apiv2.favoriot.com");

    client.println(F("apikey:rw-apikey/accesstoken"));  // change it!
    
    client.println("Content-Type: application/json");
    client.println("cache-control: no-cache");
    client.print("Content-Length: ");
    int thisLength = json.length();
    client.println(thisLength);
    client.println("Connection: close");
    client.println();
    client.println(json);
  }
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  if (!client.connected()) {
    client.stop();
  }
  delay(5000);
  }
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
