# Open Source PAPR / Negetive Ventilation Environment
Open Source PAPR firmware for the ATMEGA328 platform

The COVID-19 pandemic has caused a shortage of PAPRs - a prized form of PPE that is typically too expensive to deploy at scale. However, PAPRS offer distinct advantages to conventional respisators - they are more compatible with different face shapes, do not need to be fit tested, are comfortable for longer periods of times, and form are a integrated & comprehensive form of PPE. Additonally, the system can be reversed to function as a negetive ventilaton environment to prevent the spread of COVID-19 from a patient.

This project aims to create a flexible, adaptable package that can be easily modified and tuned by a hospital technican or engineer to use different combinations of fans and filters. 

## Challenges
- Supply chain delays and shortages have caused a lack of geniune filter material, along with lead times with parts forces us to use locally available and locally manufactured parts. As such, this package is designed to be easily tuned for different combinations of fans and filters. In addition to this, a differentional pressure sensor (used for airflow monitoring) allows the circuit to compensate for differences in configurations and filter quality. 
- Charging / power supply infrastructure: PAPRs are relatively high current, power hungry devices. As such, high density battery technologies such as Li-ion or LiPo batteries are preferred, but that in itself presents it's own challenges. As such, this packaged to designed to handle two configurations of power input - direct 12-14.4V current from an external power source, or through Quickcharge 2.0+ compatible powerbanks, which are widely available and already owned by many people.


## Project Features
The code currently runs off a Ardunio Uno - It will be made into it's own ATMEGA328P based circuit later :)

### STABLE
- PWM frequency tuning (Reduces blower noise)
- Fan Speed selection

### NIGHTLY
- Differentional pressure sensor reading 
- Power sources: Quickcharge 2.0+ external power sources up to 14.4V
- PID w/ autotuning
- FTDI

### COMING SOON...
- LED Indicators
- Mechanical design


## Schematic
![Schematic](https://raw.githubusercontent.com/natalietrinh102/PAPR_r0/master/Schematic.png)

## License
PAPR Firmware
Copyright (C) 2020  Natalie Trinh

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
