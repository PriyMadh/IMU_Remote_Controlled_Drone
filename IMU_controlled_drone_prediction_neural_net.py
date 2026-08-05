import os
import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Dropout
from tensorflow.keras import regularizers
from tensorflow.keras.activations import linear, relu


csv_path = "gesture_data_take_3_combined.csv"


if not os.path.exists(csv_path):
    raise FileNotFoundError(
        f"Could not find the file at {csv_path}."
    )

print(f"Success! Found and loading file from Finder: {csv_path}")

# 2. Load data into a Pandas DataFrame
df = pd.read_csv(csv_path)


df = df.dropna()

# 3. THE NUMPY CONVERSION:

# Get the target column 
y_train = df["Label"].to_numpy()

# Drop target column 
X_train = df.drop(columns=["Label"]).to_numpy()

tf.random.set_seed(1234) #less randomness
model = Sequential(
    [
        
        Dense(units = 12, activation = 'relu', kernel_regularizer=regularizers.l2(0.01)),
        Dropout(0.1),
        Dense(units = 7, activation = 'linear'),
    ]
)
model.compile(
    loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True), #less rounding error by using linear activation in output layer then using this loss
    optimizer = tf.keras.optimizers.SGD(learning_rate=5e-2), #use gradient descent function (tried ADAM before but didn't work so well. kept overshooting)
)

model.fit(X_train, y_train, epochs = 100)
    
model.save("IMU_controlled_drone_prediction_neural_net.keras")
