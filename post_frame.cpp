#include <string>
#include <vector>
#include <pthread.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include "../Libs/base64/base64.h"
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <boost/property_tree/json_parser.hpp>
#include <espeak/speak_lib.h>
#include <time.h>
using namespace std;
using namespace cv;
using namespace web;
using namespace web::http;
using namespace web::http::client;
cv::Mat gFrame, roi, image_tosend;
string g_detectedPerson="";
/** Global variables */
String faceCascadePath;
CascadeClassifier faceCascade;
float g_detectedFaceX=0;
float g_detectedFaceY=0;
float g_detectedFaceWidth=0;
float g_detectedFaceHeight=0;

//return the face region with Haar detection
cv::Mat detectFaceOpenCVHaar(CascadeClassifier faceCascade, Mat &frameOpenCVHaar, int inHeight = 300, int inWidth = 0)
{
  cv::Mat empty, roi;
  int x1, x2, y1, y2;
  int frameHeight = frameOpenCVHaar.rows;
  int frameWidth = frameOpenCVHaar.cols;
  if (!inWidth)
    inWidth = (int)((frameWidth / (float)frameHeight) * inHeight);

  float scaleHeight = frameHeight / (float)inHeight;
  float scaleWidth = frameWidth / (float)inWidth;

  Mat frameOpenCVHaarSmall, frameGray;
  resize(frameOpenCVHaar, frameOpenCVHaarSmall, Size(inWidth, inHeight));
  cvtColor(frameOpenCVHaarSmall, frameGray, COLOR_BGR2GRAY);

  std::vector<Rect> faces;
  faceCascade.detectMultiScale(frameGray, faces);

  for (size_t i = 0; i < faces.size(); i++)
  {
    x1 = (int)(faces[i].x * scaleWidth);
    y1 = (int)(faces[i].y * scaleHeight);
    x2 = (int)((faces[i].x + faces[i].width) * scaleWidth);
    y2 = (int)((faces[i].y + faces[i].height) * scaleHeight);
    //cout << y2-y1<<"****"<<endl;
    if ((x1 < 20) | (y1 < 20) | (x2 > (frameWidth - 50)) | (y2 > (frameHeight - 50)) | (y2 - y1) < 190)
    {
      return empty;
    }

    g_detectedFaceWidth=x2;
    g_detectedFaceHeight=y2;
    g_detectedFaceX = x1;
    g_detectedFaceY = y1;
    rectangle(frameOpenCVHaar, Point(x1, y1), Point(x2, y2 + 50), Scalar(0, 255, 0), (int)(frameHeight / 150.0), 4);
    roi = frameOpenCVHaar(cv::Rect(x1, y1, x2 - x1, (y2 + 40) - y1)).clone();
    return roi;
  }
  return empty;
}

string EncodeImageToJson(cv::Mat &roi)
{

  string encoded_png;
  vector<uchar> buf;
  cv::imencode(".png", roi, buf);
  auto base64_png = reinterpret_cast<const unsigned char *>(buf.data());
  encoded_png = base64_encode(base64_png, buf.size());
  return encoded_png;
}

std::string name;
void IdentifyDetectedFace()
{
  if (!roi.empty())
  {
    cout << "sending frame to the server" << endl;
    string bufferr = EncodeImageToJson(image_tosend);
    web::json::value json_v;
    json_v["image"] = web::json::value::string(bufferr);
    try
    {
      web::http::client::http_client client("http://192.168.153.13:6502/v1/telnetai/api");
      client.request(web::http::methods::POST, U("/users/identify"), json_v)
          .then([](const web::http::http_response &response) {
            name = response.extract_string().get();
          })
          .wait();
      boost::property_tree::ptree pt;
      std::istringstream is(name);
      boost::property_tree::json_parser::read_json(is, pt);
      g_detectedPerson=pt.get<std::string>("name");
      std::cout << "you are -------" << g_detectedPerson << "-------" << endl;
      roi.release();
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }
  }
}
int g_ThreadsSpeakStop = false;
void * SpeakName(void *threadid)
{ clock_t start;
  cout << "SpeakName Thread Started ! " << endl;
  static cv::String LastNameDetected = "";
  espeak_VOICE voice;
  memset(&voice, 0, sizeof(espeak_VOICE)); // Zero out the voice first
  voice.languages = "fr";
  voice.name = "french";
  voice.variant = 5;
  voice.gender = 2;
  espeak_POSITION_TYPE position_type;
  unsigned int *unique_identifier;
  void* user_data;
  while(g_ThreadsSpeakStop != true)
  {
    cv::String textToSpeak = "HELLO  " + g_detectedPerson;
    if((((double) (clock() - start)) / CLOCKS_PER_SEC)>10){LastNameDetected = "";start=0;}
    if ((g_detectedPerson.compare("Unkown") != 0) && (g_detectedPerson.compare(LastNameDetected) != 0))
    { 
      start = clock();
      LastNameDetected = g_detectedPerson;
      espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 2000, NULL, 0);
      espeak_SetVoiceByProperties(&voice); 
      cout << "Saying : " << textToSpeak << endl;
      char cTextToSpeak[textToSpeak.size() + 1] = {0};
      memcpy( &cTextToSpeak[0], textToSpeak.c_str(), textToSpeak.size());
      espeak_Synth(cTextToSpeak, textToSpeak.size() + 1, 0, position_type, 0, espeakCHARS_AUTO, unique_identifier, user_data);
      espeak_Synchronize( );
      espeak_Terminate( );
      cout << "Done !" << endl;
    }
    usleep(1 * 500 ); // Wait for 1s
  }
  cout << "Leaving SpeakName Thread ! " << endl;
  pthread_exit(NULL);
}

int g_FaceDetectAndIdentifyThreadstop = false;
int g_gFrameDoPrecessing = false;
//communicate with the server
void *FaceDetectAndIdentifyThread(void *threadid)
{
  faceCascadePath = "./haarcascade_frontalface_default.xml";
  if (faceCascade.load(faceCascadePath))
  {
    while (g_FaceDetectAndIdentifyThreadstop != true)
    {
      if (g_gFrameDoPrecessing == true)
      {
        roi = detectFaceOpenCVHaar(faceCascadePath, gFrame);

        IdentifyDetectedFace();

        g_gFrameDoPrecessing = false;
      }
      usleep(2 * 1000);
    }
  }
  else
  {
    printf("--(!)Error loading face cascade\n");
  }
  cout << "Leaving ServerConnect Thread ! " << endl;
  pthread_exit(NULL);
}

int main(int argc, char **argv)
{
  pthread_t threads[2];
  int rc;
  void *status;
  VideoCapture source;
  cv::Mat iFrame;

  if (pthread_create(&threads[0], NULL, FaceDetectAndIdentifyThread, NULL)) {
    cout << "Error:unable to create FacesRecognizer thread !!!" << endl;
    exit(-1);
  }

  if (pthread_create(&threads[1], NULL, SpeakName, NULL)) {
    cout << "Error:unable to create SpeakName thread !!!" << endl;
    exit(-1);
  }

  source.open(0);

  while (1)
  {
    source >> image_tosend;
    cv::resize(image_tosend, iFrame, cv::Size(800, 600));
    if ((g_detectedPerson != "" ) && (g_detectedPerson.compare("unknown") != 0))
    {
       putText(iFrame, format("Welcome--%s" ,g_detectedPerson.c_str()), Point(30, 30), CV_FONT_HERSHEY_TRIPLEX, 1.0, Scalar(255,0,0), 1.0);
    }
    imshow("OpenCV - DNN Face Detection", iFrame);
    if (g_gFrameDoPrecessing == false)
    {
      gFrame = iFrame;
      g_gFrameDoPrecessing = true;
    }

    int k = waitKey(100);
    if (k == 27)
    {
      destroyAllWindows();
      break;
    }
  }

  g_ThreadsSpeakStop = true;
  g_FaceDetectAndIdentifyThreadstop = true;
  pthread_join(threads[0], &status);
  pthread_join(threads[1], &status);
  return 0;
}
