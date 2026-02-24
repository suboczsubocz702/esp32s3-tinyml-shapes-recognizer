import numpy as np
import pandas as pd
import tensorflow as tf
import matplotlib.pyplot as plt
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(BASE_DIR, "dataset/merged/merge.csv")
MODEL_FLOAT_PATH = os.path.join(BASE_DIR, "deployment/model.h5")
MODEL_INT8_PATH = os.path.join(BASE_DIR, "deployment/model_int8.tflite")

df = pd.read_csv(DATA_PATH, header=None)

y_true = df.iloc[:, 0].values
noise_levels = df.iloc[:, 1].values
X = df.iloc[:, 2:].values.astype("float32")

assert X.shape[1] == 784, "Niepoprawna liczba pikseli!"

X = X.reshape(-1, 28, 28, 1)

print("Dataset shape:", X.shape)

float_model = tf.keras.models.load_model(MODEL_FLOAT_PATH)

y_pred_float = np.argmax(
    float_model.predict(X, batch_size=32, verbose=0),
    axis=1
)

float_acc = np.mean(y_pred_float == y_true)
print("FLOAT accuracy:", float_acc)

interpreter = tf.lite.Interpreter(model_path=MODEL_INT8_PATH)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

input_scale, input_zero = input_details[0]["quantization"]
output_scale, output_zero = output_details[0]["quantization"]

y_pred_int8 = []

for i in range(len(X)):
    x = X[i:i+1]

    # Kwantyzacja wejścia
    x_int8 = (x / input_scale + input_zero).astype(np.int8)

    interpreter.set_tensor(input_details[0]["index"], x_int8)
    interpreter.invoke()

    output = interpreter.get_tensor(output_details[0]["index"])

    # Dekwantyzacja wyjścia
    output_float = (output.astype(np.float32) - output_zero) * output_scale

    y_pred_int8.append(np.argmax(output_float))

y_pred_int8 = np.array(y_pred_int8)

int8_acc = np.mean(y_pred_int8 == y_true)
print("INT8 accuracy:", int8_acc)

print("Accuracy drop:", float_acc - int8_acc)

unique_noise = np.sort(np.unique(noise_levels))

acc_float_noise = []
acc_int8_noise = []

for n in unique_noise:
    idx = noise_levels == n
    acc_f = np.mean(y_pred_float[idx] == y_true[idx])
    acc_i = np.mean(y_pred_int8[idx] == y_true[idx])

    acc_float_noise.append(acc_f)
    acc_int8_noise.append(acc_i)

    print(f"Noise {n}% -> FLOAT {acc_f:.4f} | INT8 {acc_i:.4f}")

plt.figure(figsize=(6,4))
plt.bar(["FLOAT32", "INT8"], [float_acc, int8_acc])
plt.title("Global Accuracy Comparison")
plt.ylabel("Accuracy")
plt.ylim(0,1)
plt.grid(axis="y")
plt.tight_layout()
plt.savefig("global_accuracy_comparison.png", dpi=300)
plt.close()

plt.figure(figsize=(8,5))

plt.plot(unique_noise, acc_float_noise, marker="o", label="FLOAT32")
plt.plot(unique_noise, acc_int8_noise, marker="s", label="INT8")

plt.title("Accuracy vs Noise Level")
plt.xlabel("Noise (%)")
plt.ylabel("Accuracy")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("accuracy_vs_noise_float_vs_int8.png", dpi=300)
plt.close()

print("Wykresy zapisane (300 DPI).")
