import imageio
import sys
import os
import numpy as np

if (len(sys.argv) != 2):
    print("Format: python convert.py grayscale_image_name")
    sys.exit(1)

try:
    data = imageio.imread(sys.argv[1])
except:
    print("Wrong image name!")
    sys.exit(1)

if (len(data.shape) != 2):
    print("Image must be grayscale!")
    sys.exit(1)

output = open(os.path.splitext(sys.argv[1])[0] + ".c", "w")
output.write("const unsigned char my_pic[] = {\n")

image = data.flatten(order='C')

fimage = np.array_split(image, image.shape[0]//16)

for chunk in fimage:
    fstr = ', '.join(['0x%02x'%x for x in chunk])
    output.write("    " + fstr)
    output.write(",\n")

output.write("}")
output.close()

print("Done! The array is stored in "  +\
       os.path.splitext(sys.argv[1])[0] + ".c")
