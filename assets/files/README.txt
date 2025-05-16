
                        HackGBGay Badge 2025

                       https://hack.gbgay.com

# README

This is the badge for the HackGBGay event of 2025. It has some small games,
allows you to enter and display flags, and also has some utilities for
controlling the header input/output pins. This README file is available both
through the main menu of the badge itself, and on the USB drive available by
plugging the badge into a computer.

Note: The games here are provided for fun and to illustrate what can be done
with tiny hardware, and winning a game does not provide any secret flags.
However, no such guarantees are provided for the rest of the badge :3



# Code Entry

Flags are codes that can be found by solving the various available challenges.
They generally* start with the text "gbgay{" and end with "}". The text inbetween
the curly braces is unique for each challenge. If you find one you can enter
it by selecting "Code Entry" in the main menu, and using the little on-screen
keyboard to enter in the text.



# Snek

This a small and simple implementation of the classic game of Snake. To start,
press any direction on the stick. The play area wraps around, i.e. the walls
are not solid and allow you to go around. The score goes up as you collect the
fruits, as does the snake length and speed. The game ends when the snake runs
into itself, after which you may press (A) to try again, or (B) to exit.



# Blocks

This is a legally distinct implementation of the classic game of Tetris. An
attempt has been made to follow the Tetris Guidelines and SRS standard, but it
isn't perfect and some features (such as sound effects) are missing by necessity.



# Othello

The game of Othello, also known as Reversi. This implementation is very simple:
press up / down / left / right to move the cursor, (A) to play, and (B) to
exit.



# Gallery

A small gallery of animations (meme GIFs) included for fun.



# Bootloader

There are multiple ways to put the badge into "BOOTSEL" mode. This mode is
built into microcontroller and allows it to be "drag-and-drop programmed" by
copying a valid firmware file (*.uf2 file) onto the USB drive named "RPI-RP2"
that shows up in BOOTSEL mode. BOOTSEL mode can be entered by:

  1) Holding the BOOTSEL button and then pressing the RESET button to restart
     the microcontroller before releasing the BOOTSEL button.

  2) Pressing the RESET button twice within about half a second will make the
     microcontroller put itself in BOOTSEL mode.

  3) Using the "Bootloader" entry in the main menu, which will cause the badge
     to put itself in BOOTSEL mode.

When in BOOTSEL mode, the LCD should light up plain white.
To leave this mode simply press the RESET button to restart the badge.



# Hardware & Firmware

The microcontroller on the badge is a Raspberry Pi RP2040 chip, featuring two
ARM Cortex-M0+ cores, a 125 MHz system clock, 256 KiB of core RAM, and 16 MiB
of external FLASH memory.

When plugged into a computer, the badge will present as a USB drive containing
this file, and also as a USB COM port with debug text output.

The firmware sources and assets for the badge, including this file and others,
will be available on GitHub. Because the sources might include some of the
flags, the repository is private until after the event.

               https://github.com/farpoke/badge-2025

/Artemis



* There are, of course, some exceptions to the rule, where, due to the nature
  of the challenge and/or flag, the leading "gbgay{" and trailing "}" have
  been omitted. For example, there are two short codes hidden on the badge
  PCB itself that have strict space constraints.








































































































































































gbgay{ReadMe}
