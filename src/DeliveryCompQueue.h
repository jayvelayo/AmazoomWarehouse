#ifndef PROJECT_DELIVERYCOMPQUEUE 
#define PROJECT_DELIVERYCOMPQUEUE

#include <iostream>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include "WarehouseObjects.h"

class DeliveryCompQueue {
	std::deque<Order> deliveryCompQueue; 
	std::mutex mutex_;
	std::condition_variable cv_;


public:
	/**
	* Constructor- creates a new delivery queue
	*/
	DeliveryCompQueue() : deliveryCompQueue(), mutex_(), cv_() {}

	/**
	* Adds to delivery queue
	* @param Order, picked order to be added to delivery queue by robots 
	*/
	void addToDCQueue(Order Order)
	{
		mutex_.lock();
		deliveryCompQueue.push_back(Order);
		mutex_.unlock();
		cv_.notify_one();
	}

	/**
	* Removes an order from the delivery queue
	* @return next order in queue to be moved to truck delivery queue
	*/
	Order removeFromDCQueue(void) 
	{
		std::unique_lock <std::mutex> lock(mutex_);
		cv_.wait(lock, [this] {return !deliveryCompQueue.empty(); });
		// get first item in queue
		Order command = deliveryCompQueue.front(); 
		deliveryCompQueue.pop_front();
		lock.unlock();

		return command;
	}
};

#endif //PROJECT__DELIVERYCOMPQUEUE 