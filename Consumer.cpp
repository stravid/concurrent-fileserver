#include "Consumer.h"

Consumer::Consumer(BoundedBuffer *socketBuffer) : sockets(socketBuffer) {}

void Consumer::run() {

    while (true) {

        tcp::socket *socket = sockets->get();

        char requestBuffer[1024];
		int requestSize = socket->receive(boost::asio::buffer(requestBuffer, 1024));

		
		std::stringstream httpRequest(std::string(requestBuffer, requestSize));
		//std::cout << httpRequest.str();
		std::string method, filename;

		
		httpRequest >> method >> filename;
		std::cout << "Requested file: " << filename << std::endl;

        filename.erase(0, 1);

        std::stringstream stream;
        std::ifstream file(filename);
        std::string line;

        if (file.is_open()) {

            while (file.good()) {
                getline(file, line);
                stream << line << "\n";
            }

            file.close();
        }

        std::string payload(stream.str());
		std::stringstream httpResponse;

        int payloadSize = payload.length();

		httpResponse << "HTTP/1.0 200 OK\n";
		httpResponse << "Server: FileServer/0.0.1\n";
		httpResponse << "Content-Type: application/octet-stream\n";
		httpResponse << "Content-Length: " << payloadSize << "\n\n";
		
		socket->send(boost::asio::buffer(httpResponse.str().c_str(), httpResponse.str().length()));

		socket->send(boost::asio::buffer(payload.c_str(), payloadSize));
		socket->shutdown(tcp::socket::shutdown_both);
		socket->close();

        delete socket;
    }
}