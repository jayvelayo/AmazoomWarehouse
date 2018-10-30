#ifndef PROJECT_PICKUPQUEUE 
#define PROJECT_PICKUPQUEUE

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include "WarehouseObjects.h"
#include <cpen333\thread\semaphore.h>

class PickupQueue {
	std::deque<Order> pickupQueue;
	std::mutex mutex_;
	std::condition_variable cv_;
	cpen333::thread::semaphore pickSemaphore;
public:
	/**
	* Constructor- creates a new picking queue
	*/
	PickupQueue() : pickupQueue(), mutex_(), cv_(), pickSemaphore(0) {}

	/**
	* Adds to picking queue 
	* @param Order, confirmed order to be added to picking queue 
	*/
	void addToPQueue(Order& Order)
	{
		mutex_.lock();
		pickupQueue.push_back(Order);
		mutex_.unlock();
		pickSemaphore.notify();
	}
	/**
	* Removes an order from the picking queue
	* @return next order in queue to be picked by robots
	*/
	Order removeFromPQueue(void)
	{
		Order command; 
		std::unique_lock <std::mutex> lock(mutex_);
		bool success = pickSemaphore.try_wait();
		if (success) {
			// get first item in queue
			command = pickupQueue.front();
			pickupQueue.pop_front();
			
		}
		else {
			command = Order(-1);
		}
		lock.unlock();

		return command;
	}
};

#endif //PROJECT__PICKUPQUEUE
