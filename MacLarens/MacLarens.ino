#include <ESP8266WiFi.h>      // https://github.com/esp8266/Arduino -> /libraries/ESP8266WiFi/
#include <uMQTTBroker.h>      // https://github.com/martin-ger/uMQTTBroker
#include <IRremoteESP8266.h>  // https://github.com/crankyoldgit/IRremoteESP8266
#include <IRsend.h>           // https://github.com/crankyoldgit/IRremoteESP8266
#include <ir_Daikin.h>        // https://github.com/crankyoldgit/IRremoteESP8266


// OPTIONS
#define INIT_SLEEP 200
#define MAIN_SLEEP 10000
#define SERIAL_BAUD 115200
#define WIFI_SSID "HIMYT"
#define WIFI_PSK "UX7YQ^b+j=@jc&Py"
#define WIFI_CHANEL 8
#define WIFI_HIDDEN false
#define WIFI_MAX_CONNECTIONS 8
#define WIFI_IP      0x0104A8C0  //192.168.4.1
#define WIFI_GATEWAY 0x0104A8C0  //192.168.4.1
#define WIFI_SUBNET  0x00FFFFFF  //255.255.255.0
#define MQTT_MAX_DATA_SIZE 100
#define MQTT_ROOT_TOPIC "MacLaren's Pub"
#define MQTT_CHARACTERS_SIZE 5
#define MQTT_TED_CHARACTER "Ted Mosby"
#define MQTT_MARSHALL_CHARACTER "Marshall Eriksen"
#define MQTT_LILY_CHARACTER "Lily Aldrin"
#define MQTT_ROBIN_CHARACTER "Robin Scherbatsky"
#define MQTT_BARNEY_CHARACTER "Barney Stinson"
#define MQTT_SAVED_MEASUREMENTS 6
#define MQTT_TEMPERATURE_TOPIC "Temperature"
#define MQTT_HUMIDITY_TOPIC "Humidity"
#define MQTT_LAST_WISH_TOPIC "Last"
#define MQTT_THI_TOPIC "THI"
#define MQTT_AC_MODE_TOPIC "AC/Mode"
#define MQTT_AC_TEMPERATURE_TOPIC "AC/Temperature"
#define MQTT_AC_FAN_TOPIC "AC/Fan"
#define LED_PIN 2


struct Character
{
    float temperature[MQTT_SAVED_MEASUREMENTS];
    float humidity[MQTT_SAVED_MEASUREMENTS];
    float temperature_calculated;
    float humidity_calculated;
    uint8_t temperature_index;
    uint8_t humidity_index;
    String name;
};


void wifi_setup()
{
	WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(WIFI_IP, WIFI_GATEWAY, WIFI_SUBNET);
	WiFi.softAP(WIFI_SSID, WIFI_PSK, WIFI_CHANEL, WIFI_HIDDEN, WIFI_MAX_CONNECTIONS);
}


Character characters[MQTT_CHARACTERS_SIZE];
void character_setup(Character* character)
{
    character->humidity[MQTT_SAVED_MEASUREMENTS - 1] = NAN;
    character->temperature[MQTT_SAVED_MEASUREMENTS - 1] = NAN;
    character->humidity_index = 0;
    character->temperature_index = 0;
    character->humidity_calculated = NAN;
    character->temperature_calculated = NAN;
}

void characters_setup()
{
    for (uint8_t i = 0; i < MQTT_CHARACTERS_SIZE; i++)
    {
        character_setup(&characters[i]);
    }
    characters[0].name = MQTT_TED_CHARACTER;
    characters[1].name = MQTT_MARSHALL_CHARACTER;
    characters[2].name = MQTT_LILY_CHARACTER;
    characters[3].name = MQTT_ROBIN_CHARACTER;
    characters[4].name = MQTT_BARNEY_CHARACTER;
}

void character_set_data(Character * character, String type, String payload)
{
    if (type == MQTT_TEMPERATURE_TOPIC)
    {
        character->temperature[character->temperature_index++] = payload.toFloat();
        if (MQTT_SAVED_MEASUREMENTS < character->temperature_index)
        {
            character->temperature_index = 0;
        }
    }
    else if (type == MQTT_HUMIDITY_TOPIC)
    {
        character->humidity[character->humidity_index++] = payload.toFloat();
        if (MQTT_SAVED_MEASUREMENTS < character->humidity_index)
        {
            character->humidity_index = 0;
        }
    }
    else if (type == MQTT_LAST_WISH_TOPIC)
    {
        character_setup(character);
    }
}

char buffer[MQTT_MAX_DATA_SIZE];
class MQTTBroker : public uMQTTBroker
{
    public:
        virtual bool onConnect(IPAddress addr, uint16_t client_count)
        {
            return true;
        }

        virtual void onData(String topic, const char * data, uint32_t length)
        {
            if (length < MQTT_MAX_DATA_SIZE && topic.startsWith(MQTT_ROOT_TOPIC))
            {
                os_memcpy(buffer, data, length);
                buffer[length] = '\0';
                String payload = String(buffer);

                uint8_t index_init = topic.indexOf('/');
                uint8_t index_end = topic.indexOf('/', index_init + 1);
                String character = topic.substring(index_init + 1, index_end);

                bool found = false;
                for (uint8_t i = 0; !found && i < MQTT_CHARACTERS_SIZE; i++)
                {
                    if (character.equals(characters[i].name))
                    {
                        found = true;
                        character_set_data(&characters[i], topic.substring(index_end + 1), payload);
                    }
                }
            }
        }
};

MQTTBroker broker;
void mqtt_setup()
{
    broker.init();
    broker.subscribe("#");
}


IRDaikinESP ac(LED_PIN);
void ac_setup()
{
    ac.begin();
}


void serial_setup()
{
    Serial.begin(SERIAL_BAUD);
    Serial.println();
    Serial.println();
}


void setup()
{
    delay(INIT_SLEEP);
    wifi_setup();
    characters_setup();
    mqtt_setup();
    ac_setup();
    serial_setup();
}


uint8_t character_process_data(Character * character)
{
    if (!isnan(character->humidity[MQTT_SAVED_MEASUREMENTS - 1]) && !isnan(character->temperature[MQTT_SAVED_MEASUREMENTS - 1]))
    {
        float humidity_sum = 0;
        float temperature_sum = 0;
        for (uint8_t i = 0; i < MQTT_SAVED_MEASUREMENTS; i++)
        {
            humidity_sum += character->humidity[i];
            temperature_sum += character->temperature[i];
        }
        humidity_sum /= MQTT_SAVED_MEASUREMENTS;
        temperature_sum /= MQTT_SAVED_MEASUREMENTS;

        character->humidity_calculated = humidity_sum;
        character->temperature_calculated = temperature_sum;

        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t calculate_humidity_and_temperature(float * humidity, float * temperature)
{
    uint8_t count = 0;
    *humidity = 0;
    *temperature = 0;

    for (uint8_t i = 0; i < MQTT_CHARACTERS_SIZE; i++)
    {
        count += character_process_data(&characters[i]);
        *humidity += isnan(characters[i].humidity_calculated) ? 0 : characters[i].humidity_calculated;
        *temperature += isnan(characters[i].temperature_calculated) ? 0 : characters[i].temperature_calculated;
    }

    *humidity /= count;
    *temperature /= count;

    return count;
}

float calculate_ith(float * humidity, float * temperature)
{
    float ith = *temperature - 14.3;
    ith *= *humidity / 100;
    ith += 0.8 * *temperature;
    ith += 46.4;
    return ith;
}

void calculate_ac_behavior(float * ith, uint8_t * mode, uint8_t * temperature, uint8_t * fan)
{
    if (*ith < 67)
    {
        *mode = kDaikinHeat;
        *temperature = 25;
        *fan = 3;
    }
    else if (*ith < 69)
    {
        *mode = kDaikinHeat;
        *temperature = 23;
        *fan = 2;
    }
    else if (*ith < 71)
    {
        *mode = kDaikinAuto;
        *temperature = 23;
        *fan = 1;
    }
    else if (*ith < 73)
    {
        *mode = kDaikinCool;
        *temperature = 23;
        *fan = 2;
    }
    else
    {
        *mode = kDaikinCool;
        *temperature = 21;
        *fan = 3;
    }
}

void ac_send(uint8_t * mode, uint8_t * temperature, uint8_t * fan)
{
    if (ac.getMode() != *mode || ac.getTemp() != *temperature || ac.getFan() != *fan)
    {
        ac.on();
        ac.setMode(*mode);
        ac.setTemp(*temperature);
        ac.setFan(*fan);
        ac.setSwingVertical(false);
        ac.setSwingHorizontal(false);
        ac.send();
    }
}

void serial_loop(uint8_t * count, float * humidity, float * temperature, bool screen)
{
    if (screen)
    {
        Serial.print(*count);
        Serial.print("> T=");
        Serial.print(*temperature, 1);
        Serial.print(" H=");
        Serial.println(*humidity, 1);

        Serial.print("AC> M=");
        if (isnan(*humidity) || isnan(*temperature))
        {
            Serial.println("No sent");
        }
        else
        {
            char m = 'U';
            switch (ac.toCommonMode(ac.getMode()))
            {
                case stdAc::opmode_t::kOff:  m = 'O'; break;
                case stdAc::opmode_t::kAuto: m = 'A'; break;
                case stdAc::opmode_t::kCool: m = 'C'; break;
                case stdAc::opmode_t::kHeat: m = 'H'; break;
                case stdAc::opmode_t::kDry:  m = 'D'; break;
                case stdAc::opmode_t::kFan:  m = 'F'; break;
            }
            Serial.print(m);
            Serial.print(" T=");
            Serial.print(ac.getTemp());
            Serial.print(" F=");
            Serial.println((uint8_t)ac.toCommonFanSpeed(ac.getFan()));
        }
    }
    else
    {
        Serial.println();
        Serial.println("Status:");
        for (uint8_t i = 0; i < MQTT_CHARACTERS_SIZE; i++)
        {
            Serial.println("-> " + characters[i].name + ": temperature = " + String(characters[i].temperature_calculated) + " degrees Celsius and humidity = " + String(characters[i].humidity_calculated) + " %");
        }
        Serial.println(">> " + String(MQTT_ROOT_TOPIC) + ": temperature = " + String(*temperature) + " degrees Celsius and humidity = " + String(*humidity) + " %");
        Serial.println(">> AC: " + ac.toString());
    }
}


uint32_t sleep = MAIN_SLEEP;
void loop()
{
    uint32_t time = millis();

    float humidity, temperature;
    uint8_t count = calculate_humidity_and_temperature(&humidity, &temperature);
    broker.publish(String(MQTT_ROOT_TOPIC) + String("/") + String(MQTT_HUMIDITY_TOPIC), String(humidity));
    broker.publish(String(MQTT_ROOT_TOPIC) + String("/") + String(MQTT_TEMPERATURE_TOPIC), String(temperature));

    if (!isnan(humidity) && !isnan(temperature))
    {
        float ith = calculate_ith(&humidity, &temperature);
        broker.publish(String(MQTT_ROOT_TOPIC) + String("/") + String(MQTT_THI_TOPIC), String(ith));

        uint8_t ac_mode, ac_temperature, ac_fan;
        calculate_ac_behavior(&ith, &ac_mode, &ac_temperature, &ac_fan);
        broker.publish(String(MQTT_ROOT_TOPIC) + String("/") + String(MQTT_AC_MODE_TOPIC), String(ac_mode));
        broker.publish(String(MQTT_ROOT_TOPIC) + String("/") + String(MQTT_AC_TEMPERATURE_TOPIC), String(ac_temperature));
        broker.publish(String(MQTT_ROOT_TOPIC) + String("/") + String(MQTT_AC_FAN_TOPIC), String(ac_fan));

        ac_send(&ac_mode, &ac_temperature, &ac_fan);
    }
    
    serial_loop(&count, &humidity, &temperature, true);

    sleep -= millis() - time;
    delay(sleep <= MAIN_SLEEP ? sleep : INIT_SLEEP);
    sleep = MAIN_SLEEP;
}
