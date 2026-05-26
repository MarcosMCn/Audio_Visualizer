import serial
import numpy as np
import matplotlib.pyplot as plt
import time

PORT = "COM3"
BAUD = 921600

HEADER1 = 0xAB
HEADER2 = 0xCD

FFT_HALF_SIZE = 1024  # MUST match ESP32
PACKET_SIZE = 2 + FFT_HALF_SIZE

ser = serial.Serial(PORT, BAUD, timeout=0.01)

smooth = np.zeros(FFT_HALF_SIZE - 1)

# -----------------------------
# Read one full packet safely
# -----------------------------
def read_packet():
    while True:
        b = ser.read(1)

        if len(b) == 0:
            return None

        if b[0] == HEADER1:
            b2 = ser.read(1)

            if len(b2) == 0:
                return None

            if b2[0] == HEADER2:
                data = ser.read(FFT_HALF_SIZE)

                if len(data) == FFT_HALF_SIZE:
                    return np.frombuffer(data, dtype=np.int8)


# -----------------------------
# Plot setup
# -----------------------------
plt.ion()

fig, ax = plt.subplots()

# Skip bin 0 (DC / 0 Hz)
x = np.linspace(1, FFT_HALF_SIZE - 1, FFT_HALF_SIZE - 1)

x_log = np.log10(x)

line, = ax.plot(
    x,
    np.zeros(FFT_HALF_SIZE - 1),
    drawstyle='steps-mid'
)

ax.set_ylim(-120, 0)
# ax.set_xlim(np.log10(1), np.log10(FFT_HALF_SIZE))

ax.set_title("Real-time FFT (UART)")
ax.set_xlabel("Bin")
ax.set_ylabel("Magnitude")
# -----------------------------
# Main loop
# -----------------------------
while True:
    packet = read_packet()

    if packet is not None:

        raw = packet[1:].astype(np.float32)

        line.set_ydata(raw)

        plt.pause(0.001)