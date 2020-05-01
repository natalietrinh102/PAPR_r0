# Open Source PAPR / Negetive Ventilation Environment
Open Source PAPR firmware for the ATMEGA328 platform

The COVID-19 pandemic has caused a shortage of PAPRs - a prized form of PPE that is typically too expensive to deploy at scale. However, PAPRS offer distinct advantages to conventional respisators - they are more compatible with different face shapes, do not need to be fit tested, are comfortable for longer periods of times, and form are a integrated & comprehensive form of PPE. Additonally, the system can be reversed to function as a negetive ventilaton environment to prevent the spread of COVID-19 from a patient.

This project aims to create a flexible, adaptable package that can be easily modified and tuned by a hospital technican or engineer to use different combinations of fans and filters. 

## Challenges
- Supply chain delays and shortages have caused a lack of geniune filter material, along with lead times with parts forces us to use locally available and locally manufactured parts. As such, this package is designed to be easily tuned for different combinations of fans and filters. In addition to this, a differentional pressure sensor (used for airflow monitoring) allows the circuit to compensate for differences in configurations and filter quality. 
- Charging / power supply infrastructure: PAPRs are relatively high current, power hungry devices. As such, high density battery technologies such as Li-ion or LiPo batteries are preferred, but that in itself presents it's own challenges. As such, this packaged to designed to handle two configurations of power input - direct 12-14.4V current froma battery pack, or through Quickcharge 2.0+ compatible powerbanks, which are widely available and already owned by many people.


## Project Features
The project is being designed in phases, with more advanced features implemented in future releases.
### Stage 0 (Proof of concept)
- Manual mode (tuned by hand for each configuration)
- Only direct 12V supported
- Tested on Arduino Uno

### Stage 1 (Power Control) - CURRENT
- Code modularization 
- Quickcharge 2.0+ support
- Support for autoswitching of power supplies 
- PWM frequency tuning (Reduces blower noise)


### Stage 2 (Automation)
- Pin Interrupt mode (Allows PID loop to be interrupted on user input)
- Differentional pressure sensor reading 
- PID w/ autotuning

### Stage 3 (Miniaturization)
- Migrate from UNO to custom PCB based on ATmega328p
- Add a debug port featuring Serial Tx, Serial Rx, and RESET, and along with AVR Programmer pins to allow for easy firmware updates and tuning. 
