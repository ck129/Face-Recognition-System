#ifndef POSTFRAME_H
#define POSTFRAME_H
#include <string>
#include <vector>
#include <stdlib.h>
#include <opencv2/core.hpp>
#include <pthread.h>
#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <unistd.h>
#include "base64.h"
#include <vector>
#include <string>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace cv;
using namespace web;
using namespace web::http;
using namespace web::http::client;

class PostFrame
{

private:
    String faceCascadePath;
    CascadeClassifier faceCascade;
    cv::Mat frame;
    string label;


public:
    PostFrame();
public:
     cv::Mat detectFaceOpenCVHaar ( Mat &frameOpenCVHaar, int inHeight, int inWidth);
public:
     string EncodeImageToJson(cv::Mat &roi);
public:
    string put_frame(string path);
public:
     stringstream serialize(cv::Mat &input);
public:
     std::vector<std::string> getListName();





     cv::Mat getFrame() const;
     void setFrame(const cv::Mat &value);
     void setLabel(const string &value);
};

#endif // POSTFRAME_H
