from PIL import Image
import os

# script which was used to convert the .jpg images from the coco dataset to ppm

in_path = ""
out_path = "coco_dataset_val_2017_ppms"
for filename in os.listdir(in_path):
    out_name = filename.split(".")[0]
    in_img = Image.open("./"+in_path+ "/"+filename)
    in_img.save("./" + out_path +"/" + out_name + ".ppm")