# 1. Cel projektu
Celem projektu jest zaprojektowanie i wdrożenie lekkiego modelu klasyfikacyjnego CNN rozpoznającego proste figury geometryczne (kwadrat, koło, trójkąt) na mikrokontrolerze klasy embedded.

Dodatkowe cele:

- analiza odporności modelu na szum losowy (5–20%),
- ocena wpływu kwantyzacji INT8 na dokładność,
- pomiar zużycia zasobów (RAM, Flash, CPU).

# 2. Dataset
## 2.1 Struktura rekordu

Każda próbka ma postać:
[class_id, noise_level, pixel_0 ... pixel_783]
Rozmiar obrazu: 28×28
Format: binarny (0/1)
Liczba pikseli: 784

## 2.2 Generowanie danych

Python
- losowa pozycja figury,
- losowy rozmiar,
- dodanie szumu procentowego.

ESP
- generacja bezpośrednio na mikrokontrolerze,
- przesyłanie przez UART,
- zapis do CSV.

## 2.3 Balans klas
- 3 klasy
- Umiarkowanie równy rozkład
- 15 000 próbek łącznie

# 3. Architektura modelu
## 3.1 Struktura CNN
- Input (28x28x1)
- Conv2D(8, 3x3) + ReLU
- MaxPooling(2x2)
- Conv2D(16, 3x3) + ReLU
- MaxPooling(2x2)
- GlobalAveragePooling
- Dense(16) + ReLU
- Dense(3) + Softmax
  
## 3.2 Parametry

- Funkcja kosztu: sparse_categorical_crossentropy
- Optymalizator: Adam (lr=0.001)
- Batch size: 32
- EarlyStopping (patience=5)

# 4. Kwantyzacja INT8
## 4.1 Motywacja
Model float32 jest:
- większy,
- wolniejszy,
- bardziej pamięciochłonny.
- INT8 pozwala:
- zmniejszyć rozmiar modelu,
- przyspieszyć inferencję,
- obniżyć zużycie RAM.

## 4.2 Procedura

- Wczytanie model.h5
- Ustawienie optimizations = DEFAULT
- Zdefiniowanie representative_dataset
- Konwersja do model_int8.tflite

## 4.3 Analiza wpływu
Porównano:
- global accuracy (float vs int8)
- accuracy vs noise
Interpretacja:
- Loss jest mniejszy niż 0.0001%, więc kalibracja jest akceptowalna

# 5. Integracja z firmware
## 5.1 Środowisko

ESP-IDF

TensorFlow Lite Micro

## 5.2 Model

Model przekonwertowany do:

model_int8.tflite

Następnie do: 

xxd -i model_int8.tflite > model.cc

i włączony do firmware

# 6. Wnioski

- Model poprawnie klasyfikuje figury.
- Odporność na szum maleje wraz ze wzrostem zakłóceń.
- INT8 powoduje niewielki spadek dokładności.
- Model mieści się w ograniczeniach ESP32-S3.

# 7. Numery pinów
MOSI 11

CLK  12

CS   10

DC    6

RST   7

BCKL  3

# 8. Biblioteka tfmicro dla esp32s3

git clone https://github.com/espressif/esp-tflite-micro.git

# 9. Wykresy

## Accuracy vs noise plot

<img width="2400" height="1500" alt="accuracy_vs_noise" src="https://github.com/user-attachments/assets/725544ca-2f98-4f23-bb9a-a4c614e4e51c" />

## Loss plot

<img width="2400" height="1500" alt="loss" src="https://github.com/user-attachments/assets/eb37d56e-e134-4d75-9c54-5950d6434806" />

## Accuracy plot

<img width="2400" height="1500" alt="accuracy" src="https://github.com/user-attachments/assets/fafe5247-c5ee-4d59-be29-102ece2a9fb6" />

## int8 comparission plots 

<img width="2400" height="1500" alt="accuracy_vs_noise_float_vs_int8" src="https://github.com/user-attachments/assets/65ff828b-7639-41b2-9880-59728ab538a5" />
<img width="1800" height="1200" alt="global_accuracy_comparison" src="https://github.com/user-attachments/assets/84ebd411-69c5-41f2-8f21-29fc1f23e070" />
