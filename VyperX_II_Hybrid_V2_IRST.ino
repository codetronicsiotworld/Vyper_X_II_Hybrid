//________________________________________________________________________________________________________________//
//                                                                                                                //
//                        Vyper X II Hybrid V2 (IRST) - Auto / Sumo + RC firmware                                //
//                                        Author: Ian Goh & Marcus Wong                                           //
//                                                                                                                //
//   Mode is selected by a physical switch on GP19 and latched ONCE at boot:                                      //
//       GP19 LOW  -> Auto / AT mode (autonomous sumo)                                                            //
//       GP19 HIGH -> RC mode (manual differential drive)                                                         //
//   The pin uses an internal pulldown, so a disconnected/floating switch defaults to Auto.                       //
//   Flip the switch, then reset/power-cycle to change modes.                                                     //
//                                                                                                                //
//   Required libraries:                                                                                          //
//     - <MotorControl.h>                                                                                         //
//     - RP2350 core (https://github.com/earlephilhower/arduino-pico)                                            //
//                                                                                                                //
//   Recommended upload settings (Generic RP2350):                                                                //
//       Flash Size: 2MB (Sketch: 1984KB, FS: 64KB)   |   CPU Speed: 150 MHz   |   Optimize: -O3                  //
//   The FS partition holds the persistent motor-bias calibration (/bias.cfg) - see Bias_Config.ino.              //
//   It survives reflashing; only a full-chip erase or changing the Flash Size layout clears it.                  //
//________________________________________________________________________________________________________________//


#include <MotorControl.h>
#include <Adafruit_NeoPixel.h>  // install via Library Manager: "Adafruit NeoPixel"

MotorControl motorRight(2, 3, 1, 'M');  // MotorControl (name) Motor Pin (Dir, Pwm, Direction, Mode(Motordriver / H-bridge))
MotorControl motorLeft(4, 5, 1, 'M');

// ---- Status WS2812 (single pixel on GP25) ----
#define PIXEL_PIN 25
#define NUM_PIXELS 1
Adafruit_NeoPixel pixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Low-level: set the pixel to an RGB colour and push it out.
void statusColor(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

// -------- Status colour scheme (tweak the RGB values here) --------
//   Red   = Auto/AT (autonomous)    Blue = RC (manual)    Amber = warning
void statusAutoArmed()   { statusColor(30, 0, 0); }    // dim red - Auto armed, waiting for IR start
void statusAutoRunning() { statusColor(255, 0, 0); }   // red     - Auto match running
void statusRcOn()        { statusColor(0, 0, 255); }   // blue    - RC heartbeat, signal valid
void statusRcOff()       { statusColor(0, 0, 0); }     // off     - RC heartbeat blink-off
void statusRcFailsafe()  { statusColor(255, 40, 0); }  // amber   - RC signal lost / failsafe

// Boot confirmation: blink the latched mode's colour twice (red = Auto, blue = RC).
void bootFlash(bool rc) {
  for (int i = 0; i < 2; i++) {
    if (rc) statusRcOn();          // blue
    else statusAutoRunning();      // red
    delay(180);
    statusColor(0, 0, 0);          // off
    delay(160);
  }
}

// ---- Mode switch ----
#define MODE 19  // LOW = Auto/AT, HIGH = RC
bool rcMode = false;

// ---- Auto (sumo) pins ----
#define FS 10
#define LS 12
#define RS 6
#define FL 11
#define FR 7

#define Leftfollower 28
#define Rightfollower 26

#define ATKF 16
#define ATKL 17
#define ATKR 15

#define START 18

// ---- RC pins ----
#define LR 0  // steering channel (left-right)
#define FB 1  // throttle channel (front-back)

// ---- Auto sensor read macros ----
#define FSv !digitalRead(FS)
#define FLv !digitalRead(FL)
#define FRv !digitalRead(FR)
#define LSv !digitalRead(LS)
#define RSv !digitalRead(RS)

#define Lvalue analogRead(Leftfollower)
#define Rvalue analogRead(Rightfollower)

#define FV !digitalRead(ATKF)
#define LV !digitalRead(ATKL)
#define RV !digitalRead(ATKR)

#define executePG digitalRead(START)

// Motor biases (LBias/RBias for Auto, fl/fr/bl/brbias for RC) are defined in
// Bias_Config.ino: loaded from LittleFS /bias.cfg at boot, tunable over serial.

// Constants for delays
unsigned long lastTurnTime = 0;
const unsigned long turnDelay = 100;
const int CLK_DELAY = 0;
const int LINE_THRESHOLD = 500;

// Accelerations settings
bool resetAccel = false;
bool startAccel = false;
bool reset = false;
unsigned long lastIncrementTime = 0;
int Accel = 0;
int aclval = 0;

// Timer settings
unsigned long timerStart = 0;
bool timerActive = false;  // Indicates whether the timer is active
bool flag = false;         // Flag to initiate the function
int front = 0;             // Variable to be set to 30 after 3 seconds

// Stall Timer settings
unsigned long stalltimerStart = 0;
bool stalltimerActive = false;  // Indicates whether the timer is active
bool stallflag = false;         // Flag to initiate the function
int stallfront = 0;             // Variable to be set to 30 after 3 seconds

// Debounce settings
unsigned long debounceStart = 0;
bool debounceActive = false;  // Indicates whether the timer is active
bool debounce = false;        // Flag to initiate the function
bool debounceOk = false;      // Flag to be set to true after 10 miliseconds

int currentpreset = 0;

int senState = 0;

void setup() {
  pinMode(MODE, INPUT_PULLDOWN);  // floating -> LOW -> Auto (failsafe)

  // Auto / sumo sensors
  pinMode(Leftfollower, INPUT_PULLUP);   //sense line L
  pinMode(Rightfollower, INPUT_PULLUP);  //sense line R when black =1, when white =0
  pinMode(FS, INPUT);
  pinMode(FL, INPUT);
  pinMode(FR, INPUT);
  pinMode(LS, INPUT);
  pinMode(RS, INPUT);
  pinMode(START, INPUT);
  pinMode(ATKF, INPUT);
  pinMode(ATKL, INPUT);
  pinMode(ATKR, INPUT);

  // RC receiver inputs
  pinMode(LR, INPUT);
  pinMode(FB, INPUT);

  pixel.begin();
  pixel.clear();
  pixel.show();  // start with the pixel off

  Serial.begin(9600);
  analogReadResolution(12);
  analogWriteFreq(20000);

  delay(500);

  biasConfigInit();  // load persistent motor biases from LittleFS (Bias_Config.ino)

  rcMode = (digitalRead(MODE) == HIGH);  // latch mode once at boot
  bootFlash(rcMode);                     // confirm mode: 2x blue = RC, 2x red = Auto
}

void loop() {
  biasSerialTask();  // handle bias tuning commands from the Serial Monitor
  if (rcMode) {
    rcmode();    // RC mode: manual differential drive (see RC_Mode.ino)
  } else {
    autoMode();  // Auto/sumo mode (logic in MainProg.ino / Movement_def.ino)
  }
}

// One iteration of the autonomous sumo controller (was loop() in the Auto sketch).
void autoMode() {
  if (executePG == true) {
    delay(CLK_DELAY);
    runpreset();
    mainprgm();
  } else {
    stop();
    statusAutoArmed();  // dim red: armed, waiting for IR start
  }
  processpreset();

  // Uncomment to debug sensors (Serial.begin already enabled in setup):
  //finalcheck();
}
