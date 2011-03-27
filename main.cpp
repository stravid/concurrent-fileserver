#include <iostream>
#include "BoundedBuffer.h"
#include "Producer.h"

using namespace std;

int main() {
    
    cout << "Initializing socket buffer" << endl;
    BoundedBuffer socketsBuffer(100);

    cout << "Initializing producer" << endl;
    Producer producer(80, &socketsBuffer);

    cout << "Starting server" << endl;
    //producer.run();

    return 0;
}
