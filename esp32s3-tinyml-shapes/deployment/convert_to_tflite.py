import numpy as np
import tensorflow as tf
import pandas as pd

MODEL_PATH = "model.h5"
DATA_PATH = "../dataset/merged/merge.csv"
OUTPUT_PATH = "model_int8.tflite"

model = tf.keras.models.load_model(MODEL_PATH)

df = pd.read_csv(DATA_PATH, header=None)
X = df.iloc[:, 2:].values.astype("float32")
X = X.reshape(-1, 28, 28, 1)

calibration_data = X[:200]

def representative_dataset():
    for i in range(len(calibration_data)):
        yield [np.expand_dims(calibration_data[i], axis=0)]

converter = tf.lite.TFLiteConverter.from_keras_model(model)

converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset

converter.target_spec.supported_ops = [
        tf.lite.OpsSet.TFLITE_BUILTINS_INT8
        ]

converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8

tflite_model = converter.convert()

with open(OUTPUT_PATH, "wb") as f:
    f.write(tflite_model)

print("Model zapisany jako:", OUTPUT_PATH)
print("Rozmiar:", len(tflite_model) / 1024, "KB")
