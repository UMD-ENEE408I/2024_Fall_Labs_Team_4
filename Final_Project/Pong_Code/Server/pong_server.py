import socket
import struct
import cv2
from ultralytics import YOLO
import threading

# Host IP and port
HOST = '172.20.10.11'  # Replace with your server's IP
PORT = 9500           # Arbitrary non-privileged port (>1024)

print('here')
# Create a TCP/IP socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('here')
server_socket.bind((HOST, PORT))
print('here')
server_socket.listen(5)
print(f"Server started. Listening on {HOST}:{PORT}")

cam = cv2.VideoCapture(0)
cam.set(3, 640)
cam.set(4, 480)

model = YOLO("./best.pt")

score = [0, 0]  
threshold = 50  # Change this
format_string = '<h50s'

game = True
game_lock = threading.Lock()
client_threads = []

def handle_client(connection, client_address):
    global game
    try:
        print(f"Connection from {client_address}")
        while True:
            # Receive data
            data = connection.recv(struct.calcsize(format_string))
            if not data:
                print(f"Client {client_address} disconnected.")
                break

            # Unpack the received data
            seq, text = struct.unpack(format_string, data)
            text = text.decode('utf-8').strip()
            #print(f"Received: seq={seq}, text={text} from {client_address}")

            # Handle termination signal
            if text == "EXIT":
                print(f"Client {client_address} requested disconnection.")
                break

            _, img = cam.read()
            results = model(img, stream=True, conf=0.4)
            robot_centers = []

            for r in results:
                for box in r.boxes:
                    x1, y1, x2, y2 = map(int, box.xyxy[0])
                    cv2.rectangle(img, (x1, y1), (x2, y2), (0, 255, 0), 3)
                    robot_centers.append((int((x1 + x2) / 2), int((y1 + y2) / 2)))
                    
                    img_cls = int(box.cls[0])
                    img_cls_name = model.names[img_cls]
                    cv2.putText(img, img_cls_name, (x1 + 5, y1 + 20), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

            robot_centers.sort(key=lambda val: val[0])
            cv2.imshow('YOLO Detect', img)
            print(robot_centers)

            if len(robot_centers) != 3:
                continue

            robot_num = 0
            message = ""

            if abs(robot_centers[0][0] - robot_centers[1][0]) < threshold:
                if abs(robot_centers[0][1] - robot_centers[1][1]) > threshold:
                    score[1] += 1
                    print("Player 2 scored")
                robot_num = 2
                message = "backwards"

            elif abs(robot_centers[1][0] - robot_centers[2][0]) < threshold:
                if abs(robot_centers[1][1] - robot_centers[2][1]) > threshold:
                    score[0] += 1
                    print("Player 1 scored")
                robot_num = 2
                message = "forwards"

            else:
                if robot_centers[0][1] - robot_centers[1][1] > 0:
                    robot_num = 1
                    message = "forwards"
                else:
                    robot_num = 1
                    message = "backwards"

                if robot_centers[2][1] - robot_centers[1][1] > 0:
                    robot_num = 3
                    message = "forwards"
                else:
                    robot_num = 3
                    message = "backwards"

            if score[0] > 7 or score[1] > 7:
                game = False
                print("Game over: " + str(score))
                robot_num = 4  # all robots
                message = "over"

            # Prepare response
            with game_lock:
                if not game:
                    print(f"Game has ended. Informing client {client_address}.")
                    message = "over"
                    game = False

            message = message.encode('utf-8')

            packed_data = struct.pack(format_string, robot_num, message[:50])
            #print(f"Sending to {client_address}: robo={robot_num}, text={message.decode('utf-8')}")
            connection.sendall(packed_data)
    except Exception as e:
        print(f"Error with client {client_address}: {e}")
    finally:
        connection.close()
        print(f"Connection with {client_address} closed.")

# Accept and handle up to 3 clients
for _ in range(3):
    connection, client_address = server_socket.accept()
    client_thread = threading.Thread(target=handle_client, args=(connection, client_address))
    client_threads.append(client_thread)
    client_thread.start()

# Wait for all client threads to finish
for thread in client_threads:
    thread.join()

cam.release()
cv2.destroyAllWindows()
server_socket.close()
