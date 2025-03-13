from ultralytics import YOLO
import shutil

model = YOLO("runs/detect/train2/weights/best.pt")

results = model.predict(source="Input", show=False, show_boxes=False)

counter = 0
for result in results:
    if len(result) != 0:
        shutil.copy(result.path, "Output/image" + str(counter) + ".jpg")
        counter += 1
        print("copied")


