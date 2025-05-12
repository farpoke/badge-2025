#pragma once

// The size in bytes of the external FLASH on the badge.
#define BADGE_FLASH_SIZE (4 * 1024 * 1024)

// Use UART 1 on pins 20 and 21 for stdout and stdin.
#define PICO_DEFAULT_UART        1
#define PICO_DEFAULT_UART_TX_PIN 20
#define PICO_DEFAULT_UART_RX_PIN 21

// Use SPI 1 on pins 10-12 to talk to the LCD.
#define LCD_SPI_PORT    spi1
#define LCD_SPI_CLK_PIN 10
#define LCD_SPI_OUT_PIN 11
#define LCD_SPI_IN_PIN  12

// The LCD SPI passes through an analog switch to control the direction.
//      DIR 0 => in     DIR 1 => out
#define LCD_SPI_DIR_PIN 13

#define LCD_CS_PIN   14 // LCD chip select, active low.
#define LCD_DCX_PIN  8  // LCD data/command signal.
#define LCD_NRST_PIN 9  // LCD reset signal, active low.

// LCD backlight controlled via a PMOS transistor and PWM.
#define LCD_LED_PIN 7

// Power indicator LEDs can be disabled by the MCU via this pin.
#define PWR_LED_PIN 15

// Input pin to let the MCU sense the presence of a USB VBUS voltage.
#define VBUS_SENSE_PIN 25

// Left button/stick pins:
#define BTN_UP    23
#define BTN_DOWN  22
#define BTN_LEFT  19
#define BTN_RIGHT 24
#define BTN_PUSH  18

// Right button/stick pins:
#define BTN_A 6
#define BTN_B 5
#define BTN_C 16
#define BTN_D 17
