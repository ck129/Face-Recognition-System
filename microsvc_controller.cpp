//
//  Created by Ivan Mejia on 12/24/16.
//
// MIT License
//
// Copyright (c) 2016 ivmeroLabs.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. rapidjsonIN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
 
#include "consumer.hpp"
#include <iostream>
#include <std_micro_service.hpp>
#include "microsvc_controller.hpp"
#include <tuple>

using namespace std;
using namespace cv;
using namespace std;
using namespace http;

typedef struct {
   std::string email;
   std::string password;
  
} signonInformation;
DataBaseManagement *data =new DataBaseManagement();
server_recognition *e1=new server_recognition(data);


void MicroserviceController::identifierUser(http_request message)
{
message.extract_string().then([=](std::string request) {
            try
            {
                boost::property_tree::ptree pt;
                std::istringstream is(request);
                read_json(is, pt);
                //get Image from idetify request.
                std::cout << "Detected face (base64): received!"
                          << "\n";
                //pass to B64 image to processing stage
                string label = e1->recognize(pt.get<std::string>("image"));
                json::value response;
                //send persone name to the client
                response["name"] = json::value::string(label);
                message.reply(status_codes::OK, response);
            }
            catch (std::exception &e)
            {
                std::cout << "bad request =";
                message.reply(status_codes::BadRequest);
            }
        });
}


void MicroserviceController::afficherListe(http_request message)
{



         message.extract_string().then([=](std::string request) {
            try
            {
                std::vector<string> names=data->GetNames();
                json::value listnames= web::json::value::array(names.size());
                int i = 0;
                for (auto &n : names)
                {
                  listnames.as_array()[i++] = web::json::value(n);
                }
                json::value response;
                response["users"] = listnames;
                message.reply(status_codes::OK, response);
            }
 
            catch (std::exception &e)
            {
                std::cout << "Request Error";
                message.reply(status_codes::BadRequest);
            }
        });
}


void MicroserviceController::addUser(http_request message) {
        
	message.extract_string().then([=](std::string request) {
            try
            {
                boost::property_tree::ptree pt;
                std::istringstream is(request);
                read_json(is, pt);
                std::cout << "Add request Detected "
                          << "\n";
                int nb=std::atoi(pt.get<std::string>("nbimg").c_str());
		//int y=data->AddPerson(name, pt.get<std::string>(img),i); 
                string name=pt.get<std::string>("label");
                string path="../../path_requirement/images/"+name;
                mkdir(path.c_str(),0777); 
                for(int i=1;i<nb+1;i++)
                {  
                   string img="image"+std::to_string(i);
                   data->AddPerson(name, pt.get<std::string>(img),i);  
                }
                //Update reading data class object and recognition class by deleting the instance
                //and intializing them again.
                delete data;
                delete e1;
                data = new DataBaseManagement();
                e1 = new server_recognition(data);
                json::value response;
                //return response to the client
                response["name"] = json::value::string("Add person operation is successflly");
		/*if (y == 1)
                {
                    response["name"] = json::value::string("Error");
                }*/
                message.reply(status_codes::OK, response);
            }
            catch (std::exception &e)
            {
                std::cout << "bad request =";
                message.reply(status_codes::BadRequest);
            }
        });
}


void MicroserviceController::deleteUser(http_request message)
{
	message.extract_string().then([=](std::string request) {
            try
            {
                boost::property_tree::ptree pt;
                std::istringstream is(request);
                read_json(is, pt);
                std::cout << "Delete request Detected "
                          << "\n";
                //get name to delete them
                int x = data->DeletePerson(pt.get<std::string>("label"));
                //Update data.
                delete data;
                delete e1;
                data = new DataBaseManagement();
                e1 = new server_recognition(data);
                json::value response;
                response["name"] = json::value::string("Ok");
                if (x == 1)
                {
                    response["name"] = json::value::string("Error");
                }
                message.reply(status_codes::OK, response);
            }
            catch (std::exception &e)
            {
                std::cout << "Request Error";
                message.reply(status_codes::BadRequest);
            }
        });

}

void MicroserviceController::signupUser(http_request message) {


         message.extract_string().then([=](std::string request) {
            try
            {
                boost::property_tree::ptree pt;
                std::istringstream is(request);
                read_json(is, pt);

                DataBaseManagement::UserInformation userinfo{
                    pt.get<std::string>("email"),
                    pt.get<std::string>("password"),
                    pt.get<std::string>("name"),
                    pt.get<std::string>("lastName"),
                };
                json::value response; 
                if(data->SignUp(userinfo))response["message"] = json::value::string("succesful registration!");
                else response["message"] = json::value::string("account already exists!");
                message.reply(status_codes::OK, response);
            }
 
            catch (std::exception &e)
            {
                std::cout << "Request Error";
                message.reply(status_codes::BadRequest);
            }
        });

}


void MicroserviceController::signinUser(http_request message) {
	message.extract_string().then([=](std::string request) {
            try
            {
                boost::property_tree::ptree pt;
                std::istringstream is(request);
                read_json(is, pt);
                //data->ImagePersonBase64("chaker");
                signonInformation userinfo{
                    pt.get<std::string>("email"),
                    pt.get<std::string>("password"),
 
                };
                json::value response;
                cout<<userinfo.email<<userinfo.password<<endl;
                if(data->SignIn(userinfo.email,userinfo.password))response["message"] = json::value::string("access permitted!");
                else response["message"] = json::value::string("access deneid!");
                message.reply(status_codes::OK, response);
            }
 
            catch (std::exception &e)
            {
                std::cout << "Request Error";
                message.reply(status_codes::BadRequest);
            }
        });
    
}

/*
void MicroserviceController::getImage(http_request message) {
message.extract_string().then([=](std::string request) {
            try
            {
		

                boost::property_tree::ptree pt;
                std::istringstream is(request);
                read_json(is, pt);
                json::value response;
                response["image"] = json::value::string(data->ImagePersonBase64(pt.get<std::string>("name")));
                message.reply(status_codes::OK, response);

   		
	        
 
            }
            catch (std::exception &e)
            {
                std::cout << "Request Error";
                message.reply(status_codes::BadRequest);
            }
        });
}
*/


void MicroserviceController::getImage(http_request message) {
message.extract_string().then([=](std::string request) {
            try
            {
		
boost::property_tree::ptree pt;
                std::istringstream is(request);
                read_json(is, pt);
                json::value response;
		std::vector<string> images;
		images.push_back(data->ImagePersonBase64(pt.get<std::string>("name")));
                json::value listimages= web::json::value::array(images.size());
		int i=0;
		for (auto &n : images)
                {
                listimages.as_array()[i++] = web::json::value(n);
                }
	
                //response["image"] = json::value::string(data->ImagePersonBase64(pt.get<std::string>("name")));
		response["image"] = listimages;
                message.reply(status_codes::OK, response);





   		
	        
 
            }
            catch (std::exception &e)
            {
                std::cout << "Request Error";
                message.reply(status_codes::BadRequest);
            }
        });
}

void MicroserviceController::initRestOpHandlers() {
    _listener.support(methods::GET, std::bind(&MicroserviceController::handleGet, this, std::placeholders::_1));
    _listener.support(methods::PUT, std::bind(&MicroserviceController::handlePut, this, std::placeholders::_1));
    _listener.support(methods::POST, std::bind(&MicroserviceController::handlePost, this, std::placeholders::_1));
    _listener.support(methods::DEL, std::bind(&MicroserviceController::handleDelete, this, std::placeholders::_1));
    _listener.support(methods::PATCH, std::bind(&MicroserviceController::handlePatch, this, std::placeholders::_1));
}

void MicroserviceController::handleGet(http_request message) {
    auto path = requestPath(message);
    if (!path.empty()) {
        if (path[0] == "users" && path[1] == "signin") {
            auto response = json::value::object();
            response["version"] = json::value::string("1.0.0");
            response["status"] = json::value::string("ready for authentification: Use POST!");
            message.reply(status_codes::OK, response);
        }

    else if (!path.empty() && 
         path[0] == "users" && 
         path[1] == "getlist") { afficherListe(message);
    }
     
    
    else if (!path.empty() && 
         path[0] == "users" && 
         path[1] == "GetImage") {
		getImage(message);

         
    }

   else {
      message.reply(status_codes::NotFound);
   }
}
 
}

void MicroserviceController::handlePatch(http_request message) {
       
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::PATCH));
}

void MicroserviceController::handlePut(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::PUT));
}

void MicroserviceController::handlePost(http_request message)
{
    std::cout << "Receive POST request !" << std::endl;
    //Identfy request
    auto path = requestPath(message);
    if (!path.empty() &&
        path[0] == "users" &&
        path[1] == "identify")
    {
	identifierUser(message);
    }

    else if (!path.empty() &&
             path[0] == "users" &&
             path[1] == "Add")
    {
		addUser(message); 

    }

    else if (!path.empty() &&
             path[0] == "users" &&
             path[1] == "delete")
    { 
		deleteUser(message);
        

    }

    else if (!path.empty() && 
         path[0] == "users" && 
         path[1] == "signup")
    {
	
		signupUser(message);

    }

    else if (!path.empty() && 
         path[0] == "users" && 
         path[1] == "signin")
   {
		
		signinUser(message);


         }
    
    
    else
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::POST));
    }
}

void MicroserviceController::handleDelete(http_request message) {    
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::DEL));
}

void MicroserviceController::handleHead(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::HEAD));
}

void MicroserviceController::handleOptions(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::OPTIONS));
}

void MicroserviceController::handleTrace(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::TRCE));
}

void MicroserviceController::handleConnect(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::CONNECT));
}

void MicroserviceController::handleMerge(http_request message) {
    message.reply(status_codes::NotImplemented, responseNotImpl(methods::MERGE));
}

json::value MicroserviceController::responseNotImpl(const http::method & method) {
    auto response = json::value::object();
    response["serviceName"] = json::value::string("TELNET AI Mircroservice.");
    response["http_method"] = json::value::string(method);
    return response ;
}



