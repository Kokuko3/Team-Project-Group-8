import cv2
import numpy as np
import os
import shutil
import argparse

# Argument parser to specify input directory
parser = argparse.ArgumentParser(description='Process images from a specified directory and crop them.')
parser.add_argument('input_dir', type=str, help='Directory containing the images to be cropped')
args = parser.parse_args()

# load_images
# Loads all images inside of specified folder into array
def load_images(folder):
    images = []
    
    for file in os.listdir(folder):
        if file.lower().endswith(".png") or file.lower().endswith(".jpg"):
            path = os.path.join(folder, file)
            images.append(cv2.imread(path))
    
    return images

# Load all images from the specified directory into an array
images = load_images(args.input_dir)

# Counter for file naming purposes
counter = 0

# For each image in array perform cropping procedure
for image in images:

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Define bounds for color red
    lower_red1 = np.array([0, 180, 180])
    upper_red1 = np.array([10, 255, 255])
    lower_red2 = np.array([170, 180, 180])
    upper_red2 = np.array([180, 255, 255])

    # Apply red masks to hsv image
    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
    mask = mask1 + mask2

    # Find contours in the masked image
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # If contours are present
    if contours:
        # Isolate largest contour
        largest_contour = max(contours, key=cv2.contourArea)
        
        # Determine if contour matches a circle
        (x, y), radius = cv2.minEnclosingCircle(largest_contour)
        x, y, radius = int(x), int(y), int(radius)

        # Crop the image to the circle
        cropped_image = image[y-radius:y+radius, x-radius:x+radius]
        
        if cropped_image.size > 0:
            # Save the cropped image to the same directory
            output_path = os.path.join(args.input_dir, f"cropped_image{counter}.png")
            cv2.imwrite(output_path, cropped_image)
            print(f"Weakness detected and saved to {output_path}")
            counter += 1
    else:
        print("No weakness detected.")

# If not 10 weaknesses, insert additional to retain functionality
while counter < 10:
    shutil.copy("Image_Recognition/TotallyRealWeakness.png", os.path.join(args.input_dir, f"TotallyRealWeakness{counter}.png"))
    counter += 1    
    print("Weakness Inserted")
