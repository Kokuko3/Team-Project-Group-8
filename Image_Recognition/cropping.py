import cv2
import numpy as np
import os
import shutil

# load_images
# Loads all images inside of specified folder into array
def load_images(folder):
    images = []
    
    for file in os.listdir(folder):
        if file.lower().endswith(".png") or file.lower().endswith(".jpg"):
            path = folder + "/" + file
            images.append(cv2.imread(path))
    
    return images

# Load all images from folder into array
images = load_images("Output")

# Counter for file naming purposes
counter = 0

# For each image i n array perform cropping procedure
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

    # find contours in masked image
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # if contours present
    if contours:
        # isolate largest contour
        largest_contour = max(contours, key=cv2.contourArea)
        
        # determine if contour matches circle
        (x, y), radius = cv2.minEnclosingCircle(largest_contour)
        x, y, radius = int(x), int(y), int(radius)

        # crop image to circle
        cropped_image = image[y-radius:y+radius, x-radius:x+radius]

        # save image
        cv2.imshow("Cropped Image", cropped_image)
        cv2.imwrite("Output2/image" + str(counter) + ".png", cropped_image)
        print("Weakness detected")
        cv2.waitKey(0)
        cv2.destroyAllWindows()
        counter += 1
    else:
        print("No weakness detected.")

# If not 10 weaknesses, insert additional to retain functionality
while counter < 10:
    shutil.copy("TotallyRealWeakness.png", "Output2/TotallyRealWeakness" + str(counter) + ".png")
    counter += 1    
    print("Weakness Inserted")