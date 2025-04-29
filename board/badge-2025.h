#pragma once

// Use UART 1 on pins 20 and 21 for stdout and stdin.
#define PICO_DEFAULT_UART 1
#define PICO_DEFAULT_UART_TX_PIN 20
#define PICO_DEFAULT_UART_RX_PIN 21

// Use SPI 1 on pins 10-12 to talk to the LCD.
#define LCD_SPI_PORT spi1
#define LCD_SPI_CLK_PIN 10
#define LCD_SPI_OUT_PIN 11
#define LCD_SPI_IN_PIN  12

// The LCD SPI passes through an analog switch to control the direction.
//      DIR 0 => in     DIR 1 => out
#define LCD_SPI_DIR_PIN 13

#define LCD_CS_PIN   14 // LCD chip select, active low.
#define LCD_DCX_PIN   8 // LCD data/command signal.
#define LCD_NRST_PIN  9 // LCD reset signal, active low.

// LCD backlight controlled via a PMOS transistor and PWM.
#define LCD_LED_PIN 7 

// Left button/stick pins:
#define BTN_LEFT_COMMON 17
#define BTN_LEFT_UP     22
#define BTN_LEFT_DOWN   16
#define BTN_LEFT_LEFT   18
#define BTN_LEFT_RIGHT  19
#define BTN_LEFT_PUSH   26

// Right button/stick pins:
#define BTN_RIGHT_COMMON 5
#define BTN_RIGHT_UP     1
#define BTN_RIGHT_DOWN   4
#define BTN_RIGHT_LEFT   3
#define BTN_RIGHT_RIGHT  0
#define BTN_RIGHT_PUSH   2
