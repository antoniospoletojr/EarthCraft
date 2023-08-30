from pysheds.grid import Grid
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors
import seaborn as sns
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("filename", help="The name of the file to be processed")
parser.add_argument("output_file", help="The name of the output file")
args = parser.parse_args()

grid = Grid.from_raster(args.filename, data_name="dem")

fig, ax = plt.subplots(figsize=(8,6))
fig.patch.set_alpha(0)

im = ax.imshow(grid.dem, extent=grid.extent, cmap='terrain', zorder=1)
ax.set_title('Digital elevation map', size=14)
ax.set_xlabel('Longitude')
ax.set_ylabel('Latitude')
fig.colorbar(im, ax=ax, label='Elevation (m)')
ax.grid(zorder=0)

fig.tight_layout()
fig.savefig(args.output_file, dpi=300)