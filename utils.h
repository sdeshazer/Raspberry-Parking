#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "Parking.h"

using namespace std;

// parse parking data sets
vector<Parking> parse_parking_file(string filename);