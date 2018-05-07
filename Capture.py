import time
import picamera

with picamera.PiCamera() as camera:
camera.resolution = (1280, 720)
camera.start_preview()
# Camera warm-up time
time.sleep(2)
camera.capture('image.jpg')
