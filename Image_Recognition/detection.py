import argparse
import shutil
from ultralytics import YOLO

# Argument parser to specify output directory
parser = argparse.ArgumentParser(description='Process and save images to a specified directory.')
parser.add_argument('output_dir', type=str, help='Directory to save the result images')
args = parser.parse_args()

# Load YOLO model
model = YOLO("Image_Recognition/runs/classify/train3/weights/best.pt")

# Save results of prediction from Input folder to results array
results = model.predict(source="main_driver/jpg_folder")

# Counter for file naming
counter = 0

detections = []

# For each result, check if detection was found then copy and rename image to output folder.
for result in results:
    if result.probs.top1 == 1 and result.probs.top1conf >= 0.50:
        detections.append(result)
        print("Death star image found")
    else:
        print("No death star found")

# If more than 10 detections, sort in order of confidence and remove lowest confidence detections
if len(detections) > 10:
    for i in range(len(detections)):
        swap = False
        
        for j in range(0, len(detections)-i-1):
            if detections[j].probs.top1conf < detections[j+1].probs.top1conf:
                temp = detections[j]
                detections[j] = detections[j+1]
                detections[j+1] = temp
                swap = True

        if swap == False:
            break 
    
    while len(detections) > 10:
        detections.pop()

    print("Trimmed results")

for detection in detections:
    print(detection.probs.top1conf)
    shutil.copy(detection.path, f"{args.output_dir}/image{counter}.png")
    counter += 1

while counter < 10:
    shutil.copy("Image_Recognition/TotallyRealDeathstar.png", f"{args.output_dir}/TotallyRealDeathstar{counter}.png")
    counter += 1
    print("Death star image artificially inserted")



