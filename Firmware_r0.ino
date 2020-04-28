int fanPin = 3;  // ATMEGA328 only supports full fast PWM on pin 3 (reduces motor whine)
int switchLow = 7; //Prefer non-PWM pins for switch
int switchHigh = 4;

int fanSpeed = 255; // Integer fan speed is max by default
int LowOn;
int HighOn;
int SpeedSet;
int TargetCfm;

int CurrentCfm;

void setup() {
Serial.begin(9600);

/*
This values provide PMW frequency tuning for pin 3. Play with this if the fan/motor whines excessively.
Different fans/motors will prefer different frequencies, and some will whine regardless.
Note changing the frequency will mess with the manual mode calibration! 
*/
//TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
//TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
//TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
//TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
//TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
//TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
//TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

pinMode(fanPin,OUTPUT);
pinMode(switchLow,INPUT_PULLUP);
pinMode(switchHigh,INPUT_PULLUP);

// Allow the fan to spool up before continuing - this helps prevent stalls
Serial.println("Spooling fan");
analogWrite(fanPin, 255);
delay(1000);
}

void loop() {

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


// Push fan speed to fan
analogWrite(fanPin, fanSpeed);
}
