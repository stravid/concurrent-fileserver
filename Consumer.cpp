#include "Consumer.h"

Consumer::Consumer(BoundedBuffer *socketBuffer) : sockets(socketBuffer) {}

void Consumer::run() {

    while (true) {

        tcp::socket *socket = sockets->get();

        char requestBuffer[1024];
		int requestSize = socket->receive(boost::asio::buffer(requestBuffer, 1024));

		std::stringstream httpResponse;
		std::stringstream httpRequest(std::string(requestBuffer, requestSize));

		//std::cout << httpRequest.str();
		std::string method, filename;

		httpRequest >> method >> filename;
		std::cout << "Requested file: " << filename << std::endl;

        filename.erase(0, 1);

        boost::filesystem::path path(filename.c_str());

        if (boost::filesystem::is_regular_file(path)) {

            httpResponse << "HTTP/1.0 200 OK\n";
		    httpResponse << "Server: FileServer/0.0.1\n";
		    httpResponse << "Content-Type: application/octet-stream\n";
            httpResponse << "Content-Length: " << boost::filesystem::file_size(path) << "\n\n";

            socket->send(boost::asio::buffer(httpResponse.str().c_str(), httpResponse.str().length()));

            std::ifstream file(filename.c_str(), std::ios::binary);

            unsigned int bufferSize = 1024;
            unsigned int currentPosition = 0;
            char* buffer = new char[bufferSize];

            while (!file.eof()) {
                memset(buffer, 0, bufferSize);
                file.read(buffer, bufferSize);

                currentPosition += (unsigned int) file.gcount(); 

                socket->send(boost::asio::buffer(buffer, bufferSize));
            }

            file.close();

            delete buffer;

        } else {

            std::string errorMessage = "<!DOCTYPE html><html><head><title>Error</title></head><body>The file " + filename + " does not exist!</body></html>";

            httpResponse << "HTTP/1.0 200 OK\n";
		    httpResponse << "Server: FileServer/0.0.1\n";
		    httpResponse << "Content-Type: text/html\n";
		    httpResponse << "Content-Length: " << errorMessage.length() << "\n\n";

            socket->send(boost::asio::buffer(httpResponse.str().c_str(), httpResponse.str().length()));

            socket->send(boost::asio::buffer(errorMessage.c_str(), errorMessage.length()));

        }
		
		socket->shutdown(tcp::socket::shutdown_both);
		socket->close();

        delete socket;
        
    }
}