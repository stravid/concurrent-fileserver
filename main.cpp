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
    Producer producer(80, &socketsBuffer);


    cout << "Initializing multiple consumer:" << endl << endl;
    for (int i = 0; i < 20; i++) {
        cout << "    Starting consumer " << (i + 1) << endl;

        Consumer consumer(&socketsBuffer);
        boost::thread consumerThread(boost::bind(&Consumer::run, &consumer));
    }

    cout << endl << "Starting producer" << endl;
    producer.run();

    return 0;
}
