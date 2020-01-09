
#include "postframe.h"

cv::Mat PostFrame::getFrame() const
{
    return frame;
}

void PostFrame::setFrame(const cv::Mat &value)
{
    frame = value;
}

void PostFrame::setLabel(const string &value)
{
    label = value;
}

PostFrame::PostFrame()
{
    this->faceCascadePath = "./haarcascade_frontalface_default.xml";
    if( !this->faceCascade.load( faceCascadePath ) ){ printf("--(!)Error loading face cascade\n");  }

}
cv::Mat PostFrame::detectFaceOpenCVHaar( Mat &frameOpenCVHaar, int inHeight, int inWidth)
{
    cv::Mat empty,roi;
    int x1,x2,y1,y2;
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

    for ( size_t i = 0; i < faces.size(); i++ )
    {
        x1 = (int)(faces[i].x * scaleWidth);
        y1 = (int)(faces[i].y * scaleHeight);
        x2 = (int)((faces[i].x + faces[i].width) * scaleWidth);
        y2 = (int)((faces[i].y + faces[i].height) * scaleHeight);
        //cout << y2-y1<<"****"<<endl;
        if((x1<20) | (y1<20) | (x2>(frameWidth-50)) | (y2>(frameHeight-50))| (y2 - y1)<190)
        {
            return empty;
        }
        rectangle(frameOpenCVHaar, Point(x1, y1), Point(x2, y2+50), Scalar(0,255,0), (int)(frameHeight/150.0), 4);
        roi=frameOpenCVHaar(cv::Rect(x1, y1, x2 - x1, (y2+40) - y1)).clone();
        //return roi(frameOpenCVHaar(cv::Rect(x1, y1, x2 - x1, y2 - y1)));
        setFrame(roi);
        return roi;
    }
    return empty;

}
stringstream PostFrame::serialize(cv::Mat &input)
{
    // We will need to also serialize the width, height, type and size of the matrix
    int width = input.cols;
    int height = input.rows;
    int type = input.type();
    size_t size = input.total() * input.elemSize();
    // cout<<"size="<<size<<endl;
    // Initialize a stringstream and write the data
    stringstream ss;
    ss.write((char*)(&width), sizeof(int));
    ss.write((char*)(&height), sizeof(int));
    ss.write((char*)(&type), sizeof(int));
    ss.write((char*)(&size), sizeof(size_t));

    // Write the whole image data 921600
    ss.write((char*)input.data, size);
    ss << '\0';

    return ss;
}
std::string ListNames;
std::vector<std::string> PostFrame::getListName()
{
    web::json::value json_v;
    std::vector<string> names;
    web::http::client::http_client client("http://192.168.153.13:6502/v1/telnetai/api");
    client.request(web::http::methods::GET, U("/users/getlist"), json_v)
            .then([](const web::http::http_response& response)
    {
        ListNames=response.extract_string().get();
    }).wait();
    boost::property_tree::ptree pt;
    std::istringstream is(ListNames);
    boost::property_tree::json_parser::read_json(is, pt);

    stringstream ss(pt.get<std::string>("message"));

    while( ss.good() )
    {
        string substr;
        getline( ss, substr, ';' );

        if(substr=="")break;
        names.push_back( substr );
    }
    return names;
}

string PostFrame::EncodeImageToJson(cv::Mat &roi)
{
    string encoded_png;
    vector<uchar> buf;
    cv::imencode(".png", roi, buf);
    auto base64_png = reinterpret_cast<const unsigned char*>(buf.data());
    encoded_png =base64_encode(base64_png, buf.size());
    return encoded_png;

}
std::string name="";
string PostFrame::put_frame(string path)
{
    web::json::value json_v;
    if(path==("/users/identify")) {
        string bufferr=PostFrame::EncodeImageToJson(frame);
        json_v["image"] = web::json::value::string(bufferr);

    }if(path==("/users/Add")){
        string bufferr=PostFrame::EncodeImageToJson(frame);
        json_v["nbimg"] = web::json::value::string("1");
        json_v["label"]= web::json::value::string(label);
        json_v["image1"] = web::json::value::string(bufferr);


    }if(path==("/users/delete")){
        json_v["label"]= web::json::value::string(label);
    }
    //uri_builder builder("/users");
    //builder.append_path(path);
    web::http::client::http_client client("http://192.168.153.13:6502/v1/telnetai/api");
    client.request(web::http::methods::POST, U(path), json_v)
            .then([](const web::http::http_response& response)
    {
        name=response.extract_string().get();
    }).wait();
    boost::property_tree::ptree pt;
    std::istringstream is(name);
    boost::property_tree::json_parser::read_json(is, pt);
    frame.release();
    return pt.get<std::string>("name");
}






