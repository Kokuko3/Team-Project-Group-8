from ultralytics import YOLO
import shutil

# Load YOLO model
model = YOLO("runs/classify/train3/weights/best.pt")

# Save results of prediction from Input folder to results array
results = model.predict(source="Input")

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

# save images
for detection in detections:
    print(detection.probs.top1conf)
    shutil.copy(detection.path, "Output/image" + str(counter) + ".png")
    counter += 1

# In case of failed execution, fill in remaining slots with filler images
while counter < 10:
    shutil.copy("TotallyRealDeathstar.png", "Output/TotallyRealDeathstar" + str(counter) + ".png")
    counter += 1
    print("Death star image artificially inserted")


