import os
from pathlib import Path
import numpy as np
from PIL import Image
import math
from multiprocessing import Pool


def process_file(file):
    print("processing file: ", file)

    # Read the file
    size = os.path.getsize(file)
    dim = int(math.sqrt(size/2))
    data = np.fromfile(file, np.dtype('>i2'), dim*dim).reshape((dim, dim))
    data = data[:3600, :3600]
    
    # Normalize data
    data = data.astype(float)
    data = (data-np.min(data))/(np.max(data)-np.min(data))*255
    data = data.astype(np.uint8)
    
    # Save the image
    Image.fromarray(data).save(file.parent.joinpath(file.stem + '.tif'))

def convert_to_tif():
    dataset_path = Path('./dataset')
    file_list = list(dataset_path.glob('**/*.hgt'))
    
    with Pool() as pool:
        # Process the files in parallel using a pool of processes
        pool.map(process_file, file_list)

convert_to_tif()
