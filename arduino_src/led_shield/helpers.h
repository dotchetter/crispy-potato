
void idle();
void rainbow();
void cycle();

typedef enum
{
    UP,
    DOWN
}FADE_DIRECTION;

struct LED
{
    byte pin;
    byte active;
    byte analog_value;
    FADE_DIRECTION fade_direction;
    unsigned long last_updated_millis;
};

struct KEY
{
    byte pin;
    unsigned long last_press_millis;
};

struct UART_ST
{
    State demanded_state;
    byte active;
};

const byte debounceKey(KEY *key, int delay_time)
{
    return (millis() - key->last_press_millis) > delay_time;
}


const byte keyOnClickEvent(KEY *key)
{
    /*
        Return 1 if key was pressed, 0 if not.
        Update the last_press_millis timestamp
        for the KEY instance.
    */
    if (digitalRead(key->pin))
    {
        key->last_press_millis = millis();
        return 1;
    }
    return 0;
}


const byte assertDelayTimePassed(unsigned long comparison_millis, unsigned long delay_millis)
/*
param comparison_millis:
measurement of millis() timestamp to compare with
param delay_millis
the time which must have passed for the comparision to return 1
returns
1: minimum the delay_millis parameter time in milliseconds have
     passed
0: minimum delay_millis has not passed
*/
{
    if ((millis() - comparison_millis) > delay_millis)
    {
        return 1;
    }
    return 0;
}

const byte getDigitalPotentiometerReading(byte pin)
{
/*
 * convert reading from potentiometer (0, 1023) to 
 * digital representation of a byte (1,255)
*/
    return map(analogRead(pin), 0, 1023, 1, 255);
}

void fadeLedStateful(LED *led)
{
    if (led->analog_value == 0)
    {
        led->fade_direction = FADE_DIRECTION::UP;
    }
    else if (led->analog_value == 255)
    {
        led->fade_direction = FADE_DIRECTION::DOWN;
    }
    
    switch (led->fade_direction)
    {
        case FADE_DIRECTION::UP:
            led->analog_value += 1;
            break;
        case FADE_DIRECTION::DOWN:
            led->analog_value -=1;
            break;
    }
}
