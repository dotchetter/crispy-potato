#include "StateMachine.h"
#include "States.h"
#include "helpers.h"

#define POTENTIOMETER_PIN 0
#define KEY_1_PIN 8
#define KEY_2_PIN 12
#define RGB_RED_PIN 11
#define RGB_GREEN_PIN 9
#define RGB_BLUE_PIN 10

#define DEBOUNCE_MILLIS 180
#define POTENTIOMETER_MAX 1023

struct LED diodes [3];

KEY key1;
KEY key2;

StateMachine sm = StateMachine(&idle, State::IDLE);

void setup()
{
    key1.pin = KEY_1_PIN;
    key1.last_press_millis = 0;

    key2.pin = KEY_2_PIN;
    key2.last_press_millis = 0;

    diodes[0].pin = RGB_RED_PIN;
    diodes[0].active = 0;
    diodes[0].fade_direction = FADE_DIRECTION::DOWN;

    diodes[1].pin = RGB_GREEN_PIN;
    diodes[1].active = 0;
    diodes[1].fade_direction = FADE_DIRECTION::DOWN;

    diodes[2].pin = RGB_BLUE_PIN;
    diodes[2].active = 0;
    diodes[2].fade_direction = FADE_DIRECTION::DOWN;

    pinMode(POTENTIOMETER_PIN, INPUT);
    pinMode(key1.pin, INPUT);
    pinMode(key2.pin, INPUT);
    pinMode(diodes[0].pin, OUTPUT);
    pinMode(diodes[1].pin, OUTPUT);
    pinMode(diodes[2].pin, OUTPUT);
    
    Serial.begin(9600);

    sm.addState(&rainbow, State::LED_RAINBOW);
    sm.addState(&cycle, State::LED_CYCLE);
}


void idle()
{
    if (debounceKey(&key1, DEBOUNCE_MILLIS))
    {
        if (keyOnClickEvent(&key1))
        {
            sm.transitionTo(State::LED_CYCLE);
        }  
    }  

    if (debounceKey(&key2, 1))
    {
        if (keyOnClickEvent(&key2))
        {
            sm.transitionTo(State::LED_RAINBOW);
        }
    }
}

void rainbow()
{
    static byte last_led;
    static FADE_DIRECTION last_direction;
    size_t diodes_length = sizeof(diodes) / sizeof(diodes[0]);
    static unsigned long last_run_millis;
    int potentiometer_controlled_wait = analogRead(POTENTIOMETER_PIN) / 100;
    
    LED* current_diode;
    LED* previous_diode;
    LED* next_diode;

    if (!assertDelayTimePassed(last_run_millis, potentiometer_controlled_wait))
    {
        return;
    }

    for (byte i = last_led; i < diodes_length; i++)
    {
        current_diode = &diodes[i];

        if (i > 0)
        {
            previous_diode = &diodes[i-1];
        }
        else 
        {
            previous_diode = &diodes[0];
        }

        if (i != diodes_length)
        {
            next_diode = &diodes[i+1];
        }
        else
        {
            next_diode = &diodes[0];
        }
        
        fadeLedStateful(previous_diode);
        fadeLedStateful(current_diode);
        fadeLedStateful(next_diode);
       
        analogWrite(current_diode->pin, current_diode->analog_value);
        analogWrite(current_diode->pin, previous_diode->analog_value);
        analogWrite(current_diode->pin, next_diode->analog_value);
    }
   
    if (current_diode->fade_direction != last_direction && last_led < diodes_length)
    {
        last_led++;
    }
    else
    {
        last_led = 0;
    }

    last_run_millis = millis();
    sm.release();
}

void cycle()
{
    int potentiometer_reading = getDigitalPotentiometerReading(POTENTIOMETER_PIN);
    
    if (diodes[0].active)
    {
        diodes[0].active = 0;
        diodes[1].active = 1;
        digitalWrite(diodes[0].pin, diodes[0].active);
        analogWrite(diodes[1].pin, potentiometer_reading);
    }
    else if (diodes[1].active)
    {
        diodes[1].active = 0;
        diodes[2].active = 1;
        digitalWrite(diodes[1].pin, diodes[1].active);
        analogWrite(diodes[2].pin, potentiometer_reading);
    }
    else if (diodes[2].active)
    {
        diodes[2].active = 0;
        diodes[0].active = 1;
        digitalWrite(diodes[2].pin, diodes[2].active);
        analogWrite(diodes[0].pin, potentiometer_reading);
    }
    else
    {
        diodes[0].active = 1;
        digitalWrite(diodes[0].pin, 1);
    }
    sm.release();
}

void off()
{
    for (byte i = 0; i < sizeof(diodes) / sizeof(diodes[0]); i++)
    {
        diodes[i].active = 0;
        diodes[i].analog_value = 0;
        diodes[i].fade_direction = FADE_DIRECTION::UP;
        digitalWrite(diodes[i].pin, diodes[i].active);
    }
    sm.release();
}

void loop()
{
    sm.next()();
}
