/**
  Authors:
    Mathias Paumgarten
    David Strauﬂ
**/

#include <iostream>
#include <boost/thread.hpp>
#include "BoundedBuffer.h"
#include "Producer.h"
#include "Consumer.h"

using namespace std;

int main() {
    
    cout << "Initializing socket buffer" << endl;
    BoundedBuffer socketsBuffer(100);
    
    cout << "Initializing producer" << endl;
    Producer producer(8080, &socketsBuffer);
    
    Consumer* consumers[20];
    
    cout << "Initializing multiple consumer:" << endl << endl;
    for (int i = 0; i < 20; i++) {
        cout << "    Starting consumer " << (i + 1) << endl;

        consumers[i] = new Consumer(&socketsBuffer);
        boost::thread consumerThread(boost::bind(&Consumer::run, consumers[i]));
    }

    cout << endl << "Starting producer on port 8080" << endl;
    producer.run();

    return 0;
}
