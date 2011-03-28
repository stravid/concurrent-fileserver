/**
  Authors:
    Mathias Paumgarten
    David Strauﬂ
**/

#include "BoundedBuffer.h"

void BoundedBuffer::put(tcp::socket *socket) {

    freeElements.wait();

    {
        boost::mutex::scoped_lock lock(socketsQueueMutext);
		socketsQueue.push_back(socket);
    }

    availableElements.post();

}

tcp::socket* BoundedBuffer::get() {

    tcp::socket *element;

	availableElements.wait();

	{
		boost::mutex::scoped_lock lock(socketsQueueMutext);
		element = socketsQueue.front();
		socketsQueue.pop_front();
	}

	freeElements.post();

	return element;

}