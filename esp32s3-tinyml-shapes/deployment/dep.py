import tensorflow as tf

interpreter = tf.lite.Interpreter(model_path="model_int8.tflite")
interpreter.allocate_tensors()

for op in interpreter._get_ops_details():
    print(op["op_name"])
