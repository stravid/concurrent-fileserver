/**
  Authors:
    Mathias Paumgarten
    David Strauﬂ
**/

#ifndef CONSUMER_H
#define CONSUMER_H

#include "BoundedBuffer.h"
#include "ReaderWriterMutex.h"
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>

using boost::asio::ip::tcp;
using namespace boost::filesystem;

class Consumer {
  public:

    Consumer(BoundedBuffer *socketBuffer, std::map<std::string, ReaderWriterMutex*>* fileMutexes, boost::mutex* fileMutexesMapMutex);
    void run();

  private:
    BoundedBuffer *sockets;
    std::map<std::string, ReaderWriterMutex*>* fileMutexes;
    boost::mutex *fileMutexesMapMutex;
};

#endif