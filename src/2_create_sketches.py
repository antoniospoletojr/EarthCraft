from tqdm import tqdm
from pathlib import Path
import cv2
import numpy as np
import georasters as gr
from pysheds.grid import Grid
from skimage.morphology import skeletonize
import multiprocessing as mp
import pyproj
import warnings

# Suppress the FutureWarning
pyproj.datadir.set_data_dir('/home/spoleto/.mambaforge/envs/cg/share/proj')
warnings.simplefilter(action='ignore', category=FutureWarning)

# Takes a TIFF image as input and returns two arrays, one for rivers and one for depressions
def compute_rivers(tiff_image):

    # Create a Grid object from the TIFF image
    grid = Grid.from_raster(str(tiff_image), data_name='dem')
    
    # Detect depressions in the grid
    depressions = grid.detect_depressions('dem')
    
    # Fill depressions in the grid to create a "flooded" DEM
    grid.fill_depressions(data='dem', out_name='flooded_dem')

    # Detect flats in the flooded DEM
    flats = grid.detect_flats('flooded_dem')

    # Resolve flats in the flooded DEM to create an "inflated" DEM
    grid.resolve_flats(data='flooded_dem', out_name='inflated_dem')
    
    # Define the flow direction map
    dirmap = (64, 128, 1, 2, 4, 8, 16, 32)

    # Compute flow direction based on the inflated DEM
    grid.flowdir(data='inflated_dem', out_name='dir', dirmap=dirmap)
    
    # Compute flow accumulation based on the computed flow direction
    grid.accumulation(data='dir', out_name='acc', dirmap=dirmap)

    # Downsample the flow accumulation to create the river array
    downsampled_rivers = np.log(grid.view('acc') + 1)

    # Upsample the river array
    upsampled_rivers = cv2.resize(downsampled_rivers, (450, 450))

    # Normalize the upsampled river array to have values between 0 and 1
    upsampled_rivers = (upsampled_rivers - np.amin(upsampled_rivers)) / (np.amax(upsampled_rivers) - np.amin(upsampled_rivers))

    # Scale the upsampled river array to have values between 0 and 255
    upsampled_rivers = np.array(upsampled_rivers * 255, dtype=np.uint8)

    # Threshold the scaled upsampled river array to create a binary array
    _, thresholded_river = cv2.threshold(upsampled_rivers, 127, 255, cv2.THRESH_BINARY)

    # Set all pixels with value 255 in the binary array to 1
    thresholded_river[thresholded_river == 255] = 1

    # Skeletonize the binary array to create a thin version of the rivers
    skeletonized_rivers = skeletonize(thresholded_river)

    # Return the skeletonized river array and the upsampled depression array
    return np.expand_dims(skeletonized_rivers, axis=-1)

# Similar to compute_rivers, but it computes ridges and peaks instead of rivers and depressions
def compute_ridges(tiff_image):
    # Load DEM data from a tiff file into a grid object
    grid = Grid.from_raster(str(tiff_image), data_name='dem')
    # Invert the values of the DEM to find ridges instead of valleys
    grid.dem = grid.dem.max() - grid.dem
    # Detect local minima, which indicate the peaks or ridges
    peaks = grid.detect_depressions('dem')
    # Fill in any depressions in the DEM
    grid.fill_depressions(data='dem', out_name='flooded_dem')
    # Detect flat areas in the filled DEM
    flats = grid.detect_flats('flooded_dem')
    # Resolve flats by setting them to the average value of surrounding cells
    grid.resolve_flats(data='flooded_dem', out_name='inflated_dem')

    # Compute flow direction based on corrected DEM
    dirmap = (64, 128, 1, 2, 4, 8, 16, 32)
    grid.flowdir(data='inflated_dem', out_name='dir', dirmap=dirmap)
    # Compute flow accumulation based on computed flow direction
    grid.accumulation(data='dir', out_name='acc', dirmap=dirmap)
    # Take the logarithm of the flow accumulation to emphasize higher values
    downsampled_ridges = np.log(grid.view('acc') + 1)
    # Upsample the ridges map to match the size of the upsampled peaks map
    upsampled_ridges = cv2.resize(downsampled_ridges, (450, 450))
    # Normalize the values of the upsampled ridges to be between 0 and 1
    upsampled_ridges = (upsampled_ridges - np.amin(upsampled_ridges)) / (np.amax(upsampled_ridges) - np.amin(upsampled_ridges))
    # Convert the values of the upsampled ridges to 8-bit integers (0-255)
    upsampled_ridges = np.array(upsampled_ridges * 255, dtype=np.uint8)
    # Threshold the upsampled ridges to create a binary map
    _, thresholded_ridges = cv2.threshold(upsampled_ridges, 150, 255, cv2.THRESH_BINARY)
    # Convert the binary map to a map of values 0 and 1
    thresholded_ridges[thresholded_ridges == 255] = 1
    # Apply skeletonization to the binary map to create a one-pixel-wide ridge map
    skeletonized_ridges = skeletonize(thresholded_ridges)
    
    # Return the skeletonized ridge map and the upsampled peaks map
    return np.expand_dims(skeletonized_ridges, axis=-1)

# Computes the height map and the sketch map for a given tiff file
def compute_sketches(filepath):
    # Load data
    data = gr.from_file(str(filepath))
    # Compute rivers and basins using the compute_rivers function
    rivers = compute_rivers(filepath)
    # Compute ridges and peaks using the compute_ridges function
    ridges = compute_ridges(filepath)
    
    # Convert high-detail data to a height map
    height_map = np.array(data.raster, dtype=np.float32)
    height_map = np.expand_dims(height_map, axis=-1)
    # Normalize the height map to a range of -1 to 1
    height_map = (height_map - np.amin(height_map)) / (np.amax(height_map) - np.amin(height_map))
    height_map = height_map * 2 - 1

    # Combine ridges, rivers, peaks, and basins into a sketch map
    sketch_map = np.stack((ridges, rivers), axis=2)
    # Remove the last dimension of the sketch map (it's redundant)
    sketch_map = np.squeeze(sketch_map, axis=-1)

    filename = str(filepath.stem)
    np.savez(f'./training_set/{filename}.npz', sketch=sketch_map, height=height_map)


if __name__ == '__main__':
    patches = list(Path('./patches').glob('**/*.tif'))
    total = len(patches)

    with mp.Pool(processes=mp.cpu_count()) as pool:
        for _ in tqdm(pool.imap_unordered(compute_sketches, patches), total=total):
            pass
