import numpy as np
import csv
import random

IMG_SIZE = 28
NUM_SAMPLES = 10000
OUTPUT = "../dataset/python/dataset_python.csv"

NOISE_MIN = 5
NOISE_MAX = 20


def clear():
    return np.zeros((IMG_SIZE, IMG_SIZE), dtype=np.uint8)

def generate_square():
    img = clear()
    size = random.randint(6, 13)

    x0 = random.randint(0, IMG_SIZE - size)
    y0 = random.randint(0, IMG_SIZE - size)

    img[y0:y0+size, x0:x0+size] = 1
    return img

def generate_circle():
    img = clear()
    r = random.randint(6, 11)

    cx = random.randint(r, IMG_SIZE - r)
    cy = random.randint(r, IMG_SIZE - r)

    y, x = np.ogrid[:IMG_SIZE, :IMG_SIZE]
    mask = (x - cx)**2 + (y - cy)**2 <= r*r
    img[mask] = 1

    return img

def generate_triangle():
    img = clear()
    height = random.randint(8, 18)
    base_width = height

    x0 = random.randint(0, IMG_SIZE - base_width)
    y0 = random.randint(0, IMG_SIZE - height)

    for y in range(height):
        current_width = int((y / height) * base_width)
        offset = (base_width - current_width) // 2

        for x in range(current_width):
            px = x0 + offset + x
            py = y0 + y
            img[py, px] = 1

    return img

def apply_noise(img, noise_percent):
    noisy = img.copy()
    mask = np.random.rand(IMG_SIZE, IMG_SIZE) < (noise_percent / 100.0)
    noisy[mask] = 1 - noisy[mask]
    return noisy

with open(OUTPUT, "w", newline="") as f:
    writer = csv.writer(f)

    # balans klas
    samples_per_class = NUM_SAMPLES // 3

    for class_id in range(3):
        for _ in range(samples_per_class):

            if class_id == 0:
                img = generate_triangle()
            elif class_id == 1:
                img = generate_square()
            else:
                img = generate_circle()

            noise = random.randint(NOISE_MIN, NOISE_MAX)
            img = apply_noise(img, noise)

            row = [class_id, noise] + img.flatten().tolist()
            writer.writerow(row)

print("Dataset wygenerowany:", OUTPUT)
