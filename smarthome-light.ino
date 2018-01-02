#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdint.h>
#include "config.h"
#include "light.h"

#define LIGHT_TOGGLE_TOPIC "home/lights/" LIGHT_ID "/toggle"
#define LIGHT_MIN_BRIGHTNESS_TOPIC "home/lights/" LIGHT_ID "/min_brightness"
#define LIGHT_MAX_BRIGHTNESS_TOPIC "home/lights/" LIGHT_ID "/max_brightness"
#define LIGHT_TRANSITION_TIME_TOPIC "home/lights/" LIGHT_ID "/transition_time"

WiFiClient wifi;
PubSubClient client(wifi);
Light light(LIGHT_PIN);

// --------------------------------------------------------------------------------

void debug_message(const char *message)
{
	client.publish("debug", message);
}

void mqtt_reconnect(void)
{
	// Attempt to connect to the MQTT server.
	while (!client.connected()) {

		// Use the built-in LED to indicate whether the ESP is connected to the MQTT server (turns off when connected).
		digitalWrite(BUILTIN_LED, LOW);

		if (client.connect("smarthome-light-" LIGHT_ID)) {
	  
			// Subscribe to the topics which control this light.
			client.subscribe(LIGHT_TOGGLE_TOPIC);
			client.subscribe(LIGHT_MIN_BRIGHTNESS_TOPIC);
			client.subscribe(LIGHT_MAX_BRIGHTNESS_TOPIC);
			client.subscribe(LIGHT_TRANSITION_TIME_TOPIC);

			// Turn off the internal LED to indicate everything's good.
			digitalWrite(BUILTIN_LED, HIGH);
		}

		else {
			// Wait 5 seconds before attempting to connect again.
			delay(5000);
		}
	}
}

void mqtt_on_message_received(char *topic, byte *payload, unsigned int length)
{
	// Toggle the light.
	if (strcmp(topic, LIGHT_TOGGLE_TOPIC) == 0) {

		//bool toggle = (*(uint8_t*)payload != 0);
		bool toggle = (*payload != '0');

		light.SetToggled(toggle);
	}

	// Set the minimum brightness of the light.
	// This is used as an override to turn the light on when it may not be activated (for example when an alarm is on).
	else if (strcmp(topic, LIGHT_MIN_BRIGHTNESS_TOPIC) == 0) {

		//uint16_t value = *(uint16_t*)payload;
		uint16_t value = (uint16_t)atoi((char*)payload);

		light.SetMinBrightness(value);
	}

	// Set the maximum brightness of the light.
	else if (strcmp(topic, LIGHT_MAX_BRIGHTNESS_TOPIC) == 0) {

		//uint16_t value = *(uint16_t*)payload;
		uint16_t value = (uint16_t)atoi((char*)payload);

		light.SetMaxBrightness(value);
	}

	// Set the transition time.
	else if (strcmp(topic, LIGHT_TRANSITION_TIME_TOPIC) == 0) {

		//uint16_t value = *(uint16_t*)payload;
		uint16_t value = (uint16_t)atoi((char*)payload);

		light.SetTransitionTime(value);
	}
}

// --------------------------------------------------------------------------------

void setup()
{
	// Enable the built-in LED for debugging purposes.
	pinMode(BUILTIN_LED, OUTPUT);

	// Setup WiFi and try to connect to the network.
	delay(10);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	bool blink = false;

	// Wait until the connection has been made. This blinks the internal LED.
	while (WiFi.status() != WL_CONNECTED) {

		digitalWrite(BUILTIN_LED, blink ? LOW : HIGH);
		blink = !blink;

		delay(500);
	}

	// Turn off the access point.
	WiFi.enableAP(false);

	// Initialize settings for the MQTT client.
	client.setServer(MQTT_SERVER, MQTT_PORT);
	client.setCallback(mqtt_on_message_received);
}

void loop()
{
	// Attempt to connect to the MQTT server if the connection is closed.
	// Once the connection is made, the internal led will turn off.
	if (!client.connected()) {
		mqtt_reconnect();
	}

	client.loop();
	light.Process();
}
