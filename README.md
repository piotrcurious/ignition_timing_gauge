# ignition_timing_gauge
Visualize ignition timing (coming from ECU)

Visualizes ignition signal and its timing in relation to cam shaft sensor.
Useful to help determining set points for engine temperature, load, rpm and fuel type.

## Features
- Displays Dwell time and Ignition time in milliseconds.
- Graphical visualization of ignition timing (BTDC).
- Dynamic RPM calculation from sensor input.
- Support for 4-1 cam shaft sensor wheels.
- Adjustable "expected timing" line using a potentiometer.
- Cycle through display modes with a push button.

## Simulation and Testing
The project includes a robust mock Arduino environment and a Python-based physics hardware emulator.
The test system captures display snapshots during simulation to verify functionality across different operating points.

### Captured Snapshots (Final Version)
Here are snapshots from the simulation of `timing_gauge_fixed_final.ino`:

| Mode 0 (Dwell/Ign) | Mode 1 (Timing) | Mode 2 (Timing + Target) |
| :---: | :---: | :---: |
| ![Mode 0](snapshots/snapshot_fixed_final_gauge_0.png) | ![Mode 1](snapshots/snapshot_fixed_final_gauge_1.png) | ![Mode 2](snapshots/snapshot_fixed_final_gauge_2.png) |

## How to run simulation
1. Install dependencies:
   ```bash
   pip install Pillow numpy
   ```
2. Run the emulator:
   ```bash
   python3 emulator_runner.py
   ```
3. Snapshots will be generated in the `snapshots/` directory for all gauge files.

## File Structure
- `timing_gauge_fixed_final.ino`: Primary, fully functional implementation with dynamic RPM and refined timing logic.
- `multi_timing_gauge.ino` / `multi_timing_gauge2.ino`: Alternative multi-mode versions, fully functional.
- `timing_gauge.ino` / `timing_gauge_simple2.ino`: Legacy and simple versions, functional.
- `tester.ino` / `tester2.ino`: Signal generators for testing hardware gauges (tester2 simulates 4-1 wheel).
- `emulator_runner.py`: Python script to run simulation and capture snapshots.
- `mock_arduino/`: C++ mock layer for Arduino and Adafruit libraries.
- `snapshots/`: Visual verification results.
