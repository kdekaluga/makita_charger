rem c:\programming\avrdude\avrdude.exe -c usbasp -p m328p -v -B 10MHz
rem C:\Users\SD\.platformio\packages\tool-avrdude\avrdude.exe -c usbasp -p m328p -v -B 100kHz
c:\programming\avrdude\avrdude.exe -c usbasp -p m328p -B 100kHz -U efuse:w:0xFF:m -U hfuse:w:0xD9:m -U lfuse:w:0xFF:m
rem fuses: E:FF, H:D9, L:FF
