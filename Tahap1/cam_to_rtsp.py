import cv2

# Set the video source to the USB camera
video_source = 0  # Usually, the default USB camera is 0
video_width = 640
video_height = 480

# RTSP server URL
rtsp_server = "rtsp://localhost:5000/test"

# Open the video source
cap = cv2.VideoCapture(video_source)

# Create a GStreamer pipeline to stream the video
fourcc = cv2.VideoWriter_fourcc(*'X264')
# fourcc = cv2.VideoWriter
out = cv2.VideoWriter(rtsp_server, fourcc, 30.0, (video_width, video_height))

# Check if the video capture opened successfully
if not cap.isOpened():
    print("Error: Could not open video source.")
    exit()

# Capture and stream video
while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read frame.")
        break

    # Resize the frame to match the desired resolution
    frame = cv2.resize(frame, (video_width, video_height))

    # Write the frame to the RTSP stream
    out.write(frame)

    # Display the frame (optional)
    cv2.imshow("Video Stream", frame)

    # Exit if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the video capture and writer objects
cap.release()
out.release()
cv2.destroyAllWindows()
