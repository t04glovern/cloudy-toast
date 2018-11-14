<img src="https://raw.githubusercontent.com/t04glovern/cloudy-toast/master/img/project-banner.jpg" data-canonical-src="https://raw.githubusercontent.com/t04glovern/cloudy-toast/master/img/project-banner.jpg" align="center"/>

<div align = "center">
    <h1><em>Cloudy with a chance of Toast</em></h1>
    <p>Toaster Stationary box for the masses... IN THE CLOUD</p>
    <a href="https://manparvesh.mit-license.org/" target="_blank"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License"></a>
</div>

## Hardware

* 1x - [IMK 2 Slice Toaster](https://www.spotlightstores.com/kitchen-dining/kitchen-appliances/toasters/imk-2-slice-toaster/p/BP80271373) : $9.95
* 1x - [NodeMCU ESP8266](https://www.gearbest.com/transmitters-receivers-module/pp_366523.html) : $5.01
* 16x - Red, Blue, Green, Yellow LEDs

## Setup

### Cert / AWS

Create the `src/main.h` file based on `src/main.h.example` with the relevant information for your project and save it.

```cpp
#ifndef MAIN_H

// Wifi Details
const char *ssid = "YourWifiSSID";
const char *password = "YourWifiPassword";

const String thing_id = "YourThingID";

// AWS MQTT Details
char *aws_mqtt_server = "YourAWSThingID.iot.us-east-1.amazonaws.com";
char *aws_mqtt_client_id = "YourMQTTClientID";
char *aws_mqtt_thing_topic_pub = "Your/MQTT/Topic";
char *aws_mqtt_thing_topic_sub = "Your/MQTT/Topic";

#endif
```

### Uploading Certificates

You will also need to create the cert files based on the output from the CloudFormation deploy of the vending machine

```bash
openssl x509 -in certs/certificate.pem.crt -out data/cert.der -outform DER
openssl rsa -in certs/private.pem.key -out data/private.der -outform DER
openssl x509 -in certs/root-CA.pem -out data/ca.der -outform DER
```

Then upload the certificates using SPIFFS

```bash
pio run -t uploadfs
```

### MQTT Providers

### AWS IoT

For more information on AWS IoT, check out the [AWS IoT Setup](https://us-east-1.console.aws.amazon.com/iotv2/home?region=us-east-1#/connIntro) guide

## Platform IO

This project is build and run with PlatformIO. The library dependencies can be found in the `platformio.ini` file. Below is the current configuration targetting the NodeMCU varient of the ESP8266 development board.

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200

lib_deps =
    ArduinoJson@5.13.1
    PubSubClient@2.6
```

## Audio Encoding

The audio clip within the `/audio` directory is used in `main.h`. This byte stream cannot exceed the Program Memory size of the microcontroller you have chosen to use.

The tutorial on setting up the audio clip for encoding can be founds here: [http://highlowtech.org/?p=1963](http://highlowtech.org/?p=1963). It's important to note that you need a WAV/MP3 with a `sample rate of 8Khz` and a `bit rate of 16Khz`.

There's a copy of the `EncodeAudio.exe` binary in the `/audio` directory also incase the site mirror goes down.
