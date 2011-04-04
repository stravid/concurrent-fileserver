/**
  Authors:
    Mathias Paumgarten
    David Strauﬂ
**/

#ifndef READERWRITERMUTEX_H
#define READERWRITERMUTEX_H

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/thread/mutex.hpp>

class ReaderWriterMutex {

  public:
	ReaderWriterMutex();

	void readerLock();
	void readerUnlock();
	void writerLock();
	void writerUnlock(); 

  private:
	int readers;
	boost::mutex readersMutex;
	boost::interprocess::interprocess_semaphore allowedWriters;

};


#endif