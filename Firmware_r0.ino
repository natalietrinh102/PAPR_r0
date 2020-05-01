/*
OPEN SOURCE PAPR

Due to specfific changes to timer registers, this code is optimized to run on the ATMEGA328 platform only.

 */

#include <QC2Control.h>
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
int timer2Div = 64

// Manual Override Mode
bool manualEnabled = true; // true = disables feedback control. PAPR must be manually tuned by technician when changing between models of filters, batteries, or fans. 
int lowFanmod = 0.8 // Multiplier for low fan speed (ex. lowFanmod = 0.8 -> 80% of max PWM duty cycle)
int medFanmod = 0.9
int highFanmod = 1

// Quickcharge
bool qcEnabled = true; // true = Re-establish handshake on heart, false = disable QC Heartbreak
int qcDataPos = 2
int qcDataNeg = 5
QC2Control quickCharge(2, 5);

// Fan Control
int fanPin = 3;  // ATMEGA328 only supports full fast PWM on pin 3 (reduces motor whine)
int switchLow = 7; // Prefer non-PWM pins for switch
int switchHigh = 4;
int fanSpeed = 255; // Integer fan speed is max by default
int LowOn;
int HighOn;
int SpeedSet;

// Flow Feedback
int TargetCfm;
int CurrentCfm;

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

// Spool fan up before continuing - this helps prevent stalls
Serial.println("Spooling fan");
analogWrite(fanPin, 255);
delay(1000);
}

void loop() {



// Monitor switch state (move this to interupt later)
  Serial.println("Attempting to read switch state...");
// Read switch and determine perferred airflow.
  LowOn = digitalRead(switchLow);
  HighOn = digitalRead(switchHigh);
  if ((LowOn == 0) && (HighOn == 1)) {
    Serial.println("User requested low speed");
    SpeedSet = 1;
    TargetCfm = 4;
  }
  else if ((LowOn == 1) && (HighOn == 0)){
    Serial.println("User requested high speed");
    SpeedSet = 3;    
    TargetCfm = 9;
  }
  else if ((LowOn == 1) && (HighOn == 1)){
    Serial.println("User requested medium speed OR switch is disconnected");
    SpeedSet = 2;
    TargetCfm = 7;
  }
  else {
    Serial.println("ERROR: Invalid switch reading! Check for noise!");
  }


// Make adjustments based on sensors (currently not used)
CurrentCfm = TargetCfm; // Bypass the CFM calculations

// Alternative manual mode only
manualMode(SpeedSet);


// Push fan speed to fan
analogWrite(fanPin, fanSpeed);
}

void manualMode(SpeedSet){
  switch (SpeedSet) {
  case 1:
    Serial.println("Manually setting speed to 80%");
    fanSpeed = .80 * 255;
    break;
  case 2:
    Serial.println("Manually setting speed to 90%");
    fanSpeed = .90 * 255;
    break;
  case 3:
    Serial.println("Manually setting speed to 100%");
    fanSpeed = 1 * 255;
    break;
  default:
    Serial.println("ERROR: SpeedSet is invalid!");
    break;
}
}
