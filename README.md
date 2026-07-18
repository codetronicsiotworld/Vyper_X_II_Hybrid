# Vyper X II Hybrid V2 (IRST)

Firmware for the **Vyper X II** sumo robot, running on an RP2350 board.
One firmware, two modes:

- **Auto (AT)** — autonomous sumo: finds the opponent, attacks, and stays inside the ring on its own.
- **RC** — manual driving with a standard 2-channel RC transmitter (skid-steer).

## Selecting the mode

The mode is read from a switch on **GP19 once at boot**:

| Switch (GP19) | Mode |
|---|---|
| LOW / disconnected | Auto (default) |
| HIGH | RC |

Flip the switch, then **power-cycle or reset** — the mode does not change while running.

## Status LED (WS2812 on GP25)

| Colour | Meaning |
|---|---|
| 2 red blinks at boot | Auto mode latched |
| 2 blue blinks at boot | RC mode latched |
| Dim red | Auto: armed, waiting for the IR start signal |
| Solid red | Auto: match running |
| Blue heartbeat blink | RC: signal OK |
| Amber | RC: signal lost — motors stopped (failsafe) |

## Motor bias calibration (persistent)

Motors are never perfectly matched, so the firmware applies a left/right trim ("bias") to drive straight.
The four bias values are stored in a file (`/bias.cfg`) on the board's flash filesystem, so they
**survive reflashing** — you only lose them on a full-chip erase.

Tune them from the Arduino Serial Monitor (**9600 baud**, newline line ending):

| Command | What it does |
|---|---|
| `show` | Print the current bias values |
| `fl 0.95` | Set a bias — keys: `fl`, `fr` (forward left/right), `bl`, `br` (backward left/right) |
| `save` | Store the values permanently |
| `load` | Re-read the stored values, discarding unsaved changes |
| `defaults` | Restore the factory defaults (then `save` to keep them) |

Both modes share the same bias set; Auto mode uses `fl`/`fr` for its drive.
Values are clamped to 0–2. Tuning works whenever the robot is idle/armed; during an
Auto match, commands are processed once the match stops.

## Building & uploading

This project uses the custom **[Codetronics Exocore](https://github.com/codetronicsiotworld/codetronics-exocore)**
board package with the **Hermes** board for seamless compiling and uploading.

1. In the Arduino IDE, open **File → Preferences** and add this to *Additional boards manager URLs*:
   ```
   https://raw.githubusercontent.com/codetronicsiotworld/codetronics-exocore/main/package_kratos_hermes_vyper_index.json
   ```
2. Open **Tools → Board → Boards Manager**, search for **Codetronics Exocore** and install it.
3. Select **Tools → Board → Hermes**.
4. Install the libraries: `MotorControl`, `Adafruit NeoPixel` (Library Manager).
5. If a Flash Size option is shown, pick a layout with an FS partition
   (e.g. **2MB — Sketch: 1984KB, FS: 64KB**) — the filesystem stores the bias calibration.

## File overview

| File | Contents |
|---|---|
| `VyperX_II_Hybrid_V2_IRST.ino` | Setup, pin map, mode latch, status LED, main loop |
| `MainProg.ino` | Autonomous logic: sensor reading, attack/edge-avoid state machine, acceleration |
| `Movement_def.ino` | Basic moves (forward, backward, turns) and the timer/debounce helpers |
| `Presets.ino` | IR-selected opening moves for the start of a match |
| `RC_Mode.ino` | Manual RC driving with failsafe |
| `Bias_Config.ino` | Persistent bias storage and the serial tuning commands |
| `Serial_Check.ino` | Sensor debug printout |
