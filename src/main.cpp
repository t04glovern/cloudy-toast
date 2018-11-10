#include "FS.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "main.h"

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("[AWS] Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
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
}

void loop()
{
    if (!client.connected())
    {
        aws_reconnect();
    }

    client.loop();

    if (tick >= 10) // publish to topic every 10 seconds
    {
        tick = 0;
        working_led();

        const size_t bufferSize = JSON_OBJECT_SIZE(12) + 20;
        DynamicJsonBuffer jsonBuffer(bufferSize);

        JsonObject &root = jsonBuffer.createObject();
        JsonArray &data = root.createNestedArray("demo");
        data.add("hello world");

        String json_output;

        root.printTo(json_output);
        char payload[bufferSize];

        json_output.toCharArray(payload, bufferSize);
        sprintf(payload, json_output.c_str());

        Serial.print("[AWS MQTT] Publish Message:");
        Serial.println(payload);
        client.publish(aws_mqtt_thing_topic_pub, payload);
    }
    delay(1000);
    tick++;
}
