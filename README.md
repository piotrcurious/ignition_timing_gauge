# ignition_timing_gauge
Visualize ignition timing (coming from ECU)

Visualizes ignition signal and its timing in relation to cam shaft sensor.
Useful to help determining set points for engine temperature, load, rpm and fuel type.

## Features
- Displays Dwell time and Ignition time in milliseconds.
- Graphical visualization of ignition timing (BTDC).
- Support for 4-1 cam shaft sensor wheels.
- Adjustable "expected timing" line using a potentiometer.
- Cycle through display modes with a push button.

## Simulation and Testing
The project includes a mock Arduino environment and a Python-based physics hardware emulator.
The test system captures display snapshots during simulation to verify functionality.

### Captured Snapshots
Here are some snapshots from the simulation of `timing_gauge_fixed_final.ino`:

| Mode 0 (Dwell/Ign) | Mode 1 (Timing) |
| :---: | :---: |
| ![Mode 0](snapshots/snapshot_fixed_final_0.png) | ![Mode 1](snapshots/snapshot_fixed_final_1.png) |

## How to run simulation
1. Ensure you have `g++` and `Python 3` with `Pillow` installed.
2. Run `python3 emulator_runner.py`.
3. Snapshots will be saved in the `snapshots/` directory.

## File Structure
- `timing_gauge_fixed_final.ino`: The main, fully functional code.
- `emulator_runner.py`: Python script to run simulation and capture snapshots.
- `mock_arduino/`: C++ mock of the Arduino environment and Adafruit OLED libraries.
- `snapshots/`: Generated PNG snapshots from the simulation.
