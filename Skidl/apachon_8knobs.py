# ==============================================================================
# APACHON CTRL-01-MK1 - 8 KNOBS
# SKiDL Schematic — V2 (Final)
# MCU  : Pro Micro (ATmega32U4) — onboard USB, USB power only
# MUX  : CD74HC4067M (16-channel analog mux, SOIC-24)
# POTS : 8x 10K 16mm Alps RK163 potentiometers
# MUX select pins : S0=D4, S1=D5, S2=D6, S3=D7
# MUX signal      : A0
# MUX channels used: 0, 4, 6, 7, 8, 10, 13, 15
# Power : USB 5V only — no external supply
# ==============================================================================

import os

# ── KiCad 10 library paths ────────────────────────────────────────────────
KICAD_PATH = r'C:\Program Files\KiCad\10.0\share\kicad'
os.environ['KICAD6_SYMBOL_DIR']     = os.path.join(KICAD_PATH, 'symbols')
os.environ['KICAD7_SYMBOL_DIR']     = os.path.join(KICAD_PATH, 'symbols')
os.environ['KICAD8_SYMBOL_DIR']     = os.path.join(KICAD_PATH, 'symbols')
os.environ['KICAD9_SYMBOL_DIR']     = os.path.join(KICAD_PATH, 'symbols')
os.environ['KICAD10_SYMBOL_DIR']    = os.path.join(KICAD_PATH, 'symbols')
os.environ['KICAD10_FOOTPRINT_DIR'] = os.path.join(KICAD_PATH, 'footprints')

from skidl import *

set_default_tool(KICAD10)

# ==============================================================================
# NETS
# ==============================================================================
VCC, GND       = Net('VCC'), Net('GND')
S0, S1, S2, S3 = Net('MUX_S0'), Net('MUX_S1'), Net('MUX_S2'), Net('MUX_S3')
SIG            = Net('MUX_SIG')
CH             = [Net(f'MUX_CH{i}') for i in range(16)]

POT_CHANNELS = [0, 4, 6, 7, 8, 10, 13, 15]

# ==============================================================================
# PRO MICRO (ATmega32U4) — represented by a 2x12 pin header
# Odd/even pin numbering:
#   Left row (odd):  1=TX  3=GND  5=D2  7=D4  9=D6  11=D8
#   Right row (even):2=RX  4=GND  6=D3  8=D5  10=D7 12=D9
#   ...continued:    14=D14 16=D16 18=A0 20=A2 22=A1 24=A3
# ==============================================================================
u1 = Part(
    'Connector_Generic', 'Conn_02x12_Odd_Even',
    footprint='Connector_PinHeader_2.54mm:PinHeader_2x12_P2.54mm_Vertical'
)
u1.ref   = 'U1'
u1.value = 'Pro_Micro_ATmega32U4'

# Digital outputs → MUX select lines
u1['7']  += S0      # D4 → S0
u1['8']  += S1      # D5 → S1
u1['10'] += S2      # D6 → S2
u1['11'] += S3      # D7 → S3

# Power & ground
u1['3']  += GND     # GND
u1['4']  += GND     # GND
u1['24'] += VCC     # VCC (+5V)

# Analog input from MUX
u1['18'] += SIG     # A0 → MUX SIG

# ==============================================================================
# CD74HC4067M — 16-channel analog multiplexer
# Pin names confirmed from KiCad symbol:
#   COM=1  I0=9  I1=8  I2=7  I3=6  I4=5  I5=4  I6=3  I7=2
#   S0=10  S1=11  GND=12  S3=13  S2=14  ~{E}=15  VCC=24
#   I8=23  I9=22  I10=21  I11=20  I12=19  I13=18  I14=17  I15=16
# ==============================================================================
ic1 = Part(
    '74xx', 'CD74HC4067M',
    footprint='Package_SO:SOIC-24W_7.5x15.4mm_P1.27mm'
)
ic1.ref   = 'IC1'
ic1.value = 'CD74HC4067M'

# Power
ic1['VCC'] += VCC
ic1['GND'] += GND

# Enable (active-low) — tie to GND so the mux is always active
ic1['~{E}'] += GND

# Select lines from Pro Micro
ic1['S0'] += S0
ic1['S1'] += S1
ic1['S2'] += S2
ic1['S3'] += S3

# Common signal → Pro Micro A0
ic1['COM'] += SIG

# All 16 channel inputs (only 8 are wired to pots)
for i in range(16):
    ic1[f'I{i}'] += CH[i]

# ==============================================================================
# DECOUPLING CAPACITORS (100nF each)
# ==============================================================================
c1 = Part('Device', 'C', value='100nF',
          footprint='Capacitor_SMD:C_0805_2012Metric')
c1.ref = 'C1'
c1['1'] += VCC
c1['2'] += GND

c2 = Part('Device', 'C', value='100nF',
          footprint='Capacitor_SMD:C_0805_2012Metric')
c2.ref = 'C2'
c2['1'] += VCC
c2['2'] += GND

# ==============================================================================
# 8 POTENTIOMETERS — wired as voltage dividers
#   Pin 1 (CW)    → VCC
#   Pin 2 (Wiper) → MUX channel
#   Pin 3 (CCW)   → GND
# ==============================================================================
pots = []
for n, ch in enumerate(POT_CHANNELS):
    p = Part(
        'Device', 'R_Potentiometer', value='10K',
        footprint='Potentiometer_THT:Potentiometer_Alps_RK163_Single_Horizontal'
    )
    p.ref = f'RV{n+1}'
    p['1'] += VCC
    p['2'] += CH[ch]     # wiper → assigned MUX channel
    p['3'] += GND
    pots.append(p)

# ==============================================================================
# ERC + NETLIST
# ==============================================================================
ERC()
generate_netlist(tool=KICAD10)

print()
print('=== Apachon 8-Knobs schematic generated OK ===')
print(f'  Pots    : {len(pots)} x 10K (RV1-RV8)')
print(f'  MUX     : CD74HC4067M (IC1)')
print(f'  MCU     : Pro Micro ATmega32U4 (U1)')
print(f'  Decoup  : C1, C2 = 100nF')
print(f'  Power   : USB 5V only')
print(f'  Netlist : apachon_8knobs.net')