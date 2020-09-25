#include <ESP8266WiFi.h>   // https://github.com/esp8266/Arduino -> /libraries/ESP8266WiFi/
#include <DHTesp.h>        // https://github.com/beegee-tokyo/DHTesp
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient


// OPTIONS
#define MAIN_SLEEP 10000
#define RETRIES_SLEEP 200
#define WIFI_SSID "HIMYT"
#define WIFI_PSK "Change me, I'm your wifi password"
#define DHT_PIN 2
#define MQTT_SERVER "192.168.4.1"
#define MQTT_PORT 1883
#define MQTT_TIMEOUT 60
#define MQTT_TED_CHARACTER "Ted Mosby"
#define MQTT_MARSHALL_CHARACTER "Marshall Eriksen"
#define MQTT_LILY_CHARACTER "Lily Aldrin"
#define MQTT_ROBIN_CHARACTER "Robin Scherbatsky"
#define MQTT_BARNEY_CHARACTER "Barney Stinson"
#define MQTT_ID MQTT_TED_CHARACTER
#define MQTT_MESSAGE_IN MQTT_ID " is back!"
#define MQTT_MESSAGE_OUT MQTT_ID " is out!"
#define MQTT_ROOT_TOPIC "MacLaren's Pub"
#define MQTT_SUBSCRIBE_TOPIC MQTT_ROOT_TOPIC "/" MQTT_ID
#define MQTT_TEMPERATURE_PUBLISH_TOPIC MQTT_ROOT_TOPIC "/" MQTT_ID "/Temperature"
#define MQTT_HUMIDITY_PUBLISH_TOPIC MQTT_ROOT_TOPIC "/" MQTT_ID "/Humidity"
#define MQTT_LAST_WISH_PUBLISH_TOPIC MQTT_ROOT_TOPIC "/" MQTT_ID "/Last"


void wifi_connect_loop()
{
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(RETRIES_SLEEP);
	}
}

void wifi_setup()
{
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PSK);
	wifi_connect_loop();
}


DHTesp dht;
TempAndHumidity dht_get_values_loop()
{
	TempAndHumidity values = dht.getTempAndHumidity();
	while (dht.getStatus() != DHTesp::DHT_ERROR_t::ERROR_NONE)
	{
		delay(RETRIES_SLEEP);
		values = dht.getTempAndHumidity();
	}
	return values;
}

void dht_setup()
{
	dht.setup(DHT_PIN, DHTesp::DHT11);
}


WiFiClient mqtt_wifi;
PubSubClient mqtt_client(mqtt_wifi);

void mqtt_connect_loop()
{
    while (!mqtt_client.connected())
	{
		if (mqtt_client.connect(MQTT_ID, MQTT_LAST_WISH_PUBLISH_TOPIC, 0, false, MQTT_MESSAGE_OUT))
		{
			mqtt_client.publish(MQTT_ROOT_TOPIC, MQTT_MESSAGE_IN);
			mqtt_client.subscribe(MQTT_SUBSCRIBE_TOPIC);
		}
		else
		{
			delay(RETRIES_SLEEP);
		}
	}
}

void mqtt_send(TempAndHumidity values)
{
	mqtt_client.loop();
	mqtt_client.publish(MQTT_TEMPERATURE_PUBLISH_TOPIC, String(values.temperature).c_str());
	mqtt_client.publish(MQTT_HUMIDITY_PUBLISH_TOPIC, String(values.humidity).c_str());
}

void mqtt_callback(char * topic, byte * payload, unsigned int length)
{
	payload[length] = '\0';
	String message = String("Did someone say '") + String((char *)payload) + String("'?");
	mqtt_client.publish(MQTT_ROOT_TOPIC, message.c_str());
}

void mqtt_setup()
{
	mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
	mqtt_client.setKeepAlive(MQTT_TIMEOUT >> 1);
	mqtt_client.setSocketTimeout(MQTT_TIMEOUT >> 1);
	mqtt_client.setCallback(mqtt_callback);
}


void setup()
{
	delay(RETRIES_SLEEP);
	wifi_setup();
	dht_setup();
	mqtt_setup();
}


uint32_t sleep = MAIN_SLEEP;
void loop()
{
	uint32_t time = millis();

	wifi_connect_loop();
	TempAndHumidity values = dht_get_values_loop();
	mqtt_connect_loop();
	mqtt_send(values);

	sleep -= millis() - time;
	delay(sleep <= MAIN_SLEEP ? sleep : RETRIES_SLEEP);
	sleep = MAIN_SLEEP;
}
