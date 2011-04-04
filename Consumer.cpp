/**
  Authors:
    Mathias Paumgarten
    David Strauﬂ
**/

#include "Consumer.h"

Consumer::Consumer(BoundedBuffer *socketBuffer, std::map<std::string, ReaderWriterMutex*>* fileMutexes, boost::mutex* fileMutexesMapMutex) : 
    sockets(socketBuffer),
    fileMutexes(fileMutexes),
    fileMutexesMapMutex(fileMutexesMapMutex) {}

void Consumer::run() {

    const int REQUEST_BUFFER_SIZE = 1024;

    while (true) {

        tcp::socket *socket = sockets->get();

        // recieving the first chunk of the request
        char requestBuffer[REQUEST_BUFFER_SIZE];
		int requestSize = socket->receive(boost::asio::buffer(requestBuffer, REQUEST_BUFFER_SIZE));

		std::stringstream httpResponse;
		std::stringstream httpRequest(std::string(requestBuffer, requestSize));

		std::string method, filename;

		httpRequest >> method >> filename;

        // Erase leading slash from filepath request: /index.html ==> index.html
        filename.erase(0, 1);

        // seperate GET from POST Requests
        if (method == "GET") {
		    std::cout << "Requested file: " << filename << std::endl;

            // creating a mutext for this file, if it doesn't already exist
            {
                boost::mutex::scoped_lock lock(*fileMutexesMapMutex);
                if(fileMutexes->find(filename) == fileMutexes->end()){
                    fileMutexes->insert(std::pair<std::string, ReaderWriterMutex*>(filename, new ReaderWriterMutex()));
                }
            }

            // setting the readerslock to ensure that the file will not be deleted while read.
            fileMutexes->find(filename)->second->readerLock();

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

            fileMutexes->find(filename)->second->readerUnlock();

        } else if (method == "POST") { 

            std::cout << "Upload file: " << filename << std::endl;

            long recieveSize;
            char buffer[REQUEST_BUFFER_SIZE];
            std::string receiveString;

            receiveString.append(httpRequest.str());

            // read the whole request from socket. The while will read as until the size of the chunk that is read is
            // smaller than the REQUEST_BUFFER_SIZE. This means, that it is the last chunk.
            do {

                recieveSize = socket->receive(boost::asio::buffer(buffer, REQUEST_BUFFER_SIZE));
                receiveString.append(std::string(buffer, recieveSize));

            } while (REQUEST_BUFFER_SIZE == recieveSize);

            // Browsers sent a boundry. If so you have to cut it out from the data.
            size_t boundaryPosition = receiveString.find("boundary=");

            // Start of the actual data nd end of the header.
            size_t headerEndPosition = receiveString.find("\r\n\r\n") + 4;
    
            if (boundaryPosition != std::string::npos && boundaryPosition < headerEndPosition) {
        
                // If the browser sent a boundry -> the data starts at the second empty line.
                headerEndPosition = receiveString.find("\r\n\r\n", headerEndPosition) + 4;

                // Removeing the boundry from the end of the file.
                receiveString.erase(receiveString.find_last_of('\n', receiveString.size() - 3) + 1);
            }
    
            // Removing the header from the file.
            receiveString = receiveString.substr(headerEndPosition);

            if (receiveString.size() < 5) {

                // An empty request delets the file.
                boost::filesystem::path path(filename);
                remove(path);

                fileMutexes->erase(filename);

            } else {

                // Write in the file, only if its not currently read.

                {
                    boost::mutex::scoped_lock lock(*fileMutexesMapMutex);
                    if(fileMutexes->find(filename) == fileMutexes->end()){
                        fileMutexes->insert(std::pair<std::string, ReaderWriterMutex*>(filename, new ReaderWriterMutex()));
                    }
                }

                fileMutexes->find(filename)->second->writerLock();

                std::ofstream uploadFile;

                uploadFile.open(filename, std::ios::binary);
                uploadFile << receiveString;

                uploadFile.close();

                fileMutexes->find(filename)->second->writerUnlock();
            }
        }
		
		socket->shutdown(tcp::socket::shutdown_both);
		socket->close();

        delete socket;
        
    }
}