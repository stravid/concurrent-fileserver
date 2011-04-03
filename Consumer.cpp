/**
  Authors:
    Mathias Paumgarten
    David Strauß
**/

#include "Consumer.h"

Consumer::Consumer(BoundedBuffer *socketBuffer) : sockets(socketBuffer) {}

void Consumer::run() {

    const int REQUEST_BUFFER_SIZE = 1024;

    while (true) {

        tcp::socket *socket = sockets->get();

        char requestBuffer[REQUEST_BUFFER_SIZE];
		int requestSize = socket->receive(boost::asio::buffer(requestBuffer, REQUEST_BUFFER_SIZE));

		std::stringstream httpResponse;
		std::stringstream httpRequest(std::string(requestBuffer, requestSize));

		std::string method, filename;

		httpRequest >> method >> filename;

        // Erase leading slash from filepath request: /index.html ==> index.html
        filename.erase(0, 1);

        if (method == "GET") {
		    std::cout << "Requested file: " << filename << std::endl;

            boost::filesystem::path path(filename.c_str());

            // If file exists on hard drive
            if (boost::filesystem::is_regular_file(path)) {

                httpResponse << "HTTP/1.0 200 OK\n";
		        httpResponse << "Server: FileServer/0.0.1\n";
		        httpResponse << "Content-Type: application/octet-stream\n";
                httpResponse << "Content-Length: " << boost::filesystem::file_size(path) << "\n\n";

                socket->send(boost::asio::buffer(httpResponse.str().c_str(), httpResponse.str().length()));

                std::ifstream file(filename.c_str(), std::ios::binary);

                unsigned int currentPosition = 0;
                char* buffer = new char[REQUEST_BUFFER_SIZE];

                // read the requested file chunkwise to not overload the RAM
                while (!file.eof()) {
                    memset(buffer, 0, REQUEST_BUFFER_SIZE);
                    file.read(buffer, REQUEST_BUFFER_SIZE);

                    currentPosition += (unsigned int) file.gcount(); 

                    socket->send(boost::asio::buffer(buffer, REQUEST_BUFFER_SIZE));
                }

                file.close();

                delete buffer;

            } else { 

                // If file could not be found - respond with error message
                std::string errorMessage = "<!DOCTYPE html><html><head><title>Error</title></head><body>The file " + filename + " does not exist!</body></html>";

                httpResponse << "HTTP/1.0 200 OK\n";
		        httpResponse << "Server: FileServer/0.0.1\n";
		        httpResponse << "Content-Type: text/html\n";
		        httpResponse << "Content-Length: " << errorMessage.length() << "\n\n";

                socket->send(boost::asio::buffer(httpResponse.str().c_str(), httpResponse.str().length()));

                socket->send(boost::asio::buffer(errorMessage.c_str(), errorMessage.length()));

            }

        } else if (method == "POST") { 

            std::cout << "Upload file: " << filename << std::endl;

            int bytesTotal;
            std::string headerBuffer, content, boundary;

            // Find Content-Length in header
            httpRequest.seekg(httpRequest.str().find("Content-Length:") + 16);
            httpRequest >> headerBuffer;
            sscanf_s(headerBuffer.c_str(), "%d", &bytesTotal);

            int boundaryPosition = httpRequest.str().find("boundary=");
            if(boundaryPosition != -1){
                httpRequest.seekg(boundaryPosition + 9);
                httpRequest >> boundary;
            }

            // Find Blank Lines after header before body
            int firstBlankLine = httpRequest.str().find("\r\n\r\n");

            // Fetch content that was already recieved from first socket->recieve
            content.append(httpRequest.str().substr(firstBlankLine + 4));

            int bytesLoaded = content.size();
            
            // Fetch rest of content by 1024 Byte chunks
            while(bytesLoaded <= bytesTotal){
                memset(requestBuffer, 0, REQUEST_BUFFER_SIZE);
                socket->receive(boost::asio::buffer(requestBuffer, REQUEST_BUFFER_SIZE));
                content.append(requestBuffer);
                bytesLoaded += REQUEST_BUFFER_SIZE;
            }
            
            // crop boundary
            if(boundaryPosition != -1){
                content.erase(0, content.find("\r\n\r\n") + 4);
                int boundaryEndPosition = content.find("--" + boundary);
                //content.erase(content.find("\r\n--" + boundary));
            }

            std::ofstream uploadFile;
            uploadFile.open(filename, std::ios::binary);

		    if(uploadFile.is_open()) {
			    uploadFile << content;
			    uploadFile.close();
            }
            
        }
		
		socket->shutdown(tcp::socket::shutdown_both);
		socket->close();

        delete socket;
        
    }
}