/**
  Authors:
    Mathias Paumgarten
    David Strauﬂ
**/

#ifndef PRODUCER_H
#define PRODUCER_H

#include "BoundedBuffer.h"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

class Producer {

  public:

    Producer(int port, BoundedBuffer *socketsBuffer);
    void run();

  private:

      unsigned int port;
      BoundedBuffer *socketsBuffer;

};

#endif;