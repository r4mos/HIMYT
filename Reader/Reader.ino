#include <LiquidCrystal.h>    // http://www.arduino.cc/en/Tutorial/LiquidCrystalSerialDisplay

// OPTIONS
#define RS 12
#define EN 11
#define D4 5
#define D5 4
#define D6 3
#define D7 2
#define WIDTH 16
#define HEIGTH 2
#define SERIAL_BAUD 115200
#define NO_DATA_TIME 15000
#define MAIN_SLEEP 1000

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
void setup()
{
    Serial.begin(SERIAL_BAUD);
    lcd.begin(WIDTH, HEIGTH);
}

void print_line(String line)
{
    line = line.substring(0, WIDTH);
    if (line.charAt(line.length() - 1) == '\r')
    {
        line.remove(line.length() - 1, 1);
    }
    
    lcd.print(line);
        
    for (uint8_t column = line.length(); column < WIDTH; column++)
    {
        lcd.print(' ');
    }
}

uint32_t current_time = 0;
uint8_t current_line = 0;
void loop()
{
    if (Serial.available() > 0)
    {
        lcd.setCursor(0, current_line);
        
        print_line(Serial.readStringUntil('\n'));
        
        current_line = current_line == 0 ? 1 : 0;
        current_time = millis();
    }
    else if(millis() - current_time > NO_DATA_TIME)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        print_line("No data.");
        current_time = millis();
    }
    delay(MAIN_SLEEP);
}
