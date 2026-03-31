# ignition_timing_gauge
Visualize ignition timing (coming from ECU)

Visualizes ignition signal and its timing in relation to cam shaft sensor.
Useful to help determining set points for engine temperature, load, rpm and fuel type.

## Features
- Displays Dwell time and Ignition time in milliseconds.
- Graphical visualization of ignition timing (BTDC).
- Dynamic RPM calculation from sensor input.
- **Multiple Cam Wheel Support**:
    - 4-1 (missing tooth) wheels.
    - 4-equal (distributor style) wheels.
- Adjustable "expected timing" line using a potentiometer.
- Cycle through display modes with a push button (Short press).
- Toggle Wheel type with a long press on the button.

## Simulation and Testing
The project includes a robust mock Arduino environment and a Python-based physics hardware emulator.
The test system captures display snapshots during simulation to verify functionality across different wheel types and operating points.

### Captured Snapshots (Primary Gauge)
Snapshots from the simulation of `timing_gauge_fixed_final.ino` showing wheel type identification:

| Mode 0: 4-1 Wheel | Mode 0: 4-Equal Wheel |
| :---: | :---: |
| ![4-1 Wheel](snapshots/snapshot_fixed_final_gauge_0.png) | ![4-Equal Wheel](snapshots/snapshot_fixed_final_gauge_1.png) |

## How to run simulation
1. Install dependencies:
   ```bash
   pip install Pillow numpy
   ```
2. Run the emulator:
   ```bash
   python3 emulator_runner.py
   ```
3. Snapshots will be generated in the `snapshots/` directory.

## File Structure
- `timing_gauge_fixed_final.ino`: Main fully functional implementation with multi-wheel support.
- `multi_timing_gauge.ino` / `multi_timing_gauge2.ino`: Functional multi-mode versions.
- `tester2.ino`: Signal generator supporting 4-1 and 4-equal outputs for hardware testing.
- `emulator_runner.py`: Python script to run simulation and capture snapshots.
- `mock_arduino/`: C++ mock layer for Arduino and Adafruit libraries.
- `snapshots/`: Visual verification results.
