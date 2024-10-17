@echo off
c:\programming\avrdude\avrdude.exe -c usbasp -p m328p -B 10MHz -U flash:w:.pio\build\ATmega328P\firmware.hex
