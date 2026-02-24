import os
import numpy as np
import pandas as pd
import tensorflow as tf
import matplotlib.pyplot as plt

DATA_PATH = "../dataset/merged/merge.csv"
MODEL_PATH = "../deployment/model.h5"

if not os.path.exists(MODEL_PATH):
    raise FileNotFoundError("Model nie istnieje. Najpierw uruchom train.py")

df = pd.read_csv(DATA_PATH, header=None)

y_true = df.iloc[:, 0].values
noise_levels = df.iloc[:, 1].values
X = df.iloc[:, 2:].values.astype("float32")

assert X.shape[1] == 784, "Niepoprawna liczba pikseli!"

X = X.reshape(-1, 28, 28, 1)

print("Dataset shape:", X.shape)

model = tf.keras.models.load_model(MODEL_PATH)

y_pred_probs = model.predict(X, batch_size=32, verbose=0)
y_pred = np.argmax(y_pred_probs, axis=1)


global_acc = np.mean(y_pred == y_true)
print("Global accuracy:", global_acc)

unique_noise = np.sort(np.unique(noise_levels))
acc_per_noise = []

for n in unique_noise:
    idx = noise_levels == n
    acc = np.mean(y_pred[idx] == y_true[idx])
    acc_per_noise.append(acc)
    print(f"Noise {n}% -> accuracy {acc:.4f}")

plt.figure(figsize=(8,5))
plt.plot(unique_noise, acc_per_noise, marker="o")
plt.title("Accuracy vs Noise Level")
plt.xlabel("Noise (%)")
plt.ylabel("Accuracy")
plt.grid(True)
plt.tight_layout()
plt.savefig("accuracy_vs_noise.png", dpi=300)
plt.close()

print("Wykres zapisany jako accuracy_vs_noise.png")
