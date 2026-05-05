from ultralytics import YOLO
import cv2
import serial
import time
import smtplib
from email.message import EmailMessage

arduino = serial.Serial('COM3', 9600, timeout=1)
time.sleep(2)

ambulance_model = YOLO(r"c:\Users\mahen\Downloads\Traffic Signals\violation\ambulance.v1i.yolov8\runs\detect\train\weights\best.pt")
violation_model = YOLO(r"c:\Users\mahen\Downloads\Traffic Signals\violation\violation.v1i.yolov8\runs\detect\train\weights\best.pt")

MODE = None
last_sent = ""
email_sent = False
lane_selected = False

print("Waiting for Arduino mode command...")

EMAIL_ADDRESS = "embedded44@gmail.com"
EMAIL_PASSWORD = "alqplnjlmgbyoxst"
TO_EMAIL = "mahendrachintharaboina@gmail.com"

def send_violation_email(image_path):
    print("📨 Email thread started")
    try:
        msg = EmailMessage()
        msg["Subject"] = "Camera Capture Alert"
        msg["From"] = EMAIL_ADDRESS
        msg["To"] = TO_EMAIL
        msg.set_content("Image captured when IR detected")

        with open(image_path, "rb") as f:
            msg.add_attachment(
                f.read(),
                maintype="image",
                subtype="jpeg",
                filename=image_path
            )

        with smtplib.SMTP("smtp.gmail.com", 587, timeout=30) as smtp:
            smtp.ehlo()
            smtp.starttls()
            smtp.ehlo()
            smtp.login(EMAIL_ADDRESS, EMAIL_PASSWORD)
            smtp.send_message(msg)

        print("✅ EMAIL SENT SUCCESSFULLY")

    except Exception as e:
        print("❌ EMAIL FAILED:", e)

cap = cv2.VideoCapture(1)

if not cap.isOpened():
    print("Camera not detected")
    exit()

while True:

    if arduino.in_waiting > 0:
        cmd = arduino.readline().decode().strip()

        if "EMERGENCY:1" in cmd:
            MODE = "emergency"
            print("Emergency Mode Activated")

        elif "VIOLATION:1" in cmd:
            MODE = "violation"
            print("Violation Mode Activated")

    if MODE is None:
        continue

    ret, frame = cap.read()
    if not ret:
        break

    if MODE == "emergency":

        results = ambulance_model(frame)
        ambulance_detected = False

        for result in results:
            for box in result.boxes:
                class_id = int(box.cls[0])
                confidence = float(box.conf[0])
                class_name = ambulance_model.names[class_id]

                # Only detect if confidence > 90%
                if class_name.lower() == "ambulance" and confidence > 0.90:
                    ambulance_detected = True
                    print(f"Ambulance Detected | Confidence: {confidence:.2f}")

        if ambulance_detected:

            if not lane_selected:
                lane = input("Ambulance detected! Enter lane to clear (1-4): ")

                if lane in ["1", "2", "3", "4"]:
                    arduino.write(f"CLEAR:{lane}\n".encode())
                    print(f"Sent CLEAR:{lane} to Arduino")
                    lane_selected = True
                    last_sent = "Emergency"

        else:
            if last_sent != "Normal":
                arduino.write(b"NORMAL\n")
                print("Sent NORMAL to Arduino")
                lane_selected = False
                last_sent = "Normal"

        annotated = results[0].plot()
        cv2.imshow("Emergency Monitoring", annotated)

    # =====================================================
    # ⚠ VIOLATION MODE (EMAIL ONLY)
    # =====================================================
    elif MODE == "violation":

        results = violation_model(frame)
        violation_detected = False

        for result in results:
            for box in result.boxes:
                class_id = int(box.cls[0])
                confidence = float(box.conf[0])
                class_name = violation_model.names[class_id]

                if class_name.lower() == "violation" and confidence > 0.85:
                    violation_detected = True
                    print(f"Violation Detected | Confidence: {confidence:.2f}")
                    
        if violation_detected:
            if not email_sent:
                print("Violation Detected!")

                image_path = "violation_capture.jpg"
                cv2.imwrite(image_path, frame)

                send_violation_email(image_path)

                email_sent = True

        else:
            email_sent = False

        annotated = results[0].plot()
        cv2.imshow("Violation Monitoring", annotated)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
arduino.close()
