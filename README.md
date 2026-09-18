# Apachon CTRL-01-MK1 — 8 Knobs MIDI Controller (V7)

DIY 8-knob USB MIDI controller. Configurable from a browser editor via Web MIDI + SysEx.
No drivers, no installers.

---

## BOM

| Ref | Part | Qty |
|---|---|---|
| U1 | Arduino Pro Micro (ATmega32U4) | 1 |
| IC1 | CD74HC4067 (16-ch analog mux) | 1 |
| RV1–RV8 | 10 kΩ linear pot, 16 mm | 8 |
| C1, C2 | 100 nF ceramic capacitor | 2 |

---

## Pins

| Pro Micro | → | Component |
|---|---|---|
| D4 | → | IC1 pin 10 (S0) |
| D5 | → | IC1 pin 11 (S1) |
| D6 | → | IC1 pin 14 (S2) |
| D7 | → | IC1 pin 13 (S3) |
| A0 | → | IC1 pin 1 (COM) |
| VCC | → | IC1 pin 24, RVx pin 1, Cx pin 1 |
| GND | → | IC1 pins 12 + 15, RVx pin 3, Cx pin 2 |

---

## Pot → MUX channel

| Pot | MUX pin |
|---|---|
| RV1 | I0 (pin 9) |
| RV2 | I4 (pin 5) |
| RV3 | I6 (pin 3) |
| RV4 | I7 (pin 2) |
| RV5 | I8 (pin 23) |
| RV6 | I10 (pin 21) |
| RV7 | I13 (pin 18) |
| RV8 | I15 (pin 16) |

---

## Build steps

1. Flash `V7-Apachon_8Knobs.ino` to Pro Micro (board: **Arduino Leonardo**).
2. Wire per the tables above.
3. Place C1 near IC1 VCC pin, C2 near U1 VCC pin.
4. Open `V7-Apachon_Editor.html` in Chrome or Edge.
5. Select the Pro Micro port, click **Connecter**.

---

## Libraries

- `MIDIUSB.h` (Arduino built-in)
- `EEPROM.h` (Arduino built-in)

**Not used:** Control_Surface (it filters SysEx, breaking editor comms).

---

## SysEx protocol

Header: `F0 7D 01 [CMD] [PAYLOAD] F7`

### Editor → Arduino

| CMD | Action | Payload |
|---|---|---|
| 0x10 | Set CC | [idx] [cc 1–127] |
| 0x60 | Set mode | [idx] [0=Normal, 1=Switch] |
| 0x61 | Set curve | [idx] [0–4] |
| 0x40 | Set channel | [ch 1–16] |
| 0x30 | Load bank | [1] |
| 0x31 | Save bank | [1] |
| 0x50 | Request dump / keepalive | [0] |

### Arduino → Editor

| CMD | Action | Payload |
|---|---|---|
| 0x51 | Dump | 8×CC + 8×Mode + 8×Curve + Ch + Bank (26 B) |
| 0x62 | Touch info | [idx] [raw] [curved] [mode] [curve] |
| 0x70 | ACK | [cmd] [0x00] |
| 0x71 | ERROR | [cmd] [code] |

---

## Curves (Normal mode only)

| ID | Name | Formula |
|---|---|---|
| 0 | Linear | y = x |
| 1 | Inverse | y = 127 − x |
| 2 | Exponential | y = x² / 127 |
| 3 | Logarithmic | y = √(x/127) × 127 |
| 4 | Smoothstep | y = x²(3 − 2x) |

---

## Modes

| ID | Name | Behavior |
|---|---|---|
| 0 | Normal | Continuous CC 0–127 (curve applied) |
| 1 | Switch | CC 0 or 127 (ON ≥ 68, OFF ≤ 60) |

---

## Handshake (V7)

- Editor sends a keepalive every **3 s**.
- Arduino sets `browserConnected = true`.
- If no message for **6 s**, `browserConnected = false` → no more SysEx 0x62 touch feedback.
- CC messages are always sent (never gated by handshake).

**Effect:** USB bandwidth stays clean when no editor is open.

---

## EEPROM layout

| Addr | Size | Content |
|---|---|---|
| 0 | 1 B | Magic (0xAF) — forced reinit if changed |
| 1 | 1 B | MIDI channel (1–16) |
| 2–9 | 8 B | CC map |
| 10–17 | 8 B | Mode map |
| 18–25 | 8 B | Curve map |

⚠️ **V7 uses magic 0xAF** — first boot will wipe V6 settings.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| No port in editor | Use Chrome or Edge, not Firefox |
| No response | Click Debug in editor to listen on all ports |
| No live update | Reconnect — handshake re-enables SysEx |
| Settings revert after power cycle | EEPROM write failed — check magic 0xAF |
| Switch toggles erratically | Increase hysteresis to 75/55 in firmware |
| USB bandwidth warnings | Disconnect the editor (handshake disables SysEx) |

---



