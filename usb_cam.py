import cv2

# Initialize the tracker
tracker = cv2.TrackerCSRT_create()

# Open the USB camera (usually index 0 for the first connected camera)
video = cv2.VideoCapture(2)

# Check if the camera opened successfully
if not video.isOpened():
    print("Could not open video device")
    exit()

# Set the resolution to 640x480
video.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
video.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

# Read the first frame from the camera
ok, frame = video.read()
if not ok:
    print('Cannot read video frame')
    exit()

# Select the bounding box on the first frame
bbox = cv2.selectROI(frame, False)

# Initialize the tracker with the first frame and bounding box
ok = tracker.init(frame, bbox)

while True:
    # Read a new frame
    ok, frame = video.read()
    if not ok:
        break

    # Update the tracker
    ok, bbox = tracker.update(frame)

    # Draw the bounding box and print the center coordinates
    if ok:
        # Tracking success
        p1 = (int(bbox[0]), int(bbox[1]))
        p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
        cv2.rectangle(frame, p1, p2, (255, 0, 0), 2, 1)

        # Calculate the center of the bounding box
        center_x = int(bbox[0] + bbox[2] / 2)
        center_y = int(bbox[1] + bbox[3] / 2)

        # Print the center coordinates
        print(f"Center of bounding box: ({center_x}, {center_y})")

        # Optionally, draw the center point on the frame
        cv2.circle(frame, (center_x, center_y), 5, (0, 255, 0), -1)
    else:
        # Tracking failure
        cv2.putText(frame, "Tracking failure detected", (100, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)

    # Display the result
    cv2.imshow("Tracking", frame)

    # Exit if ESC is pressed
    if cv2.waitKey(1) & 0xFF == 27:
        break

video.release()
cv2.destroy
