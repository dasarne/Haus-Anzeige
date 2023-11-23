#include "HausWifi.hpp"
#include <WiFiManager.h> // WiFiManager-Bibliothek hinzufügen
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFiManager-Objekt erstellen
WiFiManager wifiManager;

const char *serverURL = "http://192.168.7.2:2880/aktuellsteTemperatur";

void handle_data(void *parameter);

void HausWifi::init(HausUI *myHausUI)
{
    this->myHausUI = myHausUI;

    // Initialisiere WiFi-Verbindung und konfiguriere WiFiManager
    // Erstelle einen Access Point mit dem Namen "ESP32-AP"
    while (!wifiManager.autoConnect("Haus-AP"))
    {
        delay(1000);
    }

    Serial.println("Erfolgreich mit dem WLAN verbunden");

    xTaskCreate(
        handle_data,   /* Task function. */
        "handle_data", /* String with name of task. */
        10000,         /* Stack size in bytes. */
        NULL,          /* Parameter passed as input of the task */
        1,             /* Priority of the task. */
        NULL);         /* Task handle. */
}

void handle_data(void *parameter)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        float temp = -11.1;

        // HTTP GET-Anfrage senden
        http.begin(serverURL);
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                // JSON-Antwort empfangen
                String payload = http.getString();

                // JSON-Antwort parsen
                StaticJsonDocument<128> doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (error)
                {
                    Serial.print("deserializeJson() failed: ");
                    Serial.println(error.c_str());
                }
                else
                {
                    JsonObject root_0 = doc[0];
                    int root_0_id = root_0["id"];         // 21597
                    const char *root_0_ts = root_0["ts"]; // "2023-11-09T14:15:40.000Z"
                    temp = root_0["value"];               // 10.4
                    // tempTFT.updateNeedle(temp, 0);

                    Serial.println("Die Außen-Temperatur:");
                    Serial.println(temp);
                }
            }
            else
            {
                Serial.println("Fehler bei der Anfrage");
            }
        }
        else
        {
            Serial.println("Fehler bei der Verbindung zum Server");
        }

        http.end();
    }
    delay(10000); // Warte 10 Sekunden, bevor die nächste Anfrage gesendet wird
}
