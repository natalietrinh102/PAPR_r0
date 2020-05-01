/*
OPEN SOURCE PAPR

Due to specfific changes to timer registers, this code is optimized to run on the ATMEGA328 platform only.

 */

#include <QC2Control.h>
#include <AutoPID.h>

// ------------- SETTINGS -------------- //

/*
PWM Tuning
This values provide PMW frequency tuning for pin 3. Play with this if the fan/motor whines excessively.
Different fans/motors will prefer different frequencies, and some will whine regardless. 
BIGGER IS NOT BETTER. Higher frequencies change motor whine frequency at the cost of torque and resolution.

Note changing the frequency will mess with the manual mode calibration! 

timer2Div |    PWM
   value  | frequency
-----------------------
1         | 31372.55 Hz
8         | 3921.16 Hz
32        | 980.39 Hz
64        | 490.20 Hz (The DEFAULT)
128       | 245.10 Hz
256       | 122.55 Hz
1024      | 30.64 Hz
*/
int timer2Div = 64;



// Quickcharge
bool qcEnabled = true; // true = Re-establish handshake on heart, false = disable QC Heartbreak
const int qcDataPos = 2;
const int qcDataNeg = 5;
QC2Control quickCharge(2, 5);

// Fan Control
const int fanPin = 3;  // ATMEGA328 only supports full fast PWM on pin 3 (reduces motor whine)
const int switchLow = 7; // Prefer non-PWM pins for switch
const int switchHigh = 4;
int fanSpeed = 255; // Integer fan speed is max by default
int LowOn;
int HighOn;
int SpeedSet;

// Manual Override Mode
bool manualEnabled = true; // true = disables feedback control. PAPR must be manually tuned by technician when changing between models of filters, batteries, or fans. 
int lowFanmod = 80; // Power of fan, as a percentage of max power (ex. lowFanmod = 80 -> 80% of max PWM duty cycle)
int medFanmod = 90;
int highFanmod = 100;

// PID (Feedback) Mode
// Note: Only pressure sensors with a voltage-varying output are supported. Sensors with low ranges may require an amplifier. 
const int presPin = A0; // Define which pin the pressure sensor output is connected to
const int presMin = 20; // The minimum output voltage of the pressure sensor, expressed as V * 100 (presMin = 20 -> 0.2)
const int presMax = 470; // The maximum output voltage of the pressure sensor, expressed as V * 100
int lowFanauto = 650; // Target airflow in, expressed as CFM * 100 (lowFanauto = 650 -> 6.50 CFM)
int medFanauto = 750;
int highFanauto = 950;
int CurrentCfm;

// ------------- SETUP -------------- //

void setup() {
// Start Debug interface
Serial.begin(9600);

// Adjust PWM as needed
switch (timer2Div) {
  case 1:
    Serial.println("PWM Frequency set to 31372.55 Hz");
    TCCR2B = TCCR2B & B11111000 | B00000001;
    break;
  case 8:
    Serial.println("PWM Frequency set to 3921.16 Hz");
    TCCR2B = TCCR2B & B11111000 | B00000010;
    break;
  case 32:
    Serial.println("PWM Frequency set to 980.39 Hz");
    TCCR2B = TCCR2B & B11111000 | B00000011;
    break;
  case 64:
    Serial.println("PWM Frequency set to 420.20 Hz");
    TCCR2B = TCCR2B & B11111000 | B00000100;
    break;
  case 128:
    Serial.println("PWM Frequency set to 245.10 Hz");
    TCCR2B = TCCR2B & B11111000 | B00000101;
    break;
  case 256:
    Serial.println("PWM Frequency set to 122.55 Hz");
    TCCR2B = TCCR2B & B11111000 | B00000110;
    break;
  case 1024:
    Serial.println("PWM Frequency set to 30.64 Hz");
    TCCR2B = TCCR2B & B11111000 | B00000111;
    break;
  default:
    Serial.println("ERROR: Invalid PWM Frequency requested. Frequency not changed.");
    break;
}

// Setup fan control
pinMode(fanPin,OUTPUT);
pinMode(switchLow,INPUT_PULLUP);
pinMode(switchHigh,INPUT_PULLUP);

// Initial Quickcharge handshake
// Note - QC needs a very short delay before accepting handshakes - run setVoltage() late in setup
switch (qcEnabled) {
  case true:
    Serial.println("Sending Initial Quickcharge handshake");
    quickCharge.setVoltage(12);
    break;
  case false:
    Serial.println("Quickcharge disabled in Settings, bypassing initial handshake");
    break;
}

/*
 EXPERIMENTAL CODE
// Switch interrupt, urgently reads changes in speed.
const byte interruptPin = 2; //On ATMEGA328, only pins 2 & 3 can have interrupts. 
attachInterrupt(digitalPinToInterrupt(interruptPin), speedRead, CHANGE);
*/
 
// Spool fan up before continuing - this helps prevent stalls
Serial.println("Spooling fan");
analogWrite(fanPin, 255);
delay(1000);
}

// ------------- LOOP -------------- //

void loop() {
// Read the switch setting
readSpeed();

// Changes control method
switch (manualEnabled) {
  case true:
    manualMode();
    break;
  case false:
    pidMode();
    break;
}

// Push fan speed to fan
analogWrite(fanPin, fanSpeed);
}

// ------------- FUNCTIONS -------------- //

void manualMode(){
  switch (SpeedSet) {
  case 1:
    Serial.print("Low: Manually setting speed to ");
    Serial.print(lowFanmod);
    Serial.println("%");
    fanSpeed = lowFanmod * 255 / 100;
    break;
  case 2:
    Serial.print("Medium: Manually setting speed to ");
    Serial.print(medFanmod);
    Serial.println("%");
    fanSpeed = medFanmod * 255 / 100;
    break;
  case 3:
    Serial.print("High: Manually setting speed to ");
    Serial.print(highFanmod);
    Serial.println("%");
    fanSpeed = highFanmod * 255 / 100;
    break;
  default:
    Serial.println("ERROR: SpeedSet is invalid!");
    break;
}
}

void readSpeed(){
  Serial.println("Attempting to read switch state...");
// Read switch and determine perferred airflow.
  LowOn = digitalRead(switchLow);
  HighOn = digitalRead(switchHigh);
  if ((LowOn == 0) && (HighOn == 1)) {
    Serial.println("User requested low speed");
    SpeedSet = 1;
  }
  else if ((LowOn == 1) && (HighOn == 0)){
    Serial.println("User requested high speed");
    SpeedSet = 3;    
  }
  else if ((LowOn == 1) && (HighOn == 1)){
    Serial.println("User requested medium speed OR switch is disconnected");
    SpeedSet = 2;
  }
  else {
    Serial.println("ERROR: Invalid switch reading! Check for noise!");
  }
}

void pidMode(){

// Find the target CFM
  float TargetCfm; // Remember this is multipled by 100! (Saves memory :D)
    switch (SpeedSet) {
  case 1:
    Serial.print("Low: Automatically setting CFM to ");
    Serial.println(lowFanauto / 100);
    TargetCfm = lowFanauto;
    break;
  case 2:
    Serial.print("Low: Automatically setting CFM to ");
    Serial.println(medFanauto / 100);
    TargetCfm = medFanauto;
    break;
  case 3:
    Serial.print("Low: Automatically setting CFM to ");
    Serial.println(highFanauto / 100);
    TargetCfm = highFanauto;
    break;
  default:
    Serial.println("ERROR: SpeedSet is invalid!");
    break;
}


}
