import serial
import csv
import time

PORT = "/dev/ttyACM0"
BAUDRATE = 115200
OUTPUT_FILE = "../dataset/esp/dataset_esp.csv"
NUM_SAMPLES = 10000

print("Opening serial port...")
ser = serial.Serial(PORT, BAUDRATE, timeout=2)

print("Waiting for ESP to start...")
time.sleep(3)  # daj ESP czas po resecie

samples_collected = 0

with open(OUTPUT_FILE, "w", newline="") as f:
    writer = csv.writer(f)

print("Collecting data...")

    while samples_collected < NUM_SAMPLES:
        line = ser.readline().decode(errors="ignore").strip()

        # DEBUG
        if line:
            print("RECEIVED:", line[:50])

        # odrzucamy linie bootloadera
        if not line or "," not in line:
            continue

        parts = line.split(",")

        if len(parts) != 786:
            continue

        writer.writerow(parts)
        samples_collected += 1

        print(f"{samples_collected}/{NUM_SAMPLES} samples collected")

print("Done.")
ser.close()
