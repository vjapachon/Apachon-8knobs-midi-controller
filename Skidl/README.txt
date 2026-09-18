# SKiDL Setup Guide — Apachon 8-Knobs Netlist

How to install SKiDL on Windows, wire it to KiCad 10, and generate apachon_8knobs.net from the Python circuit script.

## Prerequisites

| Software | Version | Download |
|---|---|---|
| KiCad | 10.0 | https://www.kicad.org/download/ |
| Python | 3.10 or newer | https://www.python.org/downloads/ |

## STEP 1 — Install KiCad 10

1. Download the Windows installer from kicad.org.
2. Run the installer with default options.
3. Make sure "Symbols" and "Footprints" are checked.
4. Default install path: C:\Program Files\KiCad\10.0\

Verify the symbols folder exists:

    Test-Path "C:\Program Files\KiCad\10.0\share\kicad\symbols"

Should return True.

## STEP 2 — Install Python

1. Download from python.org.
2. During install, CHECK the box "Add python.exe to PATH".
3. Verify in PowerShell:

    python --version

Should print something like Python 3.13.x.

## STEP 3 — Set KiCad symbol environment variables

SKiDL scans every KiCad version it knows about (6, 7, 8, 9, 10). You must set all of them to avoid errors.

Open PowerShell and run:

    [System.Environment]::SetEnvironmentVariable('KICAD6_SYMBOL_DIR',  'C:\Program Files\KiCad\10.0\share\kicad\symbols', 'User')
    [System.Environment]::SetEnvironmentVariable('KICAD7_SYMBOL_DIR',  'C:\Program Files\KiCad\10.0\share\kicad\symbols', 'User')
    [System.Environment]::SetEnvironmentVariable('KICAD8_SYMBOL_DIR',  'C:\Program Files\KiCad\10.0\share\kicad\symbols', 'User')
    [System.Environment]::SetEnvironmentVariable('KICAD9_SYMBOL_DIR',  'C:\Program Files\KiCad\10.0\share\kicad\symbols', 'User')
    [System.Environment]::SetEnvironmentVariable('KICAD10_SYMBOL_DIR', 'C:\Program Files\KiCad\10.0\share\kicad\symbols', 'User')
    [System.Environment]::SetEnvironmentVariable('KICAD10_FOOTPRINT_DIR', 'C:\Program Files\KiCad\10.0\share\kicad\footprints', 'User')

Close PowerShell completely and open a new one for the variables to load.

Verify:

    echo $env:KICAD10_SYMBOL_DIR

Should print the path.

## STEP 4 — Install SKiDL

Option A — Stable version (works on Python <= 3.12):

    pip install skidl

Option B — Development version (required for Python 3.13):

The stable release crashes on Python 3.13 with a logger error ('SkidlLogFileHandler' object has no attribute 'filters'). Use the development branch:

    pip install --force-reinstall --no-cache-dir git+https://github.com/devbisme/skidl@development

Verify install:

    pip show skidl

## STEP 5 — Create a working folder

Do NOT work from C:\ root — SKiDL tries to write log files in the current directory, and Windows blocks writes to the root.

    mkdir C:\apachon
    cd C:\apachon

Put apachon_8knobs.py in this folder.

## STEP 6 — Run the script

From the working folder:

    python apachon_8knobs.py

Expected output:

    === Apachon 8-Knobs schematic generated OK ===
      Pots    : 8 x 10K (RV1-RV8)
      MUX     : CD74HC4067M (IC1)
      MCU     : Pro Micro ATmega32U4 (U1)
      Decoup  : C1, C2 = 100nF
      Power   : USB 5V only
      Netlist : apachon_8knobs.net

A new file apachon_8knobs.net appears in the folder. That's the netlist.

## STEP 7 — Import into KiCad (PCB)

1. Open KiCad → File → New Project → name it Apachon_8Knobs.
2. Open the PCB Editor (Pcbnew).
3. File → Import → Netlist → select apachon_8knobs.net.
4. Click Update PCB.

All components (U1, IC1, RV1–RV8, C1, C2) appear in Pcbnew, ready for placement and routing.

## Common errors and fixes

| Error | Cause | Fix |
|---|---|---|
| 'SkidlLogFileHandler' object has no attribute 'filters' | SKiDL 2.3.0 bug on Python 3.13 | Install dev branch (Step 4, Option B) |
| KeyError: 'KICAD10_SYMBOL_DIR' | Env variable missing | Rerun Step 3, then open a NEW PowerShell window |
| [Errno 13] Permission denied: 'C:\skidl_REPL.log' | Running from C:\ root | cd to a normal folder (Step 5) |
| Unable to find part 74HC4067 in library Analog_Switch | Wrong library name | Use Part('74xx', 'CD74HC4067M') |
| No pins found using CD74HC4067M[('EN',)] | Pin name is ~{E}, not EN | Use ic1['~{E}'] in the script |
| No pins found using VCC[('~',)] | Power symbol pin naming | Omit power symbols — they are optional |
| Can't open file: Connector_PinHeader_2.54mm | That's a footprint lib, not a symbol lib | Use Part('Connector_Generic', 'Conn_02x12_Odd_Even') |
| Potentiometer_Alps_RK16812_Single_Vertical not found | Wrong footprint name | Use Potentiometer_Alps_RK163_Single_Horizontal |

## Useful diagnostic commands

List the pins of any KiCad symbol:

    import os
    os.environ['KICAD10_SYMBOL_DIR'] = r'C:\Program Files\KiCad\10.0\share\kicad\symbols'
    from skidl import *
    set_default_tool(KICAD10)
    p = Part('74xx', 'CD74HC4067M')
    for pin in p.pins:
        print(f'num={pin.num}  name="{pin.name}"')

List available Alps potentiometer footprints:

    Get-ChildItem "C:\Program Files\KiCad\10.0\share\kicad\footprints\Potentiometer_THT.pretty" -Filter "*Alps*" | Select-Object -ExpandProperty Name

## Key facts about the workflow

- SKiDL is not a KiCad replacement — it's a Python library that talks to KiCad's symbol and footprint libraries.
- KiCad must be installed first — SKiDL relies on its library files.
- All KICAD*_SYMBOL_DIR variables must be set — SKiDL 2.3.0 scans every tool version at startup, so missing ones cause warnings (and on Python 3.13, a crash).
- You only need to run the script once to generate the netlist. Re-run after every change to the circuit description.
- The netlist is portable — you can share apachon_8knobs.net with anyone who uses Altium, Eagle, EasyEDA, or Proteus.

Apachon CTRL-01-MK1 · SKiDL workflow · 2026