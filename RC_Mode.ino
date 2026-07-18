// RC mode: manual differential (skid-steer) drive from two RC receiver channels.
//   x  = LR channel (steering, left-right)
//   y  = FB channel (throttle, front-back)
//   xa = turn factor (1.0 center -> 5.0 at the extremes) from the LR axis

unsigned long previousMillis = 0;  // Store the last time the LED was updated
const long interval = 800;         // Interval between blinks
const int blinkDuration = 1;       // Blink duration (ms)
bool ledState = LOW;               // State of the LED

float vFB = 0;  // Raw throttle pulse (us)
float vLR = 0;  // Raw steering pulse (us)

const float frbias = 1.00;
const float flbias = 1.00;
const float brbias = 1.00;
const float blbias = 1.00;

const float rotate = 0.7;

const float xthres = 10;
const float ythres = 10;

float x = 0;   // steering (LR)
float y = 0;   // throttle (FB)
float xa = 0;  // turn factor

float mapValue(int inputValue) {
  if (inputValue < 1500) {
    // Map the range 1000-1500 to 5-1
    return 5.0 - ((inputValue - 1000) * 4.0 / 500.0);
  } else if (inputValue > 1500) {
    // Map the range 1500-2000 to 1-5
    return 1.0 + ((inputValue - 1500) * 4.0 / 500.0);
  } else {
    // If the inputValue is exactly 1500
    return 1.0;
  }
}

void scheck() {
  Serial.print(vFB);
  Serial.print("  ");
  Serial.print(vLR);
  Serial.print("  ");
  Serial.print(x);
  Serial.print("  ");
  Serial.print(y);
  Serial.print("  ");
  Serial.print(xa);
  Serial.println("  ");
  delay(1);
}

// One iteration of the RC controller (was loop() in the RC sketch).
void rcmode() {

  vFB = pulseIn(FB, HIGH);  //read the signal value from RC receiver (throttle)
  vLR = pulseIn(LR, HIGH);  //read the signal value from RC receiver (steering)

  if ((vFB >= 850 && vFB <= 2150) && (vLR >= 850 && vLR <= 2150)) {

    unsigned long currentMillis = millis();  // Get the current time

    // Blue heartbeat blink while the RC signal is valid
    if (ledState == LOW && currentMillis - previousMillis >= interval) {
      ledState = HIGH;
      statusRcOn();  // blue on
      previousMillis = currentMillis;
    }
    if (ledState == HIGH && currentMillis - previousMillis >= blinkDuration) {
      ledState = LOW;
      statusRcOff();  // blink off
    }

    x = (vLR - 1000) / (2000 - 1000) * (255 - (-255)) + (-255);    // steering
    y = (vFB - 1000) / (2000 - 1000) * (255 - (-255)) + (-255.5);  // throttle
    xa = mapValue(vLR);

    if (y > ythres)  //FB is up
    {
      if (x > xthres)  //LR is Right, Forward Right
      {
        motorLeft.speed(y);
        motorRight.speed(y / xa);
      } else if (x < -xthres)  //LR is Left, Forward Left
      {
        motorLeft.speed(y / xa);
        motorRight.speed(y);
      } else  //No interruption, Forward
      {
        motorLeft.speed(y * flbias);
        motorRight.speed(y * frbias);
      }
    } else if (y < -ythres)  //FB is down
    {
      if (x > xthres)  //LR is Right, Backward Right
      {
        motorLeft.speed(y);
        motorRight.speed(y / xa);
      } else if (x < -xthres)  //LR is Left, Backward Left
      {
        motorLeft.speed(y / xa);
        motorRight.speed(y);

      } else  //No interruption, Backward
      {
        motorLeft.speed(y * blbias);
        motorRight.speed(y * brbias);
      }
    } else if (x > xthres)  //Only LR is Right, rotate Right
    {
      motorLeft.speed(x * rotate);
      motorRight.speed(-x * rotate);
    } else if (x < -xthres)  //Only LR is Left, rotate Left
    {
      motorLeft.speed(x * rotate);
      motorRight.speed(-x * rotate);
    } else  //No Action, Stop
    {
      motorLeft.speed(0);
      motorRight.speed(0);
    }
  } else {
    motorLeft.speed(0);
    motorRight.speed(0);
    statusRcFailsafe();  // amber: no/invalid RC signal
  }
  scheck();
  delay(10);
}
