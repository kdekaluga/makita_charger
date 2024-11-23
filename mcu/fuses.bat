@echo off
rem c:\programming\avrdude\avrdude.exe -c usbasp -p m328p -v -B 10MHz
rem C:\Users\SD\.platformio\packages\tool-avrdude\avrdude.exe -c usbasp -p m328p -v -B 100kHz

rem Fuses:
rem - Enable brown-out detector at 4.5 V
rem - EEPROM is preserved on chip erase
rem - Watchdog is always enabled
c:\programming\avrdude\avrdude.exe -c usbasp -p m328p -B 100kHz -U efuse:w:0xFC:m -U hfuse:w:0xC1:m -U lfuse:w:0xFF:m
rem fuses: E:FF, H:D9, L:FF
