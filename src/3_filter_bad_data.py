import os
import numpy as np
from tqdm import tqdm
from multiprocessing import Pool

def process_file(file_name):
    file_path = os.path.join(data_dir, file_name)
    data = np.load(file_path)
    
    sketch = data['sketch']
    heightmap = data['height']

    sketch_channels = np.split(sketch, sketch.shape[2], axis=2)

    total_variance = heightmap.var()
    for sketch_channel in sketch_channels:
        total_variance += sketch_channel.var()
    
    if total_variance > 0.5:
        print("File:", file_name, "Variance:", total_variance)
        src = os.path.join('training_set', file_name)
        dst = os.path.join('npz_to_test', file_name)
        command = "cp " + src + " " + dst
        os.system(command)  

    if np.isnan(total_variance) or total_variance < 0.08:
        print("File:", file_name, "Variance:", total_variance)      
        os.remove(file_path)

    data.close()

if __name__ == '__main__':
    print("Filtering")
    data_dir = 'training_set'
    files = os.listdir(data_dir)

    with Pool() as pool:
        pool.map(process_file, [file_name for file_name in files if file_name.endswith('.npz')])