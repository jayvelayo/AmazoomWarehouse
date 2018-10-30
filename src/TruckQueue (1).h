#ifndef PROJECT_TRUCKQUEUE 
#define PROJECT_TRUCKQUEUE

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <cpen333\thread\semaphore.h>

#include "WarehouseObjects.h"

class tCommand {
	Order order;
	int truckNumber;

public:
	/**
	* Constructor- creates a new truck command
	* @param list, order for robot to deliver/retrieve
	* @param number, truck number to deliver to or retrieve from 
	*/
	tCommand(Order list, int number) :  order(list), truckNumber(number) {}
	tCommand(Order order) : order(order) {}


	/**
	* Gets order
	*/
	Order getOrder() {
		return order;
	}
	

	//gets truck number
	int getTruckNum() {
		return truckNumber;
	}
};

class TruckQueue {
	std::deque<tCommand> truckQueue;
	std::mutex mutex_;
	std::condition_variable cv_;
	cpen333::thread::semaphore truckSemaphore;

public:
	/**
	* Constructor- creates a new empty truck queue
	*/
	TruckQueue() : truckSemaphore(0) { }
	//TruckQueue(std::deque<tCommand> truckqueue) : truckQueue(truckqueue) {};


	/**
	* Adds to truck queue (instructions for robots)
	* @param command, command to be added to truck queue
	*/
	void addToTQueue(tCommand& command)
	{
		mutex_.lock();
		truckQueue.push_front(command);
		mutex_.unlock();
		truckSemaphore.notify();
	}


	/**
	* Removes from truck queue (instructions for robots)
	* @return command to be added to be executed by robots
	*/
	tCommand removeFromTQueue(void) 
	{
		tCommand command(Order(-1));
		std::unique_lock <std::mutex> lock(mutex_);
		bool success = false;
		success = truckSemaphore.try_wait();
		// get first item in queue
		if (success) {
			command = truckQueue.front();
			truckQueue.pop_front();
			lock.unlock();
		}	

		return command;
	}
};

#endif //PROJECT__TRUCKQUEUE 