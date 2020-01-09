#include "consumer.hpp"
using namespace cv::dnn;
using namespace dlib;
using namespace std;
using namespace cv; 
//Resnet architecture 
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                            input_rgb_image_sized<150>
                            >>>>>>>>>>>>;

#define CAFFE
const std::string caffeConfigFile = "../../path_requirement/deploy.prototxt";                               // Network description
const std::string caffeWeightFile = "../../path_requirement/res10_300x300_ssd_iter_140000_fp16.caffemodel"; //learned network

const std::string tensorflowConfigFile = "../../path_requirement/opencv_face_detector.pbtxt";
const std::string tensorflowWeightFile = "../../path_requirement/opencv_face_detector_uint8.pb";
anet_type net;
dlib::shape_predictor sp;
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
dlib::rectangle DnnDetectFace::GetFace(cv::Mat &frameOpenCVDNN)
{   
    const size_t inWidth = 300;
    const size_t inHeight = 300;
    const double inScaleFactor = 1.0;
    const float confidenceThreshold = 0.7;
    dlib::rectangle vide, rect;
    int frameHeight=frameOpenCVDNN.rows;
    int frameWidth=frameOpenCVDNN.cols;
    Net net = cv::dnn::readNetFromCaffe(caffeConfigFile, caffeWeightFile);
    cv::Mat inputBlob = cv::dnn::blobFromImage(frameOpenCVDNN, inScaleFactor, cv::Size(inWidth, inHeight), cv::Scalar(104.0, 177.0, 123.0), false, false);
    net.setInput(inputBlob, "data");
    cv::Mat detection = net.forward("detection_out");
    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
    float confidence = detectionMat.at<float>(0, 2);

    if (confidence > confidenceThreshold)
    {
        int x1 = static_cast<int>(detectionMat.at<float>(0, 3) * frameWidth);
        int y1 = static_cast<int>(detectionMat.at<float>(0, 4) * frameHeight);
        int x2 = static_cast<int>(detectionMat.at<float>(0, 5) * frameWidth);
        int y2 = static_cast<int>(detectionMat.at<float>(0, 6) * frameHeight);
        if ((x1 < 20) | (y1 < 20) | (x2 > (frameWidth - 50)) | (y2 > (frameHeight - 50)) | ((x1 + (x2 - x1)) < 150) | ((y1 + (y2 - y1)) < 150))
        {
            return vide;
        }
        rect = dlib::rectangle(x1, y1, (x1 + (x2 - x1)), (y1 + (y2 - y1)));
        return rect;
    }
    return vide;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//callback function of getting face from database
std::vector<string> aux;
static int callback(void *data, int argc, char **argv, char **azColName)
{
    aux.push_back(argv[1]);
    return 0;
}
//callback function of getting face-vector from database
std::vector<dlib::matrix<float, 0, 1>> VecFace;
static int callback1(void *data, int argc, char **argv, char **azColName)
{
    int iRow = 0;
    string lineStream;
    dlib::matrix<float, 128, 1> face_csv;
    stringstream myStreamString;
    myStreamString << argv[0];
    while (getline(myStreamString, lineStream, ';'))
    {
        face_csv(iRow, 0) = stof(lineStream);
        iRow++;
    }
    VecFace.push_back(face_csv);

    return 0;
}

DataBaseManagement::DataBaseManagement()
{

    aux.clear();
    if( access( "../../path_requirement/access.db", F_OK ) == -1 ) {
    // file doesn't exist
      cout<< "database doesn't exist, we will create one!"<<endl;
      CreateDataBase();
    } 
    cout<< "database exist"<<endl;
    // file exist
    exit = sqlite3_open("../../path_requirement/access.db", &DB);
    DataBaseRequestStatus(exit);
    string data("CALLBACK FUNCTION");
    string sql("SELECT * FROM PERSON;");
    exit = sqlite3_exec(DB, sql.c_str(), callback, (void *)data.c_str(), NULL);
    DataBaseRequestStatus(exit);
    setName(aux);
    cout << KnowsNames.size() << endl;
    for (auto &n : KnowsNames)
    {
        cout << n;
    }
    cout << endl;

    for (int i=0; i < KnowsNames.size(); i++)
    {     
        string sql1 = "SELECT ImageVector FROM Image WHERE FistNAME LIKE '"+KnowsNames.at(i)+"';";
        
        exit= sqlite3_exec(DB, sql1.c_str(), callback1, (void *)data.c_str(), NULL);
        DataBaseRequestStatus(exit);
        this->face_receptor.push_back(VecFace);
        VecFace.clear();
    }
    //add unkown at the last to get the label by index
    
    sqlite3_close(DB);
    cout << "reading from DataBase done !" <<"recept="<<face_receptor.size()<< endl;
     
}
bool DataBaseManagement::SignUp(const UserInformation &userInfo)
{
    sqlite3_stmt *res;
    exit = sqlite3_open("../../path_requirement/access.db", &DB);

    string sql = "SELECT email FROM users WHERE users.email='" + userInfo.email + "';";
    exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &res, 0);
    exit = sqlite3_step(res);
    if (exit == SQLITE_ROW)
        {sqlite3_close(DB);sqlite3_finalize(res);std::cout << "account already exists!" << endl;return false;}
    cout << userInfo.name << userInfo.lastName << userInfo.email << userInfo.password << endl;
    exit = sqlite3_open("../../path_requirement/access.db", &DB);
    sql = "INSERT INTO users(FistNAME, LastName, email, password) VALUES ('" + userInfo.name + "', '" + userInfo.lastName + "' , '" + userInfo.email + "', '" + userInfo.password + "');";
    exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, NULL);
    DataBaseRequestStatus(exit);

    std::cout << "succesful registration!" << endl;

    
    sqlite3_close(DB);
    return true;
}
bool DataBaseManagement::SignIn(const std::string email, const std::string password ) 
{ 
    sqlite3_stmt *res;
    exit = sqlite3_open("../../path_requirement/access.db", &DB); 
    string sql ="SELECT email FROM users WHERE users.email='"+email+"';";
    exit = sqlite3_prepare_v2(DB, sql.c_str(), -1, &res, 0);    
    exit = sqlite3_step(res);
    if (exit == SQLITE_ROW) {
        return true;
    } 
    sqlite3_finalize(res);
    sqlite3_close(DB);
    return false;
}
string DataBaseManagement::ImagePersonBase64(std::string name)
{
    cv::Mat blob;
    std::vector<cv::String> PictureRefFileNames;
    string encoded_png;
    std::vector<uchar> buf;
    //! Get file name from pattern: images/Dirname/filename.jpg
    cv::glob("../../path_requirement/images/" + name + "/*.*", PictureRefFileNames, true);
    cv::Mat img = imread(PictureRefFileNames.at(0));
    dlib::rectangle r = DnnDetectFace().GetFace(img);
    blob = img(cv::Rect(cv::Point2i(r.left()-20, r.top()-20), cv::Point2i(r.right() + 20, r.bottom() + 20))).clone();
    imwrite("crop.png", blob);
    cv::imencode(".png", blob, buf);
    auto base64_png = reinterpret_cast<const unsigned char *>(buf.data());
    encoded_png = base64_encode(base64_png, buf.size());
    return encoded_png;
    
}

int DataBaseManagement::DataBaseRequestStatus(int exit)
{
    if (exit != SQLITE_OK)
        {cerr << "Error Request" << endl;return 1;}
    else
        {cout << "Operation OK!" << endl;return 0;}
    
}

std::vector<std::vector<dlib::matrix<float, 0, 1>>> DataBaseManagement::GetFaceVector()
{
    return this->face_receptor;
}

std::vector<string> DataBaseManagement::GetNames()
{
    return this->KnowsNames;
}
void DataBaseManagement::setName(std::vector<string> aux)
{
    KnowsNames = aux;
}
void DataBaseManagement::AddPerson(string label, string imgstream,int i)
{   
    
    char *messaggeError;
    string sql;
    string decoded_string = base64_decode(imgstream);
    std::vector<uchar> data(decoded_string.begin(), decoded_string.end());
    Mat frame = imdecode(data, IMREAD_UNCHANGED);
    imwrite("../../path_requirement/images/"+label+"/"+label+std::to_string(i)+".png",frame);
    ClassifyRecognize cl = ClassifyRecognize(frame);
    std::vector<dlib::matrix<float, 0, 1>> newface = cl.get_face_descriptors();
    exit = sqlite3_open("../../path_requirement/access.db", &DB);
    sql = "INSERT INTO PERSON (FistNAME, LastName, Gender) VALUES ('"+label+"', '' , '');";
    exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
    DataBaseRequestStatus(exit);
    cout<<"hello"<<endl;

    //insert vector in the database as string separate by ;
    for (auto &x : newface)
    {
        string imgvalue;
        for (auto &y : x)
        {
            imgvalue = imgvalue + std::to_string(y) + ";";
        }
        sql = "INSERT INTO Image (Idperson,FistNAME,LastName,ImageVector) VALUES ('0','"+label+"', '' , '"+imgvalue+"');";
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
        DataBaseRequestStatus(exit);
    }
    sqlite3_close(DB);
}
int DataBaseManagement::DeletePerson(string label)
{    
    char *messaggeError;
    string sql;
    exit = sqlite3_open("../../path_requirement/access.db", &DB);
    string path="rm -r ../../path_requirement/images/"+label;
    system(path.c_str());
    sql = "DELETE FROM PERSON WHERE FistNAME ='" +label+"';";
    exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
    DataBaseRequestStatus(exit);
    sql = "DELETE FROM Image WHERE FistNAME ='"+ label+"';";
    exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
    sqlite3_close(DB);
    return DataBaseRequestStatus(exit);
    
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClassifyRecognize::ClassifyRecognize(cv::Mat frame)
{
    this->frame = frame;

}

std::vector<dlib::matrix<float, 0, 1>> ClassifyRecognize::get_face_descriptors()
{
    dlib::rectangle face = DnnDetectFace().GetFace(frame);
    //dlib::rectangle face(0,0,frame.cols,frame.rows);
    dlib::cv_image<bgr_pixel> cimg1(frame);
    std::vector<dlib::matrix<rgb_pixel>> faces;
    auto shape = sp(cimg1, face);
    dlib::matrix<rgb_pixel> face_chip;
    dlib::extract_image_chip(cimg1, get_face_chip_details(shape, 150, 0.25), face_chip);
    faces.push_back(move(face_chip));
    // This call asks the DNN to convert each face image in faces into a 128D vector.
    // In this 128D vector space, images from the same person will be close to each other
    // but vectors from different people will be far apart.  So we can use these vectors to
    // identify if a pair of images are from the same person or from different people.
    face_descriptors = net(faces);
    return face_descriptors;
}

string ClassifyRecognize::GetLabel(std::vector<std::vector<dlib::matrix<float, 0, 1>>> face_receptor, std::vector<string> KnowsNames)
{
    face_descriptors = get_face_descriptors();
    bool stop = false;
    string label="Unkown";
    int response ;
    for (size_t i = 0; i < face_receptor.size() && !stop; ++i)
    {
        for (size_t j = 0; j < face_receptor.at(i).size() && !stop; ++j)
        {
            float facesLength = length(face_descriptors[0] - face_receptor[i][j]);
            cout << "the euclidean distance = "
                 << "-----" << facesLength << "-----" << endl;
            if (facesLength < 0.3 && facesLength>0.1)
            { 
                response = i;
                label=KnowsNames.at(response);
                stop = true;
            }
        }
    }
    
    cout << g_detectedPerson << endl;
    return label;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
server_recognition::server_recognition(DataBaseManagement *data)
{
    this->face_receptor = data->GetFaceVector();
    this->KnowsNames = data->GetNames();
    //We will load face landmarking model to align faces to a standard pose
    deserialize("../../path_requirement/shape_predictor_5_face_landmarks.dat") >> sp;
    // we load the DNN responsible for face recognition.
    deserialize("../../path_requirement/dlib_face_recognition_resnet_model_v1.dat") >> net;
}

string server_recognition::recognize(std::string imgstream)
{   
    //get image from base 64 string
    string decoded_string = base64_decode(imgstream);
    std::vector<uchar> data(decoded_string.begin(), decoded_string.end());
    Mat frame = imdecode(data, IMREAD_UNCHANGED);
    //processing frame
    ClassifyRecognize clar = ClassifyRecognize(frame);
    string label = clar.GetLabel(face_receptor, KnowsNames);
    return label;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CreateDataBase::CreateDataBase()
{   
    std::string SqlStatement = "CREATE TABLE users(IdUser INTEGER PRIMARY KEY AUTOINCREMENT, FistNAME TEXT NOT NULL, LastName TEXT ,email TEXT, password TEXT );"
                               "CREATE TABLE PERSON(ID INTEGER PRIMARY KEY AUTOINCREMENT, FistNAME TEXT NOT NULL, LastName TEXT ,Gender TEXT);"
                               "CREATE TABLE Image (ID INTEGER PRIMARY KEY  AUTOINCREMENT, Idperson INTEGER NOT NULL ,FistNAME TEXT NOT NULL, LastName TEXT , ImageVector BLOB  NOT NULL);"
                               "CREATE TABLE History(ID INT PRIMARY KEY NOT NULL,FistNAME TEXT NOT NULL,LastName TEXT  ,DateTime TEXT );";
 
     exit = sqlite3_open("../../path_requirement/access.db", &DB);
    char *messaggeError;
    exit = sqlite3_exec(DB, SqlStatement.c_str(), NULL, 0, &messaggeError);
    if (exit != SQLITE_OK)
    {
        std::cerr << "Error Create Table" << std::endl;  
    }
    else
        std::cout << "Table created Successfully" << std::endl;
    sqlite3_close(DB);
    
   ExtractDataFromFolder();
   InsertIntoDataBase();
}
 

void CreateDataBase::ExtractDataFromFolder()
{   
    deserialize("../../path_requirement/shape_predictor_5_face_landmarks.dat") >> sp;
    deserialize("../../path_requirement/dlib_face_recognition_resnet_model_v1.dat") >> net;
    int i = 0;
    std::vector<cv::String> PictureRefFileNames;
    int PicturesPersonIndex = 0;
    struct dirent *entry;
    //! Open pictures database folder
    DIR *dir = opendir("../../path_requirement/images");

    // Loop Over each directory used to store images database
    while ((entry = readdir(dir)) != NULL)
    {
        cv::String PersonName = entry->d_name;
        // Check directory name: We are looking only for img pattern
        if ((PersonName.compare(".") != 0) && (PersonName.compare("..") != 0))
        {
            names.push_back(PersonName);
            //! Get file name from pattern: images/Dirname/filename.jpg
            cv::glob("../../path_requirement/images/" + (cv::String)entry->d_name + "/*.*", PictureRefFileNames, true);
            int j = 0;
            std::vector<matrix<rgb_pixel>> classfaces;
            for (auto &PicturePersonFileName : PictureRefFileNames)
            {
                cv::Mat img = imread(PicturePersonFileName);
                //convert opencv image to dlib image
                cv_image<bgr_pixel> cimg(img);
                dlib::rectangle face = DnnDetectFace().GetFace(img);
                // We will also use a face landmarking model to align faces to a standard pose: 
                auto shape = sp(cimg, face);
                matrix<rgb_pixel> face_chip;
                extract_image_chip(cimg, get_face_chip_details(shape, 150, 0.25), face_chip);
                classfaces.push_back(move(face_chip));
                if (classfaces.size() == 0)
                {
                    cout << "No faces found in image!" << endl;
                }
                
            }
            allfaces.push_back(classfaces);
            
        }
    }    
}
void CreateDataBase::InsertIntoDataBase()
{   
    sqlite3_stmt *insertStmt = NULL;
    exit = sqlite3_open("../../path_requirement/access.db", &DB);
    if (exit != SQLITE_OK)
    {
        std::cerr << "Error to open " << std::endl;
    }
    else
    {
        std::cout << "database opened Successfully!" << std::endl;
    }
    cout << names.size() << endl;
    int j = 0;
    for (int i = 0; i < names.size(); i++)
    {
        exit = sqlite3_prepare_v2(DB,
                                  "INSERT INTO PERSON "
                                  "(FistNAME,LastName,Gender)"
                                  "VALUES"
                                  "(:firstname,:lastname,:Gender);",
                                  -1, &insertStmt, NULL);
        if (exit != SQLITE_OK)
             std::cerr << "Error to open " << std::endl;

        exit = sqlite3_bind_text(insertStmt, 1, names.at(i).c_str(), -1, SQLITE_STATIC);
        if (exit != SQLITE_OK)
            std::cerr << "Error to open " << std::endl;
        exit = sqlite3_bind_text(insertStmt, 2, "", -1, SQLITE_STATIC);
        if (exit != SQLITE_OK)
             std::cerr << "Error to open " << std::endl;
        exit = sqlite3_bind_text(insertStmt, 3, "", -1, SQLITE_STATIC);

        exit = sqlite3_step(insertStmt);
        if (exit != SQLITE_DONE && exit != SQLITE_OK)
             std::cerr << "Error to open " << std::endl;

        sqlite3_finalize(insertStmt);
    }

    for (int i = 0; i < allfaces.size(); i++)
    {

        std::vector<matrix<float, 0, 1>> face_descriptors = net(allfaces.at(i));

        for (auto &x : face_descriptors)
        {
            string imgvalue;
            
            for (auto &y : x)
            {
                imgvalue =imgvalue + std::to_string(y)+ ";";
            }
           
            exit = sqlite3_prepare_v2(DB,
                                      "INSERT INTO Image "
                                      "(Idperson,FistNAME,LastName,ImageVector)"
                                      "VALUES"
                                      "(:Idperson,:firstname,:lastname,:ImageVector);",
                                      -1, &insertStmt, NULL);
            if (exit != SQLITE_OK)
			 std::cerr << "Error to open " << std::endl;
            exit = sqlite3_bind_int(insertStmt, 1, i+1);
            
            if (exit != SQLITE_OK)
                 std::cerr << "Error to open " << std::endl;
            exit = sqlite3_bind_text(insertStmt, 2, names.at(i).c_str(), -1, SQLITE_STATIC);
            
            if (exit != SQLITE_OK)
                 std::cerr << "Error to open " << std::endl;
            exit = sqlite3_bind_text(insertStmt, 3, "", -1, SQLITE_STATIC);
            
            if (exit != SQLITE_OK)
                 std::cerr << "Error to open " << std::endl;
            exit = sqlite3_bind_text(insertStmt, 4, imgvalue.c_str(), -1, SQLITE_STATIC);

            exit = sqlite3_step(insertStmt);
            if (exit != SQLITE_DONE && exit != SQLITE_OK)
                 std::cerr << "Error to open " << std::endl;

            sqlite3_finalize(insertStmt);
        }
    }
    std::cout << "Successfully records insert!" << std::endl;

    sqlite3_close(DB);
}

