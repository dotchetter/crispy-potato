#pragma once

enum class State
{
    IDLE,
    LED_CYCLE,
    LED_RAINBOW,
    READ_SERIAL,
    OFF,
    UART_DISPLAY_CURRENT_STATE
};