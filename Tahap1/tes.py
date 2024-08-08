import cv2

# Initialize the tracker
tracker = cv2.TrackerCSRT_create()
# Open the video file or capture from a camera
video = cv2.VideoCapture('/home/bagas/Downloads/video.mp4')

# Read the first frame of the video
ok, frame = video.read()
if not ok:
    print('Cannot read video file')
    exit()

# Define an initial bounding box
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

    # Draw bounding box
    if ok:
        # Tracking success
        p1 = (int(bbox[0]), int(bbox[1]))
        p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
        cv2.rectangle(frame, p1, p2, (255, 0, 0), 2, 1)
    else:
        # Tracking failure
        cv2.putText(frame, "Tracking failure detected", (100, 80), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 0, 255), 2)

    # Display result
    cv2.imshow("Tracking", frame)

    # Exit if ESC pressed
    if cv2.waitKey(1) & 0xFF == 27:
        break

video.release()
cv2.destroyAllWindows()
