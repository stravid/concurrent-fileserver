#include <iostream>
#include <boost/thread.hpp>
#include "BoundedBuffer.h"
#include "Producer.h"
#include "Consumer.h"

using namespace std;

int main() {
    
    cout << "Initializing buffer" << endl;
    BoundedBuffer socketsBuffer(100);

    cout << "Initializing producer" << endl;
    Producer producer(80, &socketsBuffer);

    cout << "Initializing consumer" << endl;
    Consumer consumer(&socketsBuffer);

    boost::thread consumerThread(boost::bind(&Consumer::run, &consumer));

    cout << "Starting producer" << endl;
    producer.run();

    return 0;
}
