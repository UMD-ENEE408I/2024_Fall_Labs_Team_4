import cv2
from ultralytics import YOLO
from time import sleep

cam = cv2.VideoCapture(0)
cam.set(3, 640)
cam.set(4, 480)

model = YOLO("./runs/detect/train/weights/best.pt")
# print(model.names)

while True:
    success, img = cam.read()
    results = model(img, stream=True, conf=0.4)

    for r in results:
        for box in r.boxes:
            x1, y1, x2, y2 = map(int, box.xyxy[0])

            cv2.rectangle(img, (x1,y1), (x2,y2), (0,255,0), 3)
            
            img_cls = int(box.cls[0])
            img_cls_name = model.names[img_cls]

            cv2.putText(img, img_cls_name, (x1+5, y1+20), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

    cv2.imshow('YOLO Detect', img)


    if cv2.waitKey(1) == ord('q'):
        break

    # sleep(0.5)

cam.release()
cv2.destroyAllWindows()
