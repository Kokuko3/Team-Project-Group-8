from ultralytics import YOLO
import shutil

# Load YOLO model
model = YOLO("runs/detect/train2/weights/best.pt")

# Save results of prediction from Input folder to results array
results = model.predict(source="Input", show=True)

# Counter for file naming
counter = 0

# For each result, check if detection was found then copy and rename image to output folder.
for result in results:
    if len(result) != 0:
        shutil.copy(result.path, "Output/image" + str(counter) + ".png")
        counter += 1
        print("Death star image found")

# In case of failed execution, fill in remaining slots with filler images
while counter < 10:
    shutil.copy("TotallyRealDeathstar.png", "Output/TotallyRealDeathstar" + str(counter) + ".png")
    counter += 1
    print("Death star image artificially inserted")


