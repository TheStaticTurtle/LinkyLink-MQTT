# LinkyLink-MQTT
This a alternative firmware of https://github.com/TurtleForGaming/LinkyLink

Instead of a nice interface you need to modify constants in the code directly. But most importently it's more stable that the main one

To upload the firmware you need to create a file named secret.h in the arduino sketch folder

```h
#define WIFI_SSID   "Your ssid"
#define WIFI_PSWD   "Yourpassword"

#define MQTT_SERVER "Your mqtt server"
#define MQTT_PORT   1883
#define MQTT_CLTID  "LinkyLinky"
#define MQTT_USER   "Mqtt server user"
#define MQTT_PSWD   "Mqtt server password"
```

You can also change these constants in the LinkyLink-MQTT.ino file:
```cpp
const int   LOGGING_INTERVAL = 5;
const char* MQTT_TOPIC_LINKY_ADCO = "/iot/linkylink/linky/ADCO";
const char* MQTT_TOPIC_LINKY_PAPP = "/iot/linkylink/linky/PAPP";
const char* MQTT_TOPIC_LASTUPDATE = "/iot/linkylink/last_update";
const char* MQTT_TOPIC_UPTIME     = "/iot/linkylink/uptime";
```
