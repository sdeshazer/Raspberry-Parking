import time
import picamera

with picamera.PiCamera() as camera:
    camera.resolution = (1280, 720)
    camera.start_preview()
    time.sleep(2)
    camera.capture('/home/pi/projects/VisualPark/images/latest.png')
