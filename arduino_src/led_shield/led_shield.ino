#include "StateMachine.h"
#include "helpers.h"

// Pins
#define POTENTIOMETER_PIN 0
#define KEY_1_PIN 8
#define KEY_2_PIN 12
#define RGB_RED_PIN 11
#define RGB_GREEN_PIN 9
#define RGB_BLUE_PIN 10
#define INTERRUPT_PIN 2

// Various constants
#define DEBOUNCE_MILLIS 150
#define POTENTIOMETER_MAX 1023
#define SERIAL_PARSE_INTERVAL 500

// Instances 
struct LED diodes [3];
KEY key1;
KEY key2;
UART_ST uart_control;
StateMachine sm = StateMachine(&idle, State::IDLE);



// UART interface
char serial_feed_buffer[256];
const char WELCOME[] = "** Welcome to crispy-potato! **";
const char MENU_1[] = "** Select mode and hit Enter. Once in a mode, type 'q' and enter to quit.**\n\n";
const char MENU_2[] = "1. Led Cycle mode\n2. Rainbow mode\n3. Turn off Leds\n\n(!) Tip! Use the knob to adjust the speed";
const char CURRENT_STATE_PARTIAL[] = "\n>> Crispy's current state: ";
const char STATES_TO_TEXT[4][12] = {"Led cycle", "Led rainbow", "Off"};


void setup()
{    
    Serial.begin(9600);
    memset(serial_feed_buffer, 0, sizeof(serial_feed_buffer) / sizeof(serial_feed_buffer[0]));
    
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
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    pinMode(key1.pin, INPUT);
    pinMode(key2.pin, INPUT);
    pinMode(diodes[0].pin, OUTPUT);
    pinMode(diodes[1].pin, OUTPUT);
    pinMode(diodes[2].pin, OUTPUT);

    //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), off, HIGH);
    
    uart_control.demanded_state = sm.getCurrentState();
    
    sm.addState(&rainbow, State::LED_RAINBOW);
    sm.addState(&cycle, State::LED_CYCLE);
    sm.addState(&parseSerialCommand, State::READ_SERIAL);    
    sm.addState(&off, State::OFF);

    Serial.println(WELCOME);
    Serial.println(MENU_1);
    Serial.println(MENU_2);
}

void parseSerialBus()
{
    int count;
    
    while(Serial.available())
    {
        if (count < sizeof(serial_feed_buffer) / sizeof(serial_feed_buffer[0]))
        {
            serial_feed_buffer[count] = Serial.read();
            count++;
        }
    }
    sm.release();
}

void parseSerialCommand()
{
    size_t buf_size = sizeof(serial_feed_buffer) / sizeof(serial_feed_buffer[0]);
    parseSerialBus();

    State uart_state_before_read = uart_control.demanded_state;
    
    if (serial_feed_buffer[0] != 0)
    {
        Serial.println(serial_feed_buffer[0]);
        switch(serial_feed_buffer[0])
        {
            case '1':
                uart_control.demanded_state = State::LED_CYCLE;
                break;
            case '2':
                uart_control.demanded_state = State::LED_RAINBOW;
                break;
            case '3':
                uart_control.demanded_state = State::OFF;
                break;
            case 'q':
                uart_control.demanded_state = sm.getMainState();
                break;
        }
    }

  
    if (uart_state_before_read != uart_control.demanded_state)
    {
        if (uart_control.demanded_state == State::OFF)
        {
            Serial.print(F("Turned off all LEDs."));
        }
        else if (uart_control.demanded_state == sm.getMainState())
        {
            Serial.println(F("Quit previous mode"));
        }
        else
        {
            uart_control.active = 1;
            Serial.print(F("Entering '"));
            Serial.print(STATES_TO_TEXT[(int)uart_control.demanded_state-1]);
            Serial.print(F("' mode. To quit, press 'q' or press one of the physical buttons on Crispy.\n"));
        }        
    }    
    memset(serial_feed_buffer, 0, buf_size);
    sm.release();
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

void idle()
{
    static unsigned long last_uart_state_executed_millis;
    static unsigned long last_serial_parse_millis;

    int potentiometer_reading = getDigitalPotentiometerReading(POTENTIOMETER_PIN);

    if (assertDelayTimePassed(last_uart_state_executed_millis, potentiometer_reading) && uart_control.active)
    {
        sm.transitionTo(uart_control.demanded_state);
        last_uart_state_executed_millis = millis();
    }    

    if (assertDelayTimePassed(last_serial_parse_millis, SERIAL_PARSE_INTERVAL))
    {
        sm.transitionTo(State::READ_SERIAL);
        last_serial_parse_millis = millis();
    }
    
    if (debounceKey(&key1, DEBOUNCE_MILLIS))
    {
        if (keyOnClickEvent(&key1))
        {
            uart_control.active= 0;
            sm.transitionTo(State::LED_CYCLE);
        }  
    }  

    if (debounceKey(&key2, 1))
    {
        if (keyOnClickEvent(&key2))
        {
            uart_control.active= 0;
            sm.transitionTo(State::LED_RAINBOW);
        }
    }
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