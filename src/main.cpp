#include "FS.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "main.h"

#define PWMRANGE 1023

const byte pinCount = 8;
const byte ledPins[pinCount] = {D1, D2, D3, D4, D5, D6, D7, D8};

const byte colourCount = 4;
String colours[colourCount] = {"Red", "Yellow", "Green", "Blue"};

const byte messageCount = 4;
String messages[messageCount] = {
    "Kill all humans",
    "You're toast!",
    "You breader watch out...",
    "I'm the yeast of your worries"
};

int led = 1;

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.println();
    Serial.print("[AWS] Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println();
}

WiFiClientSecure espClient;
PubSubClient client(aws_mqtt_server, 8883, callback, espClient); //set  MQTT port number to 8883 as per //standard
int tick = 0;

#define BAUD_RATE 115200

void working_led()
{
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(50);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(50);
}

void turnOnLED(const byte which)
{
    for (byte i = 0; i < pinCount; i++)
        analogWrite(ledPins[i], 0);

    if (which > 1)
        analogWrite(ledPins[which - 2], 1023); // make "which" zero-relative
} // end of turnOnLED

void setup_wifi()
{
    delay(100);
    working_led();
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("[WIFI] Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        working_led();
    }

    Serial.println("");
    Serial.print("[WIFI] WiFi connected on ");
    Serial.println(WiFi.localIP());
}

void setup_certs()
{
    if (!SPIFFS.begin())
    {
        Serial.println("[CERTS] Failed to mount file system");
        return;
    }

    Serial.print("[CERTS] Heap: ");
    Serial.println(ESP.getFreeHeap());

    // Load certificate file
    File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
    if (!cert)
    {
        Serial.println("[CERTS] Failed to open cert file");
    }
    else
        Serial.println("[CERTS] Success to open cert file");

    delay(200);

    if (espClient.loadCertificate(cert))
        Serial.println("[CERTS] cert loaded");
    else
        Serial.println("[CERTS] cert not loaded");

    // Load private key file
    File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
    if (!private_key)
    {
        Serial.println("[CERTS] Failed to open private cert file");
    }
    else
        Serial.println("[CERTS] Success to open private cert file");

    delay(200);

    if (espClient.loadPrivateKey(private_key))
        Serial.println("[CERTS] private key loaded");
    else
        Serial.println("[CERTS] private key not loaded");

    // Load CA file
    File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
    if (!ca)
    {
        Serial.println("[CERTS] Failed to open ca ");
    }
    else
        Serial.println("[CERTS] Success to open ca");
    delay(200);

    if (espClient.loadCACert(ca))
        Serial.println("[CERTS] ca loaded");
    else
        Serial.println("[CERTS] ca failed");
    Serial.print("[CERTS] Heap: ");
    Serial.println(ESP.getFreeHeap());
    working_led();
}

void aws_reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("[AWS] Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(aws_mqtt_client_id))
        {
            Serial.println("[AWS] connected");
            // ... and resubscribe
            client.subscribe(aws_mqtt_thing_topic_sub);
        }
        else
        {
            Serial.print("[AWS] failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            working_led();
            delay(5000);
        }
    }
}

void setup()
{
    delay(200);
    Serial.begin(BAUD_RATE);
    delay(200);
    setup_wifi();
    delay(200);
    setup_certs();
    delay(200);
    aws_reconnect();
    delay(200);
}

void loop()
{
    if (!client.connected())
    {
        aws_reconnect();
    }

    client.loop();

    if (tick >= 10) // publish to topic every 1 seconds
    {
        if (led > pinCount + 1)
            led = 1;

        turnOnLED(led++);

        tick = 0;
        working_led();

        String colour1 = colours[random(0, colourCount)];
        String colour2 = colours[random(0, colourCount)];
        String message = messages[random(0, messageCount)];

        const size_t bufferSize = JSON_OBJECT_SIZE(12) + 20;
        DynamicJsonBuffer jsonBuffer(bufferSize);

        JsonObject &root = jsonBuffer.createObject();
        root["message"] = message;
        JsonArray &colours = root.createNestedArray("colours");
        colours.add(colour1);
        colours.add(colour2);

        String json_output;

        root.printTo(json_output);
        char payload[bufferSize];

        json_output.toCharArray(payload, bufferSize);
        sprintf(payload, json_output.c_str());

        Serial.print("[AWS MQTT] Publish Message:");
        Serial.println(payload);
        client.publish(aws_mqtt_thing_topic_pub, payload);
    }
    delay(100);
    tick++;
}
