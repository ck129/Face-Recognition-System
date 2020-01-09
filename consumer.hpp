#ifndef CONSUMER_H
#define CONSUMER_H

#include <dlib/dnn.h>
#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
 #include <vector>
#include <string>
#include <iostream>
#include "../Libs/base64/base64.h"
#include <sqlite3.h> 
 
using namespace std;
using namespace cv; 
// ----------------------------------------------------------------------------------------

class DnnDetectFace
{
private:
    
public: 
    //extracting the face region from cv::Mat with DNN face detection.
    dlib::rectangle GetFace(cv::Mat &frameOpenCVDNN);
};

class DataBaseManagement
{ 

public :
    typedef struct {
   std::string email;
   std::string password;
   std::string name;
   std::string lastName;   
} UserInformation;
private:
    //vector of embedding face vector for every class
    std::vector<std::vector<dlib::matrix<float, 0, 1>>> face_receptor;
    //vector of faces
    std::vector<string> KnowsNames;
    sqlite3 *DB;
    int exit = 0;
  

public: 
    //get names and vectors from database
 
DataBaseManagement();
bool SignUp(const UserInformation &userInfo);
bool SignIn(const std::string email, const std::string password);
std::string ImagePersonBase64(std::string name);
//getter of face receptor from database
std::vector<std::vector<dlib::matrix<float, 0, 1>>> GetFaceVector();
//getter of names
std::vector<string> GetNames();
int DeletePerson(string label);
void AddPerson(std::string label ,std::string imgstream, int i);
void setName(std::vector<string>);
int DataBaseRequestStatus(int exit);
    
};

class CreateDataBase
{
private:
   sqlite3 *DB;
   int exit=0;
   char *messaggeError;
   std::vector<cv::String> names;
   std::vector<std::vector<dlib::matrix<dlib::rgb_pixel>>> allfaces; 
   
 

public:
    CreateDataBase();
public:    
    void ExtractDataFromFolder();
public:    
    void InsertIntoDataBase();

};

class ClassifyRecognize
{
private:
    string g_detectedPerson = "";
    cv::Mat frame;
    std::vector<dlib::matrix<float, 0, 1>> face_descriptors;

public:
    ClassifyRecognize(cv::Mat frame);
    std::vector<dlib::matrix<float, 0, 1>> get_face_descriptors();
    std::string GetLabel(std::vector<std::vector<dlib::matrix<float, 0, 1>>> face_receptor, std::vector<string> KnowsNames);
};

class server_recognition
{
private:
    std::vector<std::vector<dlib::matrix<float, 0, 1>>> face_receptor;
    std::vector<string> KnowsNames;

public:
    server_recognition(DataBaseManagement *data);
    std::string recognize(std::string imgstream);
};

#endif
