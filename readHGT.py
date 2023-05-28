import os
import argparse
import math
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
from skimage.transform import resize

# Parse arguments
argparser = argparse.ArgumentParser()
argparser.add_argument('-f', '--file', help='HGT file to read')
args = argparser.parse_args()

# Read file and convert to numpy array
file = args.file + ".png"
# size = os.path.getsize(file)
# dim = int(math.sqrt(size/2))
# assert dim*dim*2 == size, 'Invalid file size'
# data = np.fromfile(file, np.dtype('>i2'), dim*dim).reshape((dim, dim))
# print("Image size: {}x{}\nBits per pixel: {}".format(dim, dim, 8*data.itemsize))

# # Read tif image from file with numpy
data = np.array(Image.open(file))

# Normalize data and downsample
data = data.astype(float)
data = (data-np.min(data))/(np.max(data)-np.min(data))*255
data = data.astype(np.uint8)
downsampled_data = resize(data, (100, 100))

# Create meshgrid
xx, yy = np.mgrid[0:downsampled_data.shape[0], 0:downsampled_data.shape[1]]

# Plot
fig = plt.figure(figsize=(10, 5))

# First subplot: 3D plot
ax1 = fig.add_subplot(121, projection='3d')
im = ax1.plot_surface(xx, yy, downsampled_data, rstride=1, cstride=1, cmap=plt.cm.magma, linewidth=2)
ax1.set_title('3D Heightmap')

# Second subplot: 2D image
ax2 = fig.add_subplot(122)
im = ax2.imshow(data, cmap='viridis')
ax2.set_title('2D Image')
plt.colorbar(im, ax=ax2)

# Add horizontal spacing between the subplots
plt.subplots_adjust(wspace=0.2)

plt.show()