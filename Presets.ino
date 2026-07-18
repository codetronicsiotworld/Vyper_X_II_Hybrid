// Opening move for the LEFT attack IR: 3 short creep-forward pulses,
// each followed by a 2 s guarded wait (sensors stay active throughout).
void preset1() {
  for (int i = 0; i <= 2; i++) {
    readSensors();
    delay(10);
    motorLeft.speed(50 * flbias);
    motorRight.speed(50 * frbias);
    delay(80);
    readSensors();
    delay(10);
    motorLeft.speed(100 * flbias);
    motorRight.speed(100 * frbias);
    delay(120);
    readSensors();
    delay(10);

    stop();
    aclval = 0;
    front = 0;

    unsigned long startTime = millis();

    while (millis() - startTime < 2000) {
      readSensors();
      timedebounce();
      actOnSensors();
      if (executePG == false) {
        break;
      }
    }
    if (executePG == false) {
      break;
    }
  }
  aclval = 65;
}

// Opening move for the FRONT attack IR: same creep-and-wait pattern as
// preset1 but repeated 10 times, with stall detection active.
void preset2() {
  for (int i = 0; i <= 9; i++) {
    readSensors();
    timedebounce();
    delay(10);
    motorLeft.speed(50 * flbias);
    motorRight.speed(50 * frbias);
    delay(80);
    readSensors();
    timedebounce();
    delay(10);
    motorLeft.speed(100 * flbias);
    motorRight.speed(100 * frbias);
    delay(120);
    readSensors();
    timedebounce();
    delay(10);

    stop();
    aclval = 0;
    front = 0;

    unsigned long startTime = millis();

    while (millis() - startTime < 2000) {
      readSensors();
      timedebounce();
      actOnSensors();
      checkTimerStall();
      if (executePG == false) {
        break;
      }
    }
    if (executePG == false) {
      break;
    }
  }
  aclval = 65;
}

// Scripted dodge-and-flank move (currently unused).
void preset3() {
  motorLeft.speed(-200);
  motorRight.speed(200);
  delay(30);
  motorLeft.speed(50);
  motorRight.speed(50);
  delay(50);
  motorLeft.speed(180);
  motorRight.speed(180);
  delay(100);
  motorLeft.speed(250);
  motorRight.speed(250);
  delay(110);
  stop();
  delay(50);
  motorLeft.speed(200);
  motorRight.speed(0);
  delay(220);
  aclval = 60;
}

// Opening move for the RIGHT attack IR: lower top speed, strong front boost.
void preset4() {
  aclval = 40;
  front = 100;
}
