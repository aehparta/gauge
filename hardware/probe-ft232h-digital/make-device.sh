#!/bin/bash

cp ftdi_eeprom.config.template ftdi_eeprom.config.new
serial=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 6 | head -n 1)
serial="TL$serial"
echo "serial=\"$serial\"" >> ftdi_eeprom.config.new
ftdi_eeprom --flash-eeprom ftdi_eeprom.config.new
unlink ftdi_eeprom.config.new
echo "New device with serial $serial"
