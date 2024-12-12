from ultralytics import YOLO

# Load a model
model = YOLO("/Users/sanjana/Documents/ENEE408I_Model/yolov8n.pt")  # load a pretrained model (recommended for training)\

# Train the model
results = model.train(data="/Users/sanjana/Documents/ENEE408I_Model/robot_identification/data.yaml", epochs=100, imgsz=640)