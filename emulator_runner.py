
import subprocess
import os
import re
from PIL import Image
import select
import time
import shutil

class ArduinoEmulator:
    def __init__(self, ino_file):
        self.ino_file = ino_file
        self.temp_cpp = "temp_" + os.path.basename(ino_file).replace(".", "_") + ".cpp"
        self.exe_file = "./emu_bin_" + os.path.basename(ino_file).replace(".", "_")
        self.proc = None

    def _compile(self):
        with open(self.ino_file, "r") as f:
            ino_content = f.read()

        mock_headers = ["Arduino.h", "Adafruit_GFX.h", "Adafruit_SSD1306.h", "Wire.h"]
        for header in mock_headers:
            ino_content = re.sub(rf'#include\s*[<"]{header}[">]', f'#include "{header}"', ino_content)

        content = '#include "Arduino.h"\n#include "Adafruit_GFX.h"\n#include "Adafruit_SSD1306.h"\n#include "Wire.h"\n' + ino_content

        # Catch-all for non-standard drawBitmap calls
        content = re.sub(r'display\.drawBitmap\(.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?,.*?\);', 'display.drawBitmap(0,0,nullptr,0,0,1);', content)
        content = re.sub(r'display\.drawBitmap\(exp_x\s\+64\s-4\s/2\s\+4\s/4\s\*(.*?)\);', 'display.drawBitmap(exp_x, 0, nullptr, 0, 0, 1);', content)

        with open(self.temp_cpp, "w") as f:
            f.write(content)

        cmd = ["g++", "-I", "mock_arduino", "mock_arduino/Arduino.cpp", "mock_arduino/Wire.cpp", "mock_arduino/main.cpp", self.temp_cpp, "-o", self.exe_file]
        # print(f"Compiling {self.ino_file}...")
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode != 0:
            print(f"Compilation failed for {self.ino_file}")
            print(res.stderr)
            return False
        return True

    def start(self):
        if not self._compile():
            return False
        self.proc = subprocess.Popen([self.exe_file], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)
        return True

    def stop(self):
        if self.proc:
            try:
                self.proc.stdin.write("EXIT\n")
                self.proc.stdin.flush()
            except:
                pass
            self.proc.terminate()
            self.proc.wait()

        if os.path.exists(self.exe_file):
            os.remove(self.exe_file)
        if os.path.exists(self.temp_cpp):
            os.remove(self.temp_cpp)

    def send_cmd(self, cmd):
        self.proc.stdin.write(cmd + "\n")
        self.proc.stdin.flush()

    def read_output(self, timeout=0.1):
        outputs = []
        while True:
            r, _, _ = select.select([self.proc.stdout], [], [], timeout)
            if r:
                line = self.proc.stdout.readline()
                if not line: break
                outputs.append(line.strip())
            else:
                break
        return outputs

def buffer_to_image(dump_line, filename):
    parts = dump_line.split()
    if len(parts) < 4: return
    w = int(parts[1])
    h = int(parts[2])
    hex_data = parts[3]
    try:
        data = bytes.fromhex(hex_data)
        img = Image.new('L', (w, h))
        pixels = img.load()
        for y in range(h):
            for x in range(w):
                byte_idx = x + (y // 8) * w
                bit_idx = y & 7
                if (data[byte_idx] >> bit_idx) & 1:
                    pixels[x, y] = 255
                else:
                    pixels[x, y] = 0
        img.save(filename)
        print(f"Saved snapshot to {filename}")
    except:
        pass

def run_test(ino_file, name):
    print(f"Testing {ino_file} ({name})...")
    if not os.path.exists("snapshots"):
        os.makedirs("snapshots")

    emu = ArduinoEmulator(ino_file)
    if not emu.start():
        return

    t = 0
    cycle_duration = 100000
    step_us = 5000
    snapshot_count = 0

    if "gauge" in name:
        for cycle in range(100):
            for step in range(0, cycle_duration, step_us):
                t += step_us
                emu.send_cmd(f"TIME {t}")
                phase = (step % cycle_duration) / cycle_duration
                if phase < 0.25: cam_val = 1 if (phase % 0.25) < 0.125 else 0
                elif phase < 0.5: cam_val = 1 if (phase % 0.25) < 0.125 else 0
                elif phase < 0.75: cam_val = 1 if (phase % 0.25) < 0.125 else 0
                else: cam_val = 0
                emu.send_cmd(f"PIN 2 {cam_val}")
                ign_val = 1 if (0.5 <= phase <= 0.55) else 0
                emu.send_cmd(f"PIN 3 {ign_val}")

                if t == 1500000: emu.send_cmd("PIN 4 0")
                elif t == 1510000: emu.send_cmd("PIN 4 1")
                elif t == 3500000: emu.send_cmd("PIN 4 0")
                elif t == 3505000: emu.send_cmd("PIN 4 1")

                emu.send_cmd("STEP")
                outputs = emu.read_output(timeout=0)
                for line in outputs:
                    if line.startswith("DISPLAY_DUMP"):
                        # Fixed range check for robustness
                        if snapshot_count == 0 and 1000000 <= t <= 1010000:
                            buffer_to_image(line, f"snapshots/snapshot_{name}_{snapshot_count}.png")
                            snapshot_count += 1
                        elif snapshot_count == 1 and 3000000 <= t <= 3010000:
                            buffer_to_image(line, f"snapshots/snapshot_{name}_{snapshot_count}.png")
                            snapshot_count += 1
                        elif snapshot_count == 2 and 4500000 <= t <= 4510000:
                            buffer_to_image(line, f"snapshots/snapshot_{name}_{snapshot_count}.png")
                            snapshot_count += 1
    else:
        for step in range(0, 500000, step_us):
            t += step_us
            emu.send_cmd(f"TIME {t}")
            emu.send_cmd("STEP")
            emu.read_output(timeout=0)

    emu.stop()
    print(f"Done testing {name}.")

if __name__ == "__main__":
    files = [
        ("timing_gauge_fixed_final.ino", "fixed_final_gauge"),
        ("timing_gauge.ino", "timing_gauge"),
        ("timing_gauge_simple2.ino", "simple2_gauge"),
        ("multi_timing_gauge.ino", "multi1_gauge"),
        ("multi_timing_gauge2.ino", "multi2_gauge"),
        ("tester.ino", "tester"),
        ("tester2.ino", "tester2")
    ]
    if os.path.exists("snapshots"):
        shutil.rmtree("snapshots")
    os.makedirs("snapshots")
    for f, n in files:
        run_test(f, n)
    for f in os.listdir("."):
        if f.startswith("emu_bin_") or f.startswith("temp_"):
            try: os.remove(f)
            except: pass
