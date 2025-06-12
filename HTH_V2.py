import serial
import numpy as np
import matplotlib
matplotlib.use('TkAgg')  # Use 'MacOSX' if 'TkAgg' doesn't work on your Mac
import matplotlib.pyplot as plt
import time


# === Configuration ===
SERIAL_PORT = '/dev/tty.usbmodem3C8427C482882'     # Change this to your serial port (e.g., '/dev/ttyUSB0' on Linux)
BAUD_RATE = 115200
TIMEOUT = 1              # seconds

# === Initialize Serial ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)
time.sleep(2)  # Give some time for the serial connection to initialize

# === Initialize Plot ===
plt.ion()
fig, ax = plt.subplots()
heatmap = ax.imshow(np.zeros((24, 32)), vmin=20, vmax=75, cmap='coolwarm')
cbar = fig.colorbar(heatmap)
ax.set_title("MLX90640 Thermal Camera")
ax.set_xlabel("Pixel X")
ax.set_ylabel("Pixel Y")

# === Read & Plot Loop ===
try:
    while True:
        line = ser.readline().decode(errors='ignore').strip()
        if line == 'FrameStart':
            data = []
            # Read 768 temperature values
            while len(data) < 32 * 24:
                l = ser.readline().decode(errors='ignore').strip()
                if not l:
                    continue
                for p in l.split(','):
                    try:
                        data.append(float(p))
                    except ValueError:
                        pass
            # Update the heatmap
            frame = np.array(data).reshape((24, 32))
            heatmap.set_data(frame)
            #heatmap.set_clim(frame.min(), frame.max())
            fig.canvas.draw()
            fig.canvas.flush_events()
            plt.pause(0.001)
except KeyboardInterrupt:
    print("Visualization stopped by user.")
    ser.close()