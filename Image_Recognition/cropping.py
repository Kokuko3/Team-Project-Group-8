import cv2
import numpy as np
import os

def load_images(folder):
    images = []
    
    for file in os.listdir(folder):
        if file.lower().endswith(".png") or file.lower().endswith(".jpg"):
            path = folder + "/" + file
            images.append(cv2.imread(path))
    
    return images

images = load_images("Output")

counter = 0
for image in images:
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    lower_red1 = np.array([0, 200, 200])
    upper_red1 = np.array([10, 255, 255])
    lower_red2 = np.array([170, 200, 200])
    upper_red2 = np.array([180, 255, 255])

    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
    mask = mask1 + mask2

    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    if contours:
        largest_contour = max(contours, key=cv2.contourArea)
        
        (x, y), radius = cv2.minEnclosingCircle(largest_contour)
        x, y, radius = int(x), int(y), int(radius)

        cropped_image = image[y-radius:y+radius, x-radius:x+radius]

        cv2.imshow("Cropped Image", cropped_image)
        cv2.imwrite("Output2/image" + str(counter) + ".jpg", cropped_image)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
        counter += 1
    else:
        print("No red circle detected.")
