/**
  Authors:
    Mathias Paumgarten
    David Strauß
**/

#include "Producer.h"

Producer::Producer(int port, BoundedBuffer *socketsBuffer) : 
    port(port), 
    socketsBuffer(socketsBuffer) {}

void Producer::run() {

    boost::asio::io_service ioService;
    tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), port));

    while (true) {
  
        tcp::socket *socket = new tcp::socket(ioService);
		acceptor.accept(*socket);

        socketsBuffer->put(socket);
        
        std::cout << "Connected socket. Remote Endpoint: " << socket->remote_endpoint() << std::endl;
    }
    
}