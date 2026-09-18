// ============================================================================
// APACHON CTRL-01-MK1 - 8 KNOBS (V7)
// Hardware : Pro Micro (ATmega32U4)
// Pins     : MUX S0=4, S1=5, S2=6, S3=7, Signal=A0
// MUX ch   : 0, 4, 6, 7, 8, 10, 13, 15
// Libs     : MIDIUSB, EEPROM  
//
// V7 : Touch info SysEx envoyé UNIQUEMENT si le navigateur est connecté.
//      Un keepalive 0x50 du navigateur toutes les 3s maintient la connexion.
//      Timeout de 6s sans ping → browserConnected = false → plus de SysEx 0x62.
// ============================================================================

#include <MIDIUSB.h>
#include <EEPROM.h>

// ----------------------------------------------------------------------------
// PINS
// ----------------------------------------------------------------------------
const uint8_t MUX_S0  = 4;
const uint8_t MUX_S1  = 5;
const uint8_t MUX_S2  = 6;
const uint8_t MUX_S3  = 7;
const uint8_t MUX_SIG = A0;

const uint8_t NUM_KNOBS   = 8;
const uint8_t MIDI_CH_MIN = 1;
const uint8_t MIDI_CH_MAX = 16;

const uint8_t muxChannels[NUM_KNOBS] = {0, 4, 6, 7, 8, 10, 13, 15};

// ----------------------------------------------------------------------------
// COURBES
// 0 = Linéaire     : 1:1
// 1 = Inverse      : 127 - x
// 2 = Exponentiel  : x² / 127  (lent au début, rapide à la fin)
// 3 = Logarithmique: √x · 127  (rapide au début, lent à la fin)
// 4 = Smoothstep   : S-curve   (lent aux deux extrémités)
// ----------------------------------------------------------------------------
#define CURVE_LINEAR  0
#define CURVE_INVERSE 1
#define CURVE_EXP     2
#define CURVE_LOG     3
#define CURVE_SMOOTH  4
#define CURVE_COUNT   5

// ----------------------------------------------------------------------------
// EEPROM
// Addr 0     : MAGIC (0xAF)
// Addr 1     : Canal MIDI
// Addr 2-9   : CC (8 octets)
// Addr 10-17 : Modes (8 octets)
// Addr 18-25 : Courbes (8 octets)
// ----------------------------------------------------------------------------
const int     EEPROM_MAGIC       = 0;
const int     EEPROM_MIDI_CH     = 1;
const int     EEPROM_CC_MAP      = 2;
const int     EEPROM_KNOB_MODES  = 10;
const int     EEPROM_KNOB_CURVES = 18;
const uint8_t MAGIC_VALUE        = 0xAF;  // bumped V7

// ----------------------------------------------------------------------------
// VARIABLES GLOBALES
// ----------------------------------------------------------------------------
uint8_t knobCC[NUM_KNOBS];
uint8_t knobModes[NUM_KNOBS];
uint8_t knobCurve[NUM_KNOBS];
uint8_t knobValues[NUM_KNOBS] = {0};
uint8_t knobPrev[NUM_KNOBS]   = {0};
uint8_t midiChannel = 1;

// Tampon SysEx réception — taille fixe, pas de VLA
#define SYSEX_BUF_SIZE 64
uint8_t sysexBuf[SYSEX_BUF_SIZE];
uint8_t sysexLen = 0;
bool    inSysEx  = false;

// Présence navigateur — V7
bool     browserConnected = false;
uint32_t lastBrowserPing  = 0;
#define  BROWSER_TIMEOUT_MS 6000UL  // 6s sans ping → déconnecté

// ----------------------------------------------------------------------------
// MUX
// ----------------------------------------------------------------------------
void setMux(uint8_t ch) {
  digitalWrite(MUX_S0,  ch       & 1);
  digitalWrite(MUX_S1, (ch >> 1) & 1);
  digitalWrite(MUX_S2, (ch >> 2) & 1);
  digitalWrite(MUX_S3, (ch >> 3) & 1);
}

// ----------------------------------------------------------------------------
// COURBES — math entière sauf Smoothstep
// ----------------------------------------------------------------------------
uint8_t applyCurve(uint8_t raw, uint8_t curveType) {
  switch (curveType) {
    case CURVE_LINEAR:  return raw;
    case CURVE_INVERSE: return 127 - raw;
    case CURVE_EXP:     return (uint8_t)(((uint16_t)raw * raw) / 127);
    case CURVE_LOG: {
      if (raw == 0)   return 0;
      if (raw >= 127) return 127;
      // sqrt entier par approximation Newton
      uint16_t n = (uint16_t)raw * 128;
      uint16_t x = n / 2;
      for (uint8_t k = 0; k < 8; k++) x = (x + n / x) / 2;
      uint8_t r = (uint8_t)((uint32_t)x * 127 / 128);
      return (r > 127) ? 127 : r;
    }
    case CURVE_SMOOTH: {
      float t = raw / 127.0f;
      return (uint8_t)(t * t * (3.0f - 2.0f * t) * 127.0f + 0.5f);
    }
    default: return raw;
  }
}

// ----------------------------------------------------------------------------
// EEPROM — sauvegarde ciblée pour économiser les cycles
// ----------------------------------------------------------------------------
void writeByte(int addr, uint8_t val) { EEPROM.update(addr, val); }
uint8_t readByte(int addr)            { return EEPROM.read(addr); }

void applyDefaults() {
  midiChannel = 1;
  for (uint8_t i = 0; i < NUM_KNOBS; i++) {
    knobCC[i]    = 20 + i;
    knobModes[i] = 0;
    knobCurve[i] = CURVE_LINEAR;
  }
}

void saveAll() {
  writeByte(EEPROM_MIDI_CH, midiChannel);
  for (uint8_t i = 0; i < NUM_KNOBS; i++) {
    writeByte(EEPROM_CC_MAP      + i, knobCC[i]);
    writeByte(EEPROM_KNOB_MODES  + i, knobModes[i]);
    writeByte(EEPROM_KNOB_CURVES + i, knobCurve[i]);
  }
  writeByte(EEPROM_MAGIC, MAGIC_VALUE);
}

// Sauvegarde ciblée — évite d'écrire les 26 octets à chaque changement
void saveCC()     { for (uint8_t i=0;i<NUM_KNOBS;i++) writeByte(EEPROM_CC_MAP+i,      knobCC[i]);    }
void saveModes()  { for (uint8_t i=0;i<NUM_KNOBS;i++) writeByte(EEPROM_KNOB_MODES+i,  knobModes[i]); }
void saveCurves() { for (uint8_t i=0;i<NUM_KNOBS;i++) writeByte(EEPROM_KNOB_CURVES+i, knobCurve[i]); }

void loadAll() {
  if (readByte(EEPROM_MAGIC) != MAGIC_VALUE) {
    applyDefaults();
    saveAll();
    return;
  }
  uint8_t ch = readByte(EEPROM_MIDI_CH);
  midiChannel = (ch >= MIDI_CH_MIN && ch <= MIDI_CH_MAX) ? ch : 1;
  for (uint8_t i = 0; i < NUM_KNOBS; i++) {
    uint8_t cc = readByte(EEPROM_CC_MAP + i);
    knobCC[i]  = (cc >= 1 && cc <= 127) ? cc : (20 + i);
    uint8_t m  = readByte(EEPROM_KNOB_MODES + i);
    knobModes[i] = (m <= 1) ? m : 0;
    uint8_t c  = readByte(EEPROM_KNOB_CURVES + i);
    knobCurve[i] = (c < CURVE_COUNT) ? c : CURVE_LINEAR;
  }
}

// ----------------------------------------------------------------------------
// ENVOI SYSEX BRUT — buffer fixe, pas de VLA
// Format : 0xF0 0x7D 0x01 [cmd] [data...] 0xF7
// ----------------------------------------------------------------------------
#define SYSEX_SEND_BUF 64
void rawSysExSend(uint8_t cmd, const uint8_t *data, uint8_t dataLen) {
  if (dataLen > SYSEX_SEND_BUF - 4) return;
  uint8_t msg[SYSEX_SEND_BUF];
  msg[0] = 0xF0; msg[1] = 0x7D; msg[2] = 0x01; msg[3] = cmd;
  for (uint8_t i = 0; i < dataLen; i++) msg[4 + i] = data[i];
  uint8_t fullLen = 4 + dataLen;

  uint8_t i = 0;
  while (i + 3 < fullLen) {
    midiEventPacket_t p = {0x04, msg[i], msg[i+1], msg[i+2]};
    MidiUSB.sendMIDI(p); i += 3;
  }
  uint8_t rem = fullLen - i;
  if      (rem == 1) { midiEventPacket_t p = {0x06, msg[i],   0xF7,     0x00}; MidiUSB.sendMIDI(p); }
  else if (rem == 2) { midiEventPacket_t p = {0x07, msg[i],   msg[i+1], 0xF7}; MidiUSB.sendMIDI(p); }
  else               { midiEventPacket_t p = {0x04, msg[i],   msg[i+1], msg[i+2]}; MidiUSB.sendMIDI(p);
                       midiEventPacket_t q = {0x05, 0xF7,     0x00,     0x00};     MidiUSB.sendMIDI(q); }
  MidiUSB.flush();
}

// Touch info : [idx, rawVal, curvedVal, mode, curve]
// Envoyé UNIQUEMENT si browserConnected — V7
void sendTouchInfo(uint8_t idx, uint8_t rawVal, uint8_t curvedVal, uint8_t mode, uint8_t curve) {
  if (!browserConnected) return;
  uint8_t d[5] = {idx, rawVal, curvedVal, mode, curve};
  rawSysExSend(0x62, d, 5);
}

void sendDump() {
  uint8_t dump[26];
  memcpy(&dump[0],  knobCC,    8);
  memcpy(&dump[8],  knobModes, 8);
  memcpy(&dump[16], knobCurve, 8);
  dump[24] = midiChannel;
  dump[25] = 1;
  rawSysExSend(0x51, dump, 26);
}

void sendAck(uint8_t cmd)            { uint8_t d[2]={cmd,0x00}; rawSysExSend(0x70,d,2); }
void sendError(uint8_t cmd,uint8_t e){ uint8_t d[2]={cmd,e};    rawSysExSend(0x71,d,2); }

// ----------------------------------------------------------------------------
// RÉCEPTION SYSEX
// ----------------------------------------------------------------------------
void pingBrowser() {
  browserConnected = true;
  lastBrowserPing  = millis();
}

void processSysEx() {
  if (sysexLen < 3) return;
  if (sysexBuf[0] != 0x7D || sysexBuf[1] != 0x01) return;

  uint8_t        cmd        = sysexBuf[2];
  const uint8_t *payload    = &sysexBuf[3];
  uint8_t        payloadLen = sysexLen - 3;

  // Tout message reçu = navigateur actif
  pingBrowser();

  switch (cmd) {

    case 0x10: { // SET_KNOB_CC
      if (payloadLen < 2) break;
      uint8_t idx = payload[0], cc = payload[1];
      if (idx < NUM_KNOBS && cc >= 1 && cc <= 127) {
        knobCC[idx] = cc; saveCC(); sendAck(cmd);
      } else sendError(cmd, 0x02);
      break;
    }

    case 0x60: { // SET_KNOB_MODE
      if (payloadLen < 2) break;
      uint8_t idx = payload[0], mode = payload[1];
      if (idx < NUM_KNOBS && mode <= 1) {
        knobModes[idx] = mode; saveModes(); sendAck(cmd);
      } else sendError(cmd, 0x02);
      break;
    }

    case 0x61: { // SET_KNOB_CURVE
      if (payloadLen < 2) break;
      uint8_t idx = payload[0], curve = payload[1];
      if (idx < NUM_KNOBS && curve < CURVE_COUNT) {
        knobCurve[idx] = curve; saveCurves(); sendAck(cmd);
      } else sendError(cmd, 0x02);
      break;
    }

    case 0x40: { // SET_MIDI_CHANNEL
      if (payloadLen < 1) break;
      uint8_t ch = payload[0];
      if (ch >= MIDI_CH_MIN && ch <= MIDI_CH_MAX) {
        midiChannel = ch; writeByte(EEPROM_MIDI_CH, midiChannel); sendAck(cmd);
      } else sendError(cmd, 0x03);
      break;
    }

    case 0x31: { // SAVE_BANK
      if (payloadLen < 1 || payload[0] != 1) break;
      saveAll(); sendAck(cmd);
      break;
    }

    case 0x30: { // LOAD_BANK
      if (payloadLen < 1 || payload[0] != 1) break;
      loadAll();
      for (uint8_t i = 0; i < NUM_KNOBS; i++) knobPrev[i] = 0;
      sendDump(); sendAck(cmd);
      break;
    }

    case 0x50: { // REQUEST_DUMP / KEEPALIVE
      sendDump();
      break;
    }

    default: break;
  }
}

void readRawMIDI() {
  midiEventPacket_t pkt;
  while ((pkt = MidiUSB.read()).header != 0) {
    uint8_t cin = pkt.header & 0x0F;
    switch (cin) {
      case 0x04:
        if (pkt.byte1 == 0xF0) {
          sysexLen = 0; inSysEx = true;
          if (pkt.byte2 != 0xF7 && sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte2;
          if (pkt.byte3 != 0xF7 && sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte3;
        } else if (inSysEx) {
          if (sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte1;
          if (sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte2;
          if (sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte3;
        }
        break;
      case 0x05:
        if (inSysEx) { inSysEx = false; processSysEx(); }
        break;
      case 0x06:
        if (inSysEx) {
          if (pkt.byte1 != 0xF7 && sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte1;
          inSysEx = false; processSysEx();
        }
        break;
      case 0x07:
        if (inSysEx) {
          if (pkt.byte1 != 0xF7 && sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte1;
          if (pkt.byte2 != 0xF7 && sysexLen < SYSEX_BUF_SIZE) sysexBuf[sysexLen++] = pkt.byte2;
          inSysEx = false; processSysEx();
        }
        break;
      default: break;
    }
  }
}

// ----------------------------------------------------------------------------
// SCAN + ENVOI CC
// ----------------------------------------------------------------------------
void scanKnobs() {
  for (uint8_t i = 0; i < NUM_KNOBS; i++) {
    setMux(muxChannels[i]);
    delayMicroseconds(50);
    knobValues[i] = analogRead(MUX_SIG) >> 3;
  }
}

void sendKnobs() {
  for (uint8_t i = 0; i < NUM_KNOBS; i++) {
    if (abs((int)knobValues[i] - (int)knobPrev[i]) > 1) {
      uint8_t raw = knobValues[i];
      uint8_t val;

      if (knobModes[i] == 1) {
        // Switch avec hystérésis
        val = (raw >= 68) ? 127 : (raw <= 60 ? 0 : (knobPrev[i] >= 64 ? 127 : 0));
      } else {
        val = applyCurve(raw, knobCurve[i]);
      }

      // Envoi CC MIDI
      midiEventPacket_t pkt = {
        0x0B,
        (uint8_t)(0xB0 | ((midiChannel - 1) & 0x0F)),
        knobCC[i], val
      };
      MidiUSB.sendMIDI(pkt);
      MidiUSB.flush();

      // Touch info — seulement si le navigateur est actif (V7)
      sendTouchInfo(i, raw, val, knobModes[i], knobCurve[i]);

      knobPrev[i] = raw;
    }
  }
}

// ----------------------------------------------------------------------------
// SETUP / LOOP
// ----------------------------------------------------------------------------
void setup() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  loadAll();
  scanKnobs();
  for (uint8_t i = 0; i < NUM_KNOBS; i++) knobPrev[i] = knobValues[i];
}

void loop() {
  // Timeout navigateur — V7
  if (browserConnected && (millis() - lastBrowserPing > BROWSER_TIMEOUT_MS)) {
    browserConnected = false;
  }

  readRawMIDI();
  scanKnobs();
  sendKnobs();
  delay(1);
}
