import socket
import struct
import speech_recognition as sr
import cv2
import time
import matplotlib.pyplot as plt
import numpy as np
from ultralytics import YOLO
import math

MODEL_PATH = "" # change this
score = (0, 0)
threshold = 0 # change this

model = YOLO(MODEL_PATH)
cam = cv2.VideoCapture(0)

# dark image
_, image = cam.read()

game = True

while game:
    try:
        _, img = cam.read()
        img_resized = cv2.resize(img, (640, 640), interpolation = cv2.INTER_CUBIC)
        result_boxes = model.track(img_resized, persist=True, verbose=False)

        robot_centers = []

        for box in result_boxes[0].boxes:
            # assuming class 0 is robot
            if box.cls == 0 and box.conf > 0.4:
                x1,y1,x2,y2 = box.xyxy[0].numpy()
                robot_centers.append((int((x1 + x2)/2), int((y1 + y2)/2)))

        robot_centers.sort(key=lambda val: val[0])
        # order: player 1, ball, player 2

        if abs(robot_centers[0][0] - robot_centers[1][0]) < threshold:
            if abs(robot_centers[0][1] - robot_centers[1][1]) < threshold:
                # ball robot considered hit, so it should go in opposite direction
                placeholder_for_now = 0
            else:
                # player 2 scored
                score[1] += 1

        elif abs(robot_centers[1][0] - robot_centers[2][0]) < threshold:
            if abs(robot_centers[1][1] - robot_centers[2][1]) < threshold:
                # ball robot considered hit, so it should go in opposite direction
                placeholder_for_now = 0
            else:
                # player 1 scored
                score[0] += 1

        else:
            if robot_centers[0][1] - robot_centers[1][1] > 0:
                # tell player 1 to move forwards
                placeholder_for_now = 0
            else:
                # tell player 1 to move backwards
                placeholder_for_now = 0

            if robot_centers[2][1] - robot_centers[1][1] > 0:
                # tell player 1 to move forwards
                placeholder_for_now = 0
            else:
                # tell player 1 to move backwards
                placeholder_for_now = 0

        if score[0] > 7 or score[1] > 7:
            # game over
            game = False

    except:
        print("ERROR")
        continue