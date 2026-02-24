import pandas as pd

esp = pd.read_csv("../dataset/esp/dataset_esp.csv", header=None)
py = pd.read_csv("../dataset/python/dataset_python.csv", header=None)

merged = pd.concat([esp, py], ignore_index=True)
merged.to_csv("../dataset/merged/merge.csv", index=False, header=False)

print("Merged shape:", merged.shape)
