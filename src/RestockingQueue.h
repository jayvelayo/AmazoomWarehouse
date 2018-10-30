#ifndef PROJECT_RESTOCKINGQUEUE 
#define PROJECT_RESTOCKINGQUEUE

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <cpen333\thread\semaphore.h>
#include "WarehouseObjects.h"

class trCommand {
	int loadingDockNumber;

public:
	/**
	* Constructor- creates a new truck command
	* @param command, dock for truck 
	* @param item, item for robot to retrieve
	* @param number, truck number to retrieve from
	*/
	trCommand( int number) : loadingDockNumber(number) {}

	int getDockNumber() {
		return loadingDockNumber;
	}
};

class RestockingQueue {
	std::deque<trCommand> restockingQueue; 
	std::mutex mutex_;
	cpen333::thread::semaphore restockSemaphore;

public:
	/**
	* Constructor- creates a new empty truck queue
	*/
	RestockingQueue() : restockingQueue(), mutex_(), restockSemaphore(0) {}


	/**
	* Adds to truck queue (instructions for robots)
	* @param command, command to be added to truck queue
	*/
	void addToTQueue(trCommand command)
	{
		mutex_.lock();
		restockingQueue.push_front(command);
		mutex_.unlock();
		restockSemaphore.notify();
	}


	/**
	* Removes from truck queue (instructions for robots)
	* @return command to be added to be executed by robots
	*/
	trCommand removeFromTQueue(void)
	{
		trCommand command(-1);
		std::unique_lock <std::mutex> lock(mutex_);
		bool success = restockSemaphore.try_wait();
		if (success) {
			// get first item in queue
			command = restockingQueue.front();
			restockingQueue.pop_front();

		}
		else {
			command = trCommand(-1);
		}
		lock.unlock();

		return command;
	}
};

#endif //PROJECT__RESTOCKINGQUEUE 