import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3' 
import tensorflow as tf
import tensorflow.keras.backend as K
from tensorflow.keras.layers import (Concatenate, Conv2D, Input, Layer, MaxPooling2D, ReLU, UpSampling2D, ZeroPadding2D)
from tensorflow.keras.models import Model
import numpy as np
from PIL import Image
from typing import Dict, Tuple


class TerrainGANBuilder:
    def __init__(self):
        self._map_shape = (450, 450)

    def load_model(self, gen_path):
        gen_image_shape = (*self._map_shape, 4) #(450, 450, 4)
        gen_inputs, downsampling_outs = self._get_scale_down(gen_image_shape)

        upsampling_out = self._get_scale_up(downsampling_outs, 1)
        generator = Model([gen_inputs['image_input'], gen_inputs['noise']], upsampling_out)
        
        generator.load_weights(gen_path)
        return generator
    
    def _add_conv_layer(self, layer: Layer, x) -> Layer:
        out = layer(x)
        return ReLU()(out)

    def _get_scale_down_block(self, previous_output, filter_out: int):
        conv = self._add_conv_layer(Conv2D(filter_out, 3, padding='same'), previous_output)
        conv = self._add_conv_layer(Conv2D(filter_out, 3, padding='same'), conv)
        return conv, MaxPooling2D(pool_size=(2, 2))(conv)

    def _get_scale_up_block(self, previous_output, combine_with, filter_out: int):
        upsampled = UpSampling2D(size=(2, 2))(previous_output)
        # Reshape "upsampled" to match previous_output dimensions
        if filter_out==128:
            desired_size = (225, 225)
            upsampled = tf.image.resize(upsampled, size=desired_size, method=tf.image.ResizeMethod.NEAREST_NEIGHBOR)

        up = self._add_conv_layer(Conv2D(filter_out, 2, padding='same'), upsampled)
        merge = Concatenate()([combine_with, up])
        conv = Conv2D(filter_out, 3, padding='same')(merge)
        return Conv2D(filter_out, 3, padding='same')(conv)

    def _get_scale_down(self, inputs: Tuple[int, ...]) -> Tuple[Dict[str, Layer], ...]:
        inputs = Input(shape=inputs)
        conv1, pool1 = self._get_scale_down_block(inputs, 64)
        conv2, pool2 = self._get_scale_down_block(pool1, 128)
        conv3, pool3 = self._get_scale_down_block(pool2, 256)
        conv4, pool4 = self._get_scale_down_block(pool3, 512)
        conv5 = self._add_conv_layer(Conv2D(1024, 3, padding='same'), pool4)
        conv5 = self._add_conv_layer(Conv2D(1024, 3, padding='same'), conv5)
        noise = Input((K.int_shape(conv5)[1], K.int_shape(conv5)[2], K.int_shape(conv5)[3]))
        conv5 = Concatenate()([conv5, noise])

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

        upsampled = UpSampling2D(size=(2, 2))(downsampling_outs['conv5'])
        up = self._add_conv_layer(Conv2D(512, 2, padding='same'), upsampled)
        merge = Concatenate()([downsampling_outs['conv4'], up])
        conv = Conv2D(512, 3, padding='same')(merge)
        block1 = Conv2D(512, 3, padding='same')(conv)

        upsampled = UpSampling2D(size=(2, 2))(block1)
        up = self._add_conv_layer(Conv2D(256, 2, padding='same'), upsampled)
        merge = Concatenate()([downsampling_outs['conv3'], up])
        conv = Conv2D(256, 3, padding='same')(merge)
        block2 = Conv2D(256, 3, padding='same')(conv)

        upsampled = UpSampling2D(size=(2, 2))(block2)
        up = self._add_conv_layer(Conv2D(128, 2, padding='same'), upsampled)
        up_padded = ZeroPadding2D(padding=((1, 0), (1, 0)))(up)
        merge = Concatenate()([downsampling_outs['conv2'], up_padded])
        conv = Conv2D(128, 3, padding='same')(merge)
        block3 = Conv2D(128, 3, padding='same')(conv)

        up9 = Conv2D(64, 2, padding='same')(UpSampling2D(size=(2, 2))(block3))
        merge9 = Concatenate()([downsampling_outs['conv1'], up9])
        conv9 = Conv2D(64, 3, padding='same')(merge9)
        conv9 = Conv2D(64, 3, padding='same')(conv9)
        conv9 = Conv2D(32, 3, padding='same')(conv9)
        conv10 = Conv2D(out_channels, 1, activation='tanh')(conv9)
        reshaped_output = tf.keras.layers.Reshape((450, 450))(conv10)
        return reshaped_output



def predict():

    print("Predicting...")

     # Load model
    builder = TerrainGANBuilder()
    generator = builder.load_model("./model/model.h5")
    
    # Load input images
    channels = [np.array(Image.open("./data/" + str(i) + ".png").convert("L")) for i in range(1,5)]
    channels = np.stack(channels, axis=-1)
    input_image = np.expand_dims(channels, axis=0)
    input_image = input_image / 255.0
    
    # Load noise
    noise = np.random.normal(0, 1, (1, 28, 28, 1024))

    # Predict
    output = generator([input_image, noise])
    
    # Save
    output = np.squeeze(np.uint8(output * 127.5 + 127.5), axis=0)
    image = Image.fromarray(output, mode='L')
    image.save('./data/heightmap.png')
    with open('./data/heightmap.png', 'rb') as file:
        file.flush()

    

if __name__ == "__main__":
    predict()