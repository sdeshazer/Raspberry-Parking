while true
do
host=rasbperrypi #if needed
user=username #if needed 
pass=password #if needed
cd /home/pi/
python capture.py #captures image resolution of (1280 x 720)
cd /home/pi/projects/VisualPark/
./VisualPark
curl --user lcccodin:password -T /home/pi/projects/VisualPark/Output/ParkingStatuses.yml ftp://www.lcccodingclub.org/public_html/parkingapp/ParkingStatuses.yml
sleep 30
done
