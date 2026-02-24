import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix
import seaborn as sns
import tensorflow as tf

DATA_PATH = "../dataset/merged/merge.csv"
MODEL_PATH = "../deployment/model.h5"

df = pd.read_csv(DATA_PATH, header=None)

X = df.iloc[:, 1:].values.astype("float32")
y = df.iloc[:, 0].values

plt.figure(figsize=(8,6))
for i in range(9):
    plt.subplot(3,3,i+1)
    plt.imshow(X[i].reshape(28,28), cmap="gray")
    plt.title(f"Class {y[i]}")
    plt.axis("off")
plt.tight_layout()
plt.show()

unique, counts = np.unique(y, return_counts=True)

plt.figure()
plt.bar(unique, counts)
plt.xlabel("Class")
plt.ylabel("Samples")
plt.title("Class Distribution")
plt.show()

model = tf.keras.models.load_model(MODEL_PATH)

y_pred = model.predict(X)
y_pred_classes = np.argmax(y_pred, axis=1)

cm = confusion_matrix(y, y_pred_classes)

plt.figure()
sns.heatmap(cm, annot=True, fmt="d", cmap="Blues")
plt.xlabel("Predicted")
plt.ylabel("True")
plt.title("Confusion Matrix")
plt.show()
