//________________________________________________________________________________________________________________//
//                                                                                                                //
//   Persistent motor-bias calibration (LittleFS)                                                                 //
//                                                                                                                //
//   All six drive biases live in /bias.cfg on a LittleFS flash partition. The sketch upload only writes the      //
//   program area, so the config survives reflashing - it is only lost on a full-chip erase or when the           //
//   Flash Size layout is changed in the IDE.                                                                     //
//                                                                                                                //
//   REQUIRED upload setting:  Flash Size: 2MB (Sketch: 1984KB, FS: 64KB)                                         //
//   (with "No FS" selected, LittleFS.begin() fails and the compiled defaults below are used instead)             //
//                                                                                                                //
//   One shared set of four biases (fl, fr, bl, br) is used by BOTH modes: RC applies them per direction,         //
//   and Auto uses fl/fr for its (forward) drive. Tune over the Serial Monitor (9600 baud, newline ending):       //
//       show          - print the current bias values                                                            //
//       fl 0.945      - set a bias (keys: fl, fr = forward left/right; bl, br = backward left/right)             //
//       save          - write the current values to /bias.cfg (persists across reflash)                          //
//       load          - re-read /bias.cfg, discarding unsaved changes                                            //
//       defaults      - restore the compiled defaults (RAM only until 'save')                                    //
//                                                                                                                //
//   Commands are processed from loop(), so tuning works while the robot is idle/armed in either mode.            //
//   During an Auto match mainprgm() blocks, so commands are picked up once the match stops.                      //
//________________________________________________________________________________________________________________//

#include <LittleFS.h>

#define BIAS_FILE "/bias.cfg"

// Compiled-in defaults, used when no config file exists yet (first boot after a
// full erase) or when LittleFS cannot be mounted.
// VX Alpha R1.00 L0.86
const float flbiasDefault = 0.945;  // forward, left motor (also Auto drive)
const float frbiasDefault = 1.00;   // forward, right motor (also Auto drive)
const float blbiasDefault = 1.00;   // backward, left motor
const float brbiasDefault = 1.00;   // backward, right motor

// Runtime values used by the drive code (MainProg / Presets / RC_Mode).
// Loaded from /bias.cfg at boot, tunable over serial.
float flbias = flbiasDefault;
float frbias = frbiasDefault;
float blbias = blbiasDefault;
float brbias = brbiasDefault;

bool biasFsOk = false;
String biasCmdBuf = "";

// Keep tuning values in a sane range - a typo like "l 945" must not
// command a 945x speed multiplier.
float clampBias(float v) {
  if (v < 0.0) return 0.0;
  if (v > 2.0) return 2.0;
  return v;
}

float* biasByKey(String key) {
  if (key == "fl") return &flbias;
  if (key == "fr") return &frbias;
  if (key == "bl") return &blbias;
  if (key == "br") return &brbias;
  return nullptr;
}

void biasApplyDefaults() {
  flbias = flbiasDefault;
  frbias = frbiasDefault;
  blbias = blbiasDefault;
  brbias = brbiasDefault;
}

void printBias() {
  Serial.print("[bias] FL=");
  Serial.print(flbias, 3);
  Serial.print("  FR=");
  Serial.print(frbias, 3);
  Serial.print("  BL=");
  Serial.print(blbias, 3);
  Serial.print("  BR=");
  Serial.println(brbias, 3);
}

bool loadBiasConfig() {
  File f = LittleFS.open(BIAS_FILE, "r");
  if (!f) return false;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    int eq = line.indexOf('=');
    if (eq <= 0) continue;  // skip blank/comment/malformed lines
    String key = line.substring(0, eq);
    key.trim();
    key.toLowerCase();
    float* target = biasByKey(key);
    if (target) *target = clampBias(line.substring(eq + 1).toFloat());
  }
  f.close();
  return true;
}

bool saveBiasConfig() {
  File f = LittleFS.open(BIAS_FILE, "w");
  if (!f) return false;
  f.print("fl=");
  f.println(flbias, 3);
  f.print("fr=");
  f.println(frbias, 3);
  f.print("bl=");
  f.println(blbias, 3);
  f.print("br=");
  f.println(brbias, 3);
  f.close();
  return true;
}

// Call once from setup() after Serial.begin(). Mounts LittleFS (auto-formats a
// blank partition), loads /bias.cfg, or seeds it with the defaults on first boot.
void biasConfigInit() {
  biasFsOk = LittleFS.begin();
  if (!biasFsOk) {
    Serial.println("[bias] LittleFS mount FAILED - check Flash Size setting (need a FS partition). Using compiled defaults.");
    return;
  }
  if (loadBiasConfig()) {
    Serial.println("[bias] loaded /bias.cfg");
  } else {
    Serial.println("[bias] no /bias.cfg yet - seeding it with compiled defaults");
    saveBiasConfig();
  }
  printBias();
}

void handleBiasCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();
  if (cmd.length() == 0) return;

  if (cmd == "show") {
    printBias();
    return;
  }
  if (cmd == "save") {
    if (!biasFsOk) {
      Serial.println("[bias] save FAILED - LittleFS not mounted");
    } else {
      Serial.println(saveBiasConfig() ? "[bias] saved to /bias.cfg" : "[bias] save FAILED");
    }
    return;
  }
  if (cmd == "load") {
    if (biasFsOk && loadBiasConfig()) {
      Serial.println("[bias] reloaded /bias.cfg");
      printBias();
    } else {
      Serial.println("[bias] load FAILED");
    }
    return;
  }
  if (cmd == "defaults") {
    biasApplyDefaults();
    Serial.println("[bias] compiled defaults restored (RAM only - 'save' to persist)");
    printBias();
    return;
  }

  // "<key> <value>" e.g. "l 0.945"
  int sp = cmd.indexOf(' ');
  if (sp > 0) {
    float* target = biasByKey(cmd.substring(0, sp));
    if (target) {
      *target = clampBias(cmd.substring(sp + 1).toFloat());
      printBias();
      Serial.println("[bias] set (RAM only - 'save' to persist)");
      return;
    }
  }

  Serial.println("[bias] unknown command - try: show | save | load | defaults | <fl|fr|bl|br> <value>");
}

// Call every loop() iteration - non-blocking, reads one command per line.
void biasSerialTask() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (biasCmdBuf.length() > 0) {
        handleBiasCommand(biasCmdBuf);
        biasCmdBuf = "";
      }
    } else if (biasCmdBuf.length() < 40) {
      biasCmdBuf += c;
    }
  }
}
