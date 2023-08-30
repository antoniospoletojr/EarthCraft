from pathlib import Path
import numpy as np
import georasters as gr
from skimage.util import view_as_blocks
from tqdm import tqdm
import os
import multiprocessing as mp


# Function to extract patches from a single raster file
def extract_patches_from_file(raster_file):
    # Read the raster file using the GeoRaster library
    data = gr.from_file(str(raster_file))
    # Resize the raster to 3600x3600 pixels (it was 3601x3601)
    data = data.resize((3600, 3600))
    # Divide the raster into non-overlapping blocks of a specified size (obtaining 8 patches total)
    raster_blocks = view_as_blocks(data.raster, (450, 450))

    # Loop through all blocks
    for i in range(raster_blocks.shape[0]):
        for j in range(raster_blocks.shape[1]):
            raster_data = raster_blocks[i, j]
            image = raster_data.astype(float)
            # Normalize the image
            image = (image-np.min(image))/(np.max(image)-np.min(image)+1e-7)*255
            image = image.astype(np.uint8)
            # Convert the original patch to a GeoRaster object and save it as a TIFF file
            data_out = gr.GeoRaster(raster_data, data.geot, nodata_value=-1, projection=data.projection, datatype=data.datatype,)
            output_filename = str(raster_file).split(".")[0].split("/")[-1] + "_" + str(i) + "_" + str(j)
            data_out.to_tiff('./patches/' + output_filename)

# Extract patches from all raster files in the dataset folder and its subfolders using multiprocessing
def extract_patches_from_raster():
    # Get a list of all raster files in the dataset folder and its subfolders
    raster_files = list(Path('./dataset').glob('**/*.hgt'))

    # Create a multiprocessing pool
    num_processes = mp.cpu_count()
    print(num_processes)
    pool = mp.Pool(processes=num_processes)

    # Use the multiprocessing pool to extract patches from all raster files
    pool.map(extract_patches_from_file, raster_files)

    # Close the multiprocessing pool
    pool.close()
    pool.join()

# Call the function to extract patches from raster files in the dataset folder
extract_patches_from_raster()

