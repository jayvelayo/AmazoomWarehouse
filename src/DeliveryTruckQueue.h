#ifndef PROJECT_DELIVERYTRUCKQUEUE 
#define PROJECT_DELIVERYTRUCKQUEUE

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <cpen333\thread\semaphore.h>
#include "WarehouseObjects.h"


class tCommand {
	Order order;
	int loadingDockNumber;

public:
	/**
	* Constructor- creates a new truck command
	* @param command, dock for truck
	* @param list, order for robot to deliver
	* @param number, truck number to deliver to 
	*/
	tCommand(Order list, int number) : order(list), loadingDockNumber(number) {}

	int getDockNumber() {
		return loadingDockNumber;
	}

	Order getOrder() {
		return order;
	}
};

class DeliveryTruckQueue {
	std::deque<tCommand> deliveryTruckQueue; 
	std::mutex mutex_;
	cpen333::thread::semaphore delTSemaphore;


public:
	/**
	* Constructor- creates a new delivery queue
	*/
	DeliveryTruckQueue() : deliveryTruckQueue(), mutex_(), delTSemaphore(0) {}

	/**
	* Adds to delivery queue
	* @param Order, picked order to be added to delivery queue
	*/
	void addToDTQueue(tCommand command)
	{
		mutex_.lock();
		deliveryTruckQueue.push_back(command);
		mutex_.unlock();
		delTSemaphore.notify();
	}

	/**
	* Removes an order from the delivery queue
	* @return next order in queue to be delivered to trucks by robots
	*/
	tCommand removeFromDTQueue(void) 
	{
		tCommand command(Order(-1),-1);
		std::unique_lock <std::mutex> lock(mutex_);
		bool success = delTSemaphore.try_wait();
		if (success) {
			// get first item in queue
			command = deliveryTruckQueue.front();
			deliveryTruckQueue.pop_front();

		}
		else {
			
		}
		lock.unlock();

		return command;
	}
};

#endif //PROJECT__DELIVERYTRUCKQUEUE 