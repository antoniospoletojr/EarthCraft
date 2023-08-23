## @package Predict
#  Predict script using the trained CNN GAN model to generate a heightmap from the input sketches.

import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
import tensorflow as tf
import tensorflow.keras.backend as K
from tensorflow.keras.layers import (Concatenate, Conv2D, Input, Layer, MaxPooling2D, ReLU, UpSampling2D, ZeroPadding2D)
from tensorflow.keras.models import Model
import numpy as np
from PIL import Image
from typing import Dict, Tuple

## Class to build the CNN GAN model.
#  The model is built using the Keras functional API.
class TerrainGANBuilder:
    def __init__(self):
        self._map_shape = (450, 450)  # Initialize the map dimensions

    def load_model(self, gen_path):
        gen_image_shape = (*self._map_shape, 4)  # Define the shape of the generated image (450, 450, 4)
        gen_inputs, downsampling_outs = self._get_scale_down(gen_image_shape)
        
        upsampling_out = self._get_scale_up(downsampling_outs, 1)  # Perform upsampling
        generator = Model([gen_inputs['image_input'], gen_inputs['noise']], upsampling_out)  # Create the generator model
        
        generator.load_weights(gen_path)  # Load pre-trained weights
        return generator
    
    def _add_conv_layer(self, layer: Layer, x) -> Layer:
        out = layer(x)  # Apply the convolutional layer
        return ReLU()(out)  # Apply ReLU activation function

    def _get_scale_down_block(self, previous_output, filter_out: int):
        conv = self._add_conv_layer(Conv2D(filter_out, 3, padding='same'), previous_output)  # Apply convolution
        conv = self._add_conv_layer(Conv2D(filter_out, 3, padding='same'), conv)  # Apply another convolution
        return conv, MaxPooling2D(pool_size=(2, 2))(conv)  # Apply max pooling

    def _get_scale_up_block(self, previous_output, combine_with, filter_out: int):
        upsampled = UpSampling2D(size=(2, 2))(previous_output)  # Perform upsampling
        # Reshape "upsampled" to match previous_output dimensions if needed
        if filter_out == 128:
            desired_size = (225, 225)
            upsampled = tf.image.resize(upsampled, size=desired_size, method=tf.image.ResizeMethod.NEAREST_NEIGHBOR)

        up = self._add_conv_layer(Conv2D(filter_out, 2, padding='same'), upsampled)  # Apply convolution
        merge = Concatenate()([combine_with, up])  # Concatenate feature maps
        conv = Conv2D(filter_out, 3, padding='same')(merge)  # Apply convolution
        return Conv2D(filter_out, 3, padding='same')(conv)  # Apply another convolution
    
    def _get_scale_down(self, inputs: Tuple[int, ...]) -> Tuple[Dict[str, Layer], ...]:
        inputs = Input(shape=inputs)                                                            # Create an input layer
        conv1, pool1 = self._get_scale_down_block(inputs, 64)                                   # Create a downscaling block
        conv2, pool2 = self._get_scale_down_block(pool1, 128)                                   # Create another downscaling block
        conv3, pool3 = self._get_scale_down_block(pool2, 256)                                   # ...
        conv4, pool4 = self._get_scale_down_block(pool3, 512)
        conv5 = self._add_conv_layer(Conv2D(1024, 3, padding='same'), pool4)                    # Apply convolution
        conv5 = self._add_conv_layer(Conv2D(1024, 3, padding='same'), conv5)                    # Apply another convolution
        noise = Input((K.int_shape(conv5)[1], K.int_shape(conv5)[2], K.int_shape(conv5)[3]))    # Create noise input
        conv5 = Concatenate()([conv5, noise])                                                   # Concatenate with noise input

        conv_out = {
            'conv1': conv1,
            'conv2': conv2,
            'conv3': conv3,
            'conv4': conv4,
            'conv5': conv5
        }
        
        ins = {
            'image_input': inputs,
            'noise': noise
        }

        return ins, conv_out

    def _get_scale_up(self, downsampling_outs: Dict[str, Layer], out_channels: int):
        
        upsampled = UpSampling2D(size=(2, 2))(downsampling_outs['conv5'])       # Perform upsampling
        up = self._add_conv_layer(Conv2D(512, 2, padding='same'), upsampled)    # Apply convolution
        merge = Concatenate()([downsampling_outs['conv4'], up])                 # Concatenate feature maps
        conv = Conv2D(512, 3, padding='same')(merge)                            # Apply convolution
        block1 = Conv2D(512, 3, padding='same')(conv)                           # Apply another convolution

        upsampled = UpSampling2D(size=(2, 2))(block1)                           # Perform upsampling
        up = self._add_conv_layer(Conv2D(256, 2, padding='same'), upsampled)    # Apply convolution
        merge = Concatenate()([downsampling_outs['conv3'], up])                 # Concatenate feature maps
        conv = Conv2D(256, 3, padding='same')(merge)                            # Apply convolution
        block2 = Conv2D(256, 3, padding='same')(conv)                           # Apply another convolution
        
        upsampled = UpSampling2D(size=(2, 2))(block2)                           # Perform upsampling
        up = self._add_conv_layer(Conv2D(128, 2, padding='same'), upsampled)    # Apply convolution
        up_padded = ZeroPadding2D(padding=((1, 0), (1, 0)))(up)                 # Apply zero padding
        merge = Concatenate()([downsampling_outs['conv2'], up_padded])          # Concatenate feature maps
        conv = Conv2D(128, 3, padding='same')(merge)                            # Apply convolution
        block3 = Conv2D(128, 3, padding='same')(conv)                           # Apply another convolution
        
        up9 = Conv2D(64, 2, padding='same')(UpSampling2D(size=(2, 2))(block3))  # Perform upsampling
        merge9 = Concatenate()([downsampling_outs['conv1'], up9])               # Concatenate feature maps
        conv9 = Conv2D(64, 3, padding='same')(merge9)                           # Apply convolution
        conv9 = Conv2D(64, 3, padding='same')(conv9)                            # Apply another convolution
        conv9 = Conv2D(32, 3, padding='same')(conv9)                            # Apply another convolution
        conv10 = Conv2D(out_channels, 1, activation='tanh')(conv9)              # Apply final convolution
        reshaped_output = tf.keras.layers.Reshape((450, 450))(conv10)           # Reshape the output
        return reshaped_output

## Function to predict the heightmap from the input sketches.
#  The function loads the trained model and generates the heightmap. It is called from the c++ code.
def predict():
    
    print("Predicting...")
     
    # Load model
    builder = TerrainGANBuilder()
    generator = builder.load_model("./neural_network/model.h5")
    
    # File names
    file_names = ["ridges.png", "rivers.png", "peaks.png", "basins.png"]
    
    # Load input images
    channels = [np.array(Image.open("./assets/sketches/" + file_names[i]).convert("L")) for i in range(4)]
    channels = np.stack(channels, axis=-1)
    input_image = np.expand_dims(channels, axis=0)
    input_image = input_image / 255.0
    
    # Load noise
    noise = np.random.normal(0, 1, (1, 28, 28, 1024))
    
    # Predict
    output = generator([input_image, noise])
    
    # Save
    output = np.squeeze(np.uint8(output * 127.5 + 127.5), axis=0)
    
    # make the borders (5 px wide) black
    output[0:5, :] = 0
    output[:, 0:5] = 0
    output[445:450, :] = 0
    output[:, 445:450] = 0
    
    image = Image.fromarray(output, mode='L')
    
    # Save image
    with open("./assets/sketches/heightmap.png", "wb") as file:
        image.save(file, "PNG")
        file.flush()
        os.fsync(file.fileno())
    

if __name__ == "__main__":
    predict()