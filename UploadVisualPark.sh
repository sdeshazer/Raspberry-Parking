while true
do
cd /home/pi/
python capture.py
sleep 2
cd /home/pi/projects/VisualPark/
./VisualPark
curl  --user lcccodin:3C73m3vjqA -T /home/pi/projects/VisualPark/Output/ParkingStatuses.yml  ftp://www.lcccodingclub.org/public_html/ParkingStatuses.yml
done
