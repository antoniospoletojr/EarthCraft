import os
import numpy as np
import cv2

# Directory path containing the .npz files
data_dir = 'test_patches'

# Output directory path for saving the generated images
output_dir = 'output_images'
os.makedirs(output_dir, exist_ok=True)

# Load .npz files and generate output images
for i, file_name in enumerate(os.listdir(data_dir)):
    if file_name.endswith('.npz'):
        file_path = os.path.join(data_dir, file_name)
        data = np.load(file_path)
       
        # Extract the sketch and heightmap from the npz file
        sketch = data['sketch']
        heightmap = data['height']


        # Split the sketch into five individual channels
        sketch_channels = np.split(sketch, sketch.shape[2], axis=2)

        concatenated_images = []
        for sketch_channel in sketch_channels:
            concatenated_images.append(np.uint8(sketch_channel * 127.5 + 127.5))
        
        concatenated_images.append(np.uint8(heightmap * 127.5 + 127.5))

        # Concatenate the individual images horizontally
        final_image = np.concatenate(concatenated_images, axis=1)

        output_file_name = file_name.replace('.npz', f'_output_{i}.png')
        output_file_path = os.path.join(output_dir, output_file_name)
        cv2.imwrite(output_file_path, final_image)

        data.close()