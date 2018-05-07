//======================================================
//    Optimized For Raspberry Pi 3 B + Pi Cam
//======================================================


#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

int main() {

	//  ======================================
	//  Define File Type: AVI, MP4, PNG, JPG
	//  ======================================
	string fileType = "jpg";


	//  ======================================
	//  File Name Path
	//  ======================================
	string videoName = "/home/pi/VisionPark/Images/test." + fileType;

	//  ======================================
	//  Parking Data Set Map Overlay:                                     imageset1
	//  ======================================                            parkinglot_1
	string parkingData = "/home/pi/VisionPark/DataSets/imageset1.txt";

	//  =========================================
	//  Binary Set - yml updates simulataneously:
	//  =========================================
	ofstream ParkingStatuses;
	map<int, int> parkingStatuses;

	//  ======================================
	//  Parse Configuration File
	//  ======================================
	ConfigLoad::parse();

	//  ======================================
	//  Constant Declaration
	//  ======================================
	const string videoFilename = videoName;
	//  parsing data set for map overlay:==============================
	vector<Parking> parking_data = parse_parking_file(parkingData);
	const int delay = 1;

	// allocating space for reading in the media file
	cv::VideoCapture cap;

	//  ==================================================
	//   If Using Live Camera Feed (Default is 0):
	//  =================================================
	if (videoFilename == "0" || videoFilename == "1" || videoFilename == "2") {

		printf("Loading Connected Camera... \n");
		cap.open(stoi(videoFilename));
		cv::waitKey(500);

	}//if

	 //  ==================================================
	 //   IF IMAGE IS USED:
	 //  =================================================
	if (fileType == "jpg" || fileType == "png") {
		cap >> cv::imread(videoFilename);
	}//if

	else {

		cap.open(videoFilename);

		if (!cap.isOpened()) {

			cout << "Could not open" << videoFilename << endl;
			return -1;

		}//if
	}//else


	const double total_frames = cap.get(cv::CAP_PROP_FRAME_COUNT);

	cv::Size videoSize = cv::Size((int)cap.get(cv::CAP_PROP_FRAME_WIDTH),
		(int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));

	cv::VideoWriter outputVideo;

	//  ==================================================
	//   IF VIDEO IS USED:
	//  =================================================
	// set config save_video to false to only
	// output the binary data set instead.
	if (fileType == "avi" || fileType == "mp4") {
		cout << "VIDEO DETECTED" << endl;
		if (ConfigLoad::options["SAVE_VIDEO"] == "true") {

			string::size_type pAt = videoFilename.find_last_of('.');
			const string videoOutFilename = videoFilename.substr(0, pAt) + "_out2." + fileType;
			int ex = static_cast<int>(cap.get(cv::CAP_PROP_FOURCC)); // get Codec type
			int fourCC = CV_FOURCC('M', 'J', 'P', 'G');
			outputVideo.open(videoOutFilename, fourCC, cap.get(cv::CAP_PROP_FPS), videoSize, true);

		}// if SAVE_VIDEO = TRUE

		cv::Mat frame, //frame from cap
			frame_blur, //blur matrix
			frame_gray, // grayscale matrix
			frame_out, //output matrix
			roi, //Region of Interest
			laplacian; //Gradiance mapping, determines if occupied
		cv::Scalar // Bounding rectangle properties
			delta,
			color;

		char c;
		ostringstream oss;
		cv::Size blur_kernel = cv::Size(5, 5);

		cv::namedWindow("Parking", cv::WINDOW_AUTOSIZE);

		while (cap.isOpened()) {

			cap.read(frame);

			if (frame.empty()) {
				printf("error reading frames or end of video");
				return -1;
			}//if

			double video_pos_msec = cap.get(cv::CAP_PROP_POS_MSEC); //milliseconds
			double video_pos_frame = cap.get(cv::CAP_PROP_POS_FRAMES); // number of frames

			frame_out = frame.clone();

			cv::cvtColor(frame, //convert color
				frame_gray,
				cv::COLOR_BGR2GRAY);

			cv::GaussianBlur(frame_gray,
				frame_blur,
				blur_kernel,
				3,
				3);


			if (ConfigLoad::options["DETECT_PARKING"] == "true") {
				for (Parking& park : parking_data) {
					//If you get an error here, please check that the map
					//data set does not exceed the image resolution!
					roi = frame_blur(park.getBoundingRect());
					cv::Laplacian(roi, laplacian, CV_64F);
					delta = cv::mean(cv::abs(laplacian), park.getMask());
					park.setStatus(delta[0] < atof(ConfigLoad::options["PARK_LAPLACIAN_TH"].c_str()));
					//	printf("| %d: d=%-5.1f", park.getId(), delta[0]);
				}//for Parking
			}//if Detect_Parking


			 //===================================================
			 //			Open Binary Set: [ video ]
			 //===================================================
			ParkingStatuses.open("/home/pi/VisionPark/Output/ParkingStatuses.yml");//, std::ofstream::binary);

			for (int i = 0; i < parking_data.size(); i++) {

				if (parking_data[i].getStatus()) {

					color = cv::Scalar(0, 255, 0);

					parkingStatuses[i] = 0; // spot open

				}//

				else {
					color = cv::Scalar(0, 0, 255);

					parkingStatuses[i] = 1; //closed

				}//else


				ParkingStatuses << "ParkingSpot " << i << ": " << parkingStatuses[i] << endl;

				// ========================================================
				//			Adding Text per ID
				// ========================================================
				cv::drawContours(frame_out,
					parking_data[i].getContourPoints(),
					-1,
					color,
					2,
					cv::LINE_AA);

				cv::Point p = parking_data[i].getCenterPoint(); // (2 * x + width)/2, (2* y + height )/2

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x + 1, p.y + 1),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x - 1, p.y - 1),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x - 1, p.y + 1),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x + 1, p.y - 1),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					p,
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(0, 0, 0),
					2,
					cv::LINE_AA);
			}//for

			 // Close Binary:
			ParkingStatuses.close();

			oss.str("");
			oss << (unsigned long int)video_pos_frame << " / " << total_frames;

			cv::putText(frame_out,
				oss.str(),
				cv::Point(5, 30),
				cv::FONT_HERSHEY_SIMPLEX,
				0.7,
				cv::Scalar(0, 255, 255),
				2,
				cv::LINE_AA);

			//save video:
			if (ConfigLoad::options["SAVE_VIDEO"] == "true") {

				outputVideo.write(frame_out);
			}//if Save is true

			cv::imshow("Parking", frame_out);
			c = (char)cv::waitKey(delay);
			if (c == 27) break;
		}//while cap is opened
	}//IF VIDEO USED

	 //  =========================================================================================

	else if (fileType == "png" || fileType == "jpg") {

		cv::Mat frame,
			frame_blur,
			frame_gray,
			frame_out,
			roi,
			laplacian;

		cv::Scalar
			delta,
			color;

		char c;
		ostringstream oss;
		cv::Size blur_kernel = cv::Size(5, 5);
		cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

		frame = cv::imread(videoFilename);
		//***
		if (!frame.empty()) {

			if (frame.empty()) {
				printf("error reading frames or end of frames");
				return -1;
			}//if
			double video_pos_msec = cap.get(cv::CAP_PROP_POS_MSEC);
			double video_pos_frame = cap.get(cv::CAP_PROP_POS_FRAMES);

			frame_out = frame.clone();
			cout << "Cloned frame!" << endl;
			cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
			cv::GaussianBlur(frame_gray, frame_blur, blur_kernel, 3, 3);

			printf("\n");

			if (ConfigLoad::options["DETECT_PARKING"] == "true") {
				for (Parking& park : parking_data) {
					roi = frame_blur(park.getBoundingRect());
					cv::Laplacian(roi, laplacian, CV_64F);
					delta = cv::mean(cv::abs(laplacian), park.getMask());
					park.setStatus(delta[0] < atof(ConfigLoad::options["PARK_LAPLACIAN_TH"].c_str()));
					printf("| %d: d=%-5.1f", park.getId(), delta[0]);
				}//for
			}//if
			printf("\n");
			//===================================================
			//			Open Binary Set: [ image ]
			//===================================================
			ParkingStatuses.open("/home/pi/VisionPark/Output/ParkingStatuses.yml");//, std::ofstream::binary);

			for (int i = 0; i < parking_data.size(); i++) {
				if (parking_data[i].getStatus()) color = cv::Scalar(0, 255, 0);
				else color = cv::Scalar(0, 0, 255);
				cv::drawContours(frame_out,
					parking_data[i].getContourPoints(),
					-1,
					color,
					2,
					cv::LINE_AA);

				cv::Point p = parking_data[i].getCenterPoint();
				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x + 1, p.y + 1),
					cv::FONT_HERSHEY_SIMPLEX, 0.5,
					cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x - 1, p.y - 1),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x - 1, p.y + 1),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					cv::Point(p.x + 1, p.y - 1),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5, cv::Scalar(255, 255, 255),
					2,
					cv::LINE_AA);

				cv::putText(frame_out,
					to_string(parking_data[i].getId()),
					p,
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(0, 0, 0),
					2,
					cv::LINE_AA);

				//Video Overlay:
				oss.str("");
				//                        Frames Processed     /     Total Frames
				oss << (unsigned long int)video_pos_frame << " / " << total_frames;
				cv::putText(frame_out, oss.str(), cv::Point(5, 30),
					cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2, cv::LINE_AA);

				cv::imshow("Image", frame_out);
				cout << "Showing Output Frame" << endl;
				// ========================================================
				//	Saving Image:
				// ========================================================
				if (ConfigLoad::options["SAVE_VIDEO"] == "true") {
					string::size_type pAt = videoFilename.find_last_of('.');
					const string videoOutFilename = videoFilename.substr(0, pAt) + "_out2." + fileType;

					cv::imwrite(videoOutFilename,
						frame_out);
					printf("Saving Image!");

				}//if


				c = (char)cv::waitKey(delay);
				if (c == 27) {
					cvWaitKey(0);

					// close the window (needed for linux)
					if (frame_out.empty()) {
						cvDestroyWindow("Output");

					}//if out of frames
				}//if esc

				if (parking_data[i].getStatus()) {
					parkingStatuses[i] = 0; // spot open

				}//if

				else {
					parkingStatuses[i] = 1; //closed
				}

				ParkingStatuses << "ParkingSpot " << i << ": " << parkingStatuses[i] << endl;
				cout << "ParkingSpot " << i << ": " << parkingStatuses[i] << endl;
				cout << endl;
			}//for yml Loop
			 // Close Binary:
			cout << "Closes File" << endl;
			ParkingStatuses.close();
		}//if frame not empty
	}//else If Image
}//main
