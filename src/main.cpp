#include <LittleFS.h>
#include <service/FSUpdate.hpp>

const char *ssid = "Signature Smart Hotel";
const char *password = "smarthotelbali";
const char *urlJson = "https://raw.githubusercontent.com/EdySurya/HTTP_OTA_ESP/LittleFS/data/CurrentVersion.json";

void initLittleFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

void readFile(const char *path, const char *mode)
{
  File file = LittleFS.open(path, mode);
  while (file.available())
  {
    Serial.write(file.read());
  }
  Serial.println("");
  file.close();
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }
  Serial.println("Connected to the WiFi network");
  initLittleFS();
  readFile("/CurrentVersion.json", "r");
  File file = LittleFS.open("/CurrentVersion.json", "r");
  if (file.available())
  {
    /* code */
    deserializeJson(docVer, file);
    my_Struct.currentFirmVers = docVer["FirmVers"].as<String>();
    my_Struct.currentFSVers = docVer["FSVers"].as<String>();
    Serial.println("Firmware Current Version :" + (my_Struct.currentFirmVers));
    Serial.println("FS Current Version :" + (my_Struct.currentFSVers));
  }
  file.close();
}
unsigned long intervalRead = 0;
uint8_t i = 1;
void loop()
{
  if (millis() - intervalRead >= 1000)
  {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure server and url
    http.useHTTP10(true);
    http.begin(Client,urlJson);
    // start connection and send HTTP header
    my_Struct.httpCode = http.GET();
    if (my_Struct.httpCode > 0)
    {
      Serial.printf("[HTTP] GET... code: %d\n", my_Struct.httpCode);
      deserializeJson(doc, http.getStream());
      Serial.print("FS Version : ");
      Serial.println(doc["FSVers"].as<String>());
      Serial.print("Firmware Version : ");
      Serial.println(doc["FirmVers"].as<String>());
      Serial.println("Firmware Current Version :" + String(my_Struct.currentFirmVers));
      Serial.println("FS Current Version :" + String(my_Struct.currentFSVers));
      if (doc["FSVers"].as<String>() != my_Struct.currentFSVers)
      {
        my_Struct.currentFSVers = doc["FSVers"].as<String>();
        Serial.println("Memulai Update Versi baru dari Little FS");
        UpdateFS();
      }
      else if (doc["FirmVers"].as<String>() != my_Struct.currentFirmVers)
      {
        my_Struct.currentFirmVers = doc["FirmVers"].as<String>();
        Serial.println("Memulai Update Versi baru Firmware");
        UpdateFirm();
      }
      readFile("/CurrentVersion.json", "r");
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(my_Struct.httpCode).c_str());
    }
    http.end();
    intervalRead = millis();
  }
}