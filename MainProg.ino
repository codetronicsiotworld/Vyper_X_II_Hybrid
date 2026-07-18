void runpreset() {

  processpreset();

  switch (currentpreset) {
    case 0:  // No preset selected
      aclval = 50;
      break;

    case 1:
      for (int i = 0; i < 1; i++) {
        preset1();
      }
      break;

    case 2:
      for (int i = 0; i < 1; i++) {
        preset2();
      }
      break;

    case 3:
      for (int i = 0; i < 1; i++) {
        preset4();
      }
      break;
  }
}

// Latch the preset chosen by the attack-select IR inputs (L / F / R).
void processpreset() {
  if (LV) {
    currentpreset = 1;
  } else if (FV) {
    currentpreset = 2;
  } else if (RV) {
    currentpreset = 3;
  }
}

// Ramp Accel from startValue up to maxValue, one step every incrementSpeed ms.
void acceleration(int startValue, int maxValue, unsigned long incrementSpeed) {
  if (resetAccel) {
    Accel = startValue;
    resetAccel = false;
  }

  if (startAccel) {
    if (millis() - lastIncrementTime >= incrementSpeed) {
      lastIncrementTime = millis();

      if (Accel < maxValue) {
        Accel++;
        readSensors();
        actOnSensors();
      } else {
        Accel = maxValue;
        readSensors();
        actOnSensors();
      }
    }
  }
}

void resetflags() {
  resetAccel = true;
  startAccel = false;
  reset = false;
}

// Main autonomous match loop: runs until the IR stop signal.
void mainprgm() {

  statusAutoRunning();  // solid red: match running
  startAccel = true;
  resetAccel = true;
  while (executePG == true) {
    acceleration(0, aclval, 2);
    checkTimer();
    checkTimerStall();

    readSensors();
    timedebounce();
    //testbias();
    actOnSensors();
    delay(3);
  }
}

// Same as mainprgm() but runs forever (button-started testing).
void buttonmainprgm() {

  statusAutoRunning();
  startAccel = true;
  resetAccel = true;
  while (true) {
    acceleration(0, aclval, 3);
    checkTimer();

    readSensors();
    timedebounce();
    actOnSensors();
    delay(3);
  }
}

// Read line + proximity sensors into senState.
// A white line triggers an immediate (debounced) edge-avoid manoeuvre.
// Otherwise senState is a bitmask of the proximity sensors:
//   L = 8, FL = 16, F = 32, FR = 64, R = 128
void readSensors() {
  senState = 0;

  if (Lvalue < LINE_THRESHOLD) {  // white line on the left
    senState = 1;
    debounce = true;
    if (debounceOk == true) {
      resetflags();
      stop();
      delay(20);
      Backward();
      delay(150);
      Turnright();
      delay(115);
    }
  } else if (Rvalue < LINE_THRESHOLD) {  // white line on the right
    senState = 2;
    debounce = true;
    if (debounceOk == true) {
      resetflags();
      stop();
      delay(20);
      Backward();
      delay(150);
      Turnleft();
      delay(115);
    }
  } else if (Rvalue < LINE_THRESHOLD && Lvalue < LINE_THRESHOLD) {  // both lines
    senState = 3;
    debounce = true;
    if (debounceOk == true) {
      resetflags();
      stop();
      delay(20);
      Backward();
      delay(150);
      Turnright();
      delay(110);
    }
  }

  if (senState == 0) {  // on the black field: read proximity sensors
    if (LSv) senState += 8;
    if (FLv) senState += 16;
    if (FSv) senState += 32;
    if (FRv) senState += 64;
    if (RSv) senState += 128;
  }
}

// Debug helper: drive straight with bias applied (enable in mainprgm).
void testbias() {
  if (senState == 0) {
    motorLeft.speed(Accel * flbias);
    motorRight.speed(Accel * frbias);
  }
}

// Act on the sensor state from readSensors().
void actOnSensors() {

  delay(5);

  switch (senState) {
    case 0:  // Nothing detected: drive forward with acceleration
      if (!reset) {
        resetAccel = true;
        startAccel = true;
        reset = true;
      }
      flag = false;
      stallflag = false;
      motorLeft.speed(Accel * flbias);
      motorRight.speed(Accel * frbias);
      break;

    case 8:  // Left proximity sensor
      resetAccel = true;
      Turnleft();
      delay(turnDelay);
      startAccel = true;
      break;

    case 24:  // Error L + FL
    case 16:  // Front-left proximity sensor
      motorLeft.speed(30);
      motorRight.speed(175);
      break;

    case 32:  // Front proximity sensor

      flag = true;
      motorLeft.speed(Accel * flbias);
      motorRight.speed(Accel * frbias);
      break;

    case 56:  // Error L + FL + F
    case 48:  // Front-left & front sensors
      motorLeft.speed(30);
      motorRight.speed(125);
      break;

    case 192:  // Error R + FR
    case 64:   // Front-right proximity sensor
      motorLeft.speed(175);
      motorRight.speed(30);
      break;

    case 224:  // Error R + FR + F
    case 96:   // Front-right & front sensors
      motorLeft.speed(125);
      motorRight.speed(30);
      break;

    case 128:  // Right proximity sensor
      resetAccel = true;
      Turnright();
      delay(turnDelay);
      startAccel = true;
      break;

    case 120:  // Error L + FL + F + FR
    case 136:  // Error L + R
    case 240:  // Error R + FR + F + FL
    case 248:  // Error L + FL + F + FR + R
    case 112:  // Front-left, front & front-right sensors: full push
      flag = true;
      stallflag = true;
      motorLeft.speed((Accel - front * 0.8 + stallfront) * flbias);
      motorRight.speed((Accel - front * 0.8 + stallfront) * frbias);
      break;
  }
}
