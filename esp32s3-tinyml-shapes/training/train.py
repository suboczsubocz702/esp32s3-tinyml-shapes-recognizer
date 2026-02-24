import os
import numpy as np
import pandas as pd
import tensorflow as tf
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split

DATA_PATH = "../dataset/merged/merge.csv"

df = pd.read_csv(DATA_PATH, header=None)

y = df.iloc[:, 0].values
noise = df.iloc[:, 1].values  # zachowujemy do ewentualnej analizy
X = df.iloc[:, 2:].values.astype("float32")

assert X.shape[1] == 784, "Niepoprawna liczba pikseli!"

print("Dataset shape:", X.shape)

X_temp, X_test, y_temp, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42
        )

X_train, X_val, y_train, y_val = train_test_split(
        X_temp, y_temp, test_size=0.2, random_state=42
        )

print("Train:", len(X_train))
print("Validation:", len(X_val))
print("Test:", len(X_test))

X_train = X_train.reshape(-1, 28, 28, 1)
X_val   = X_val.reshape(-1, 28, 28, 1)
X_test  = X_test.reshape(-1, 28, 28, 1)

model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(28, 28, 1)),

    tf.keras.layers.Conv2D(8, 3, activation="relu"),
    tf.keras.layers.MaxPooling2D(2),

    tf.keras.layers.Conv2D(16, 3, activation="relu"),
    tf.keras.layers.MaxPooling2D(2),

    tf.keras.layers.GlobalAveragePooling2D(),

    tf.keras.layers.Dense(16, activation="relu"),
    tf.keras.layers.Dense(3, activation="softmax")
    ])

model.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"]
        )

model.summary()

early_stop = tf.keras.callbacks.EarlyStopping(
        monitor="val_loss",
        patience=5,
        restore_best_weights=True
        )

history = model.fit(
        X_train, y_train,
        epochs=300,
        batch_size=32,
        validation_data=(X_val, y_val),
        callbacks=[early_stop],
        verbose=1
        )

loss, acc = model.evaluate(X_test, y_test)
print("Test accuracy:", acc)


os.makedirs("../deployment", exist_ok=True)
model.save("../deployment/model.h5")
print("Model zapisany w deployment/model.h5")

# =========================
# Wykres Accuracy
# =========================
plt.figure(figsize=(8,5))
plt.plot(history.history["accuracy"], label="Train")
plt.plot(history.history["val_accuracy"], label="Validation")
plt.title("Model Accuracy")
plt.xlabel("Epoch")
plt.ylabel("Accuracy")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("accuracy.png", dpi=300)
plt.close()

# =========================
# Wykres Loss
# =========================
plt.figure(figsize=(8,5))
plt.plot(history.history["loss"], label="Train")
plt.plot(history.history["val_loss"], label="Validation")
plt.title("Model Loss")
plt.xlabel("Epoch")
plt.ylabel("Loss")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("loss.png", dpi=300)
plt.close()

print("Wykresy zapisane.")
