void customMove1() {
  for (int fadeValue = 0; fadeValue <= 500; fadeValue += 1) {
    motorLeft.speed(160);
    motorRight.speed(160);
    delay(1);
  }
}

void customMove2() {
  for (int fadeValue = 30; fadeValue <= 200; fadeValue += 20) {
    motorLeft.speed(fadeValue);
    motorRight.speed(fadeValue * 0.55);
    delay(2);
  }
}

void customMove3() {
  motorLeft.speed(250);
  motorRight.speed(180);
}

// After 'flag' has been high for 500 ms, set front = 40 (head-on push adjust).
// Resets whenever flag goes low.
void checkTimer() {
  if (flag) {
    if (!timerActive) {
      timerStart = millis();
      timerActive = true;
    }

    if (millis() - timerStart >= 500) {
      front = 40;
      timerActive = false;
    }
  } else {
    timerActive = false;
    timerStart = 0;
    front = 0;
  }
}

// After 'stallflag' has been high for 2 s (stalemate push), set
// stallfront = 150 for extra power. Resets whenever stallflag goes low.
void checkTimerStall() {
  if (stallflag) {
    if (!stalltimerActive) {
      stalltimerStart = millis();
      stalltimerActive = true;
    }

    if (millis() - stalltimerStart >= 2000) {
      stallfront = 150;
      stalltimerActive = false;
    }
  } else {
    stalltimerActive = false;
    stalltimerStart = 0;
    stallfront = 0;
  }
}

// Line-sensor debounce: debounceOk goes true once 'debounce' has been
// held high for 25 ms.
void timedebounce() {
  if (debounce) {
    if (!debounceActive) {
      debounceStart = millis();
      debounceActive = true;
    }

    if (millis() - debounceStart >= 25) {
      debounceOk = true;
      debounceActive = false;
    }
  } else {
    debounceActive = false;
    debounceStart = 0;
    debounceOk = false;
  }
}

// ---- Basic moves ----

void stop() {
  motorLeft.speed(0);
  motorRight.speed(0);
}

void Backward() {
  motorLeft.speed(-160);
  motorRight.speed(-160);
}

void Turnleft() {
  motorLeft.speed(-200);
  motorRight.speed(200);
}

void Turnright() {
  motorLeft.speed(200);
  motorRight.speed(-200);
}

void Forward() {
  motorLeft.speed(160);
  motorRight.speed(160);
}

void search() {
  motorLeft.speed(-80);
  motorRight.speed(-80);
}
