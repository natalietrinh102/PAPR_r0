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

Note changing the frequency will mess with the manual and PID calibration! 

timer2Div |    PWM
   value  | frequency
-----------------------
0         | Disable frequency tuning (Compatiable with non-ATMEGA328 chips)
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
bool qcEnabled = true; // true = Re-establish handshake on heart, false = disable QC2.0 Heartbeat
const int qcDataPos = 8;
const int qcDataNeg = 5;
QC2Control quickCharge(qcDataPos, qcDataNeg);

// Fan Control
const int fanPin = 3;  // ATMEGA328 only supports full fast PWM on pin 3 (reduces motor whine)
const int switchLow = 7;
const int switchHigh = 4;
const int spoolTime = 1000; // Delays control inputs for in ms, prevents stalls
int fanSpeed = 255; // Sets initial fan speed. It is highly preferred to change spoolTime, but this may be useful for preventing surges in current draw during spooling
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
const int lowFanauto = 650; // Target airflow in, expressed as CFM * 100 (lowFanauto = 650 -> 6.50 CFM)
const int medFanauto = 750;
const int highFanauto = 950;
#define OUTPUT_MIN 100 // Minimum output. Adjust this to prevent stalls
#define OUTPUT_MAX 255 // Maximum output. Best to leave this at the max of 255, unless your fan is massively overpowered.
#define KP .12 // Initial PID proportional gain
#define KI .0003 // Initial PID integral gain
#define KD 0 // Initial PID derivative gain
double currentCfm, setPoint, outputVal;
const int initialStep = 500; // The inital PID time interval in ms. Use an aggressive low setting in the beginning.
int slowStep = 5000; // Slower PID interval in ms. Reduces resource use.

AutoPID fanPID(&currentCfm, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);


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

// Setup PID
fanPID.setTimeStep(initialStep);

// Initial Quickcharge handshake
// Note - QC needs a short delay before accepting handshakes - run setVoltage() late in setup
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
// Switch interrupt, urgently reads changes in speed setting.
const byte interruptPin = 2; //On ATMEGA328, only pins 2 & 3 can have interrupts. 
attachInterrupt(digitalPinToInterrupt(interruptPin), speedRead, CHANGE);
*/
 
// Spool up fan (Anti-stall)
Serial.println("Spooling fan");
analogWrite(fanPin, fanSpeed);
delay(spoolTime);
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
analogWrite(fanPin, fanSpeed);
}

void readSpeed(){
  int LowOn;
  int HighOn;

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
    switch (SpeedSet) {
  case 1:
    Serial.print("Low: Automatically setting CFM to ");
    Serial.println(lowFanauto / 100);
    setPoint = lowFanauto;
    break;
  case 2:
    Serial.print("Low: Automatically setting CFM to ");
    Serial.println(medFanauto / 100);
    setPoint = medFanauto;
    break;
  case 3:
    Serial.print("Low: Automatically setting CFM to ");
    Serial.println(highFanauto / 100);
    setPoint = highFanauto;
    break;
  default:
    Serial.println("ERROR: SpeedSet is invalid!");
    break;
}

// Start PID control
fanPID.run(); // Although this is called in every loop, it will only update per interval specified in setTimeStep.
analogWrite(fanPin, outputVal);
}
