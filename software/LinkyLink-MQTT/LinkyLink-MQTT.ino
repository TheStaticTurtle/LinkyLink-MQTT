#include "ESP8266WiFi.h"
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "linky.h"

// USER VARIABLES
#include "secret.h"
const int   LOGGING_INTERVAL = 5;
const char* MQTT_TOPIC_LINKY_ADCO = "/iot/linkylink/linky/ADCO";
const char* MQTT_TOPIC_LINKY_PAPP = "/iot/linkylink/linky/PAPP";
const char* MQTT_TOPIC_LASTUPDATE = "/iot/linkylink/last_update";
const char* MQTT_TOPIC_UPTIME     = "/iot/linkylink/uptime";


//SYSTEM VARIABLES
const char* MODULE_SERIAL_NUM   = "LinkyLinkyMQTT-00001";
const char* MODULE_VERSION_HARD = "V1.0";

const int   PINS_LED_STATUS_WIFI = D3;
const int   PINS_LED_STATUS_LINK = D5;
const int   PINS_LED_STATUS_ERR  = D4;
const int   PINS_CTRL_DATA_IN    = D6;

#define MSG_BUFFER_SIZE	(50)
char msg_buffer[MSG_BUFFER_SIZE];

Linky mLinky(D7,-1,PINS_CTRL_DATA_IN);
WiFiClient espClient;
PubSubClient client(espClient);



void connectToMQTTServer() {
	Serial.print("[MQTT] Connecting ");
	while (!client.connected()) {
		delay(250);
		Serial.print("*");  
		if (client.connect(MODULE_SERIAL_NUM, MQTT_USER, MQTT_PSWD )) {
			Serial.println("Connected");  
		} else {
			Serial.print("Failed to connect with state: ");
			Serial.println(client.state());
		}
	}
}

void setup() {
	pinMode(PINS_LED_STATUS_ERR ,OUTPUT);
	pinMode(PINS_LED_STATUS_LINK,OUTPUT);
	pinMode(PINS_LED_STATUS_WIFI,OUTPUT);
	pinMode(PINS_CTRL_DATA_IN   ,OUTPUT);
	digitalWrite(PINS_CTRL_DATA_IN   ,false);

	digitalWrite(PINS_LED_STATUS_ERR ,true);
	digitalWrite(PINS_LED_STATUS_LINK,true);
	digitalWrite(PINS_LED_STATUS_WIFI,true);
	delay(500);
	digitalWrite(PINS_LED_STATUS_ERR ,false);
	digitalWrite(PINS_LED_STATUS_LINK,false);
	digitalWrite(PINS_LED_STATUS_WIFI,true);


	Serial.begin(115200);
	Serial.println();
	Serial.println();
	Serial.println("Booting up....");
	
	WiFi.hostname(MODULE_SERIAL_NUM);
		
	Serial.print("Connecting to wifi: ");

	WiFi.begin(WIFI_SSID, WIFI_PSWD);
	while (WiFi.status() != WL_CONNECTED)  {
		delay(250);
		Serial.print("*");
		digitalWrite(PINS_LED_STATUS_WIFI, !digitalRead(PINS_LED_STATUS_WIFI));
	}
	digitalWrite(PINS_LED_STATUS_WIFI, true);
	Serial.println(" Connected");

	Serial.print("[WiFi] IP Address: ");
	Serial.println(WiFi.localIP());

	if (MDNS.begin(MODULE_SERIAL_NUM)) {
		Serial.println("[MDNS] Begin ok");
	} else  { 
		Serial.println("[MDNS] Begin failed");
	}

	client.setServer(MQTT_SERVER, MQTT_PORT);

	ArduinoOTA.setHostname(MODULE_SERIAL_NUM);

	ArduinoOTA.onStart([]() { Serial.println("[OTA] Start updating"); });
	ArduinoOTA.onEnd([]() {  Serial.println("\nEnd"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100))); });
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("[OTA] Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) { Serial.println("[OTA] Auth Failed"); }
		else if (error == OTA_BEGIN_ERROR) { Serial.println("[OTA] Begin Failed"); }
		else if (error == OTA_CONNECT_ERROR) { Serial.println("[OTA] Connect Failed"); }
		else if (error == OTA_RECEIVE_ERROR) { Serial.println("[OTA] Receive Failed"); }
		else if (error == OTA_END_ERROR) { Serial.println("[OTA] End Failed"); }
	});
	ArduinoOTA.begin();
	Serial.println(F("[OTA] Begin"));


	client.publish("esp/test", "Hello from ESP8266");

	mLinky.begin();
	Serial.println(F("LinkyLink-MQTT V1 is in the place"));
}


unsigned long lastLog = 0;
void loop() {
	client.loop();
	ArduinoOTA.handle();

	if(!client.connected()) {
		Serial.println("[MQTT] Not connected");
		connectToMQTTServer();
	}

	if( mLinky.updateAsync(PINS_LED_STATUS_LINK,true) ) {
		Serial.println("[LINKY] Got frame");
		digitalWrite(PINS_LED_STATUS_ERR, false);
	}

	if (millis() - lastLog > LOGGING_INTERVAL*1000) {

		lastLog = millis();
		LinkyData linkyData = mLinky.grab();
		if(mLinky.lastFullFrame() > LOGGING_INTERVAL*1000) {
			Serial.println("Unable to connect to linky");
			digitalWrite(PINS_LED_STATUS_ERR, true);
			digitalWrite(PINS_LED_STATUS_LINK, false);

    		snprintf (msg_buffer, MSG_BUFFER_SIZE, "0");
			client.publish(MQTT_TOPIC_LINKY_PAPP, msg_buffer);
		} else {
			digitalWrite(PINS_LED_STATUS_ERR, false);
			Serial.print("Numero compteur: ");
			Serial.print(linkyData.ADCO);
			Serial.print(" | Consomation: ");
			Serial.print(linkyData.PAPP);
			Serial.println(" VA");

			client.publish(MQTT_TOPIC_LINKY_ADCO, linkyData.ADCO);

    		snprintf (msg_buffer, MSG_BUFFER_SIZE, "%ld", linkyData.PAPP);
			client.publish(MQTT_TOPIC_LINKY_PAPP, msg_buffer);
		}

    	snprintf (msg_buffer, MSG_BUFFER_SIZE, "%ld", mLinky.lastFullFrame()/1000);
		client.publish(MQTT_TOPIC_LASTUPDATE,msg_buffer);

    	snprintf (msg_buffer, MSG_BUFFER_SIZE, "%ld",  millis()/1000);
		client.publish(MQTT_TOPIC_UPTIME,msg_buffer);

	}
}