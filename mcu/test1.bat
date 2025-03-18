@echo off
cls
rem c:\programming\avrdude\avrdude.exe -c usbasp -p m328p -v -B 10MHz
c:\programming\avrdude\avrdude.exe -c usbasp -p m328p -v -B 10kHz
rem fuses: E:FF, H:D9, L:FF
