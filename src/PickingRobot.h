#ifndef PROJECT_ROBOT_H
#define PROJECT_ROBOT_H

#include <cpen333/thread/thread_object.h>
#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <iostream>
#include <thread>
#include <random>

#include "Shelf.h"
#include "WarehouseObjects.h"
#include "PickupQueue.h"
#include "DeliveryQueue.h"
#include "TruckQueue.h"
#include "safe_printf.h"
#include "WarehouseCommon.h"

/**
* The Robot grabs orders from the pickupqueue, searches shelves 
* for products to fulfill order, then adds the fulfilled orders to a 
* new queue for the delivery robots to delivery
*/
class Robot : public cpen333::thread::thread_object {
	cpen333::process::shared_object<SharedData> memory_;
	cpen333::process::mutex mutex_;
	
	Coordinates currentPosition;
	std::vector<Item> holdingItems;
	bool isFull;

	PickupQueue& pickup_;
	DeliveryQueue& delivery_;
	TruckQueue & truck_; 
	int id_;

	Order order;
	Order POISON_ORDER = order; 
	
	Coordinates moveToCoordinates(Coordinates newCoordinates) {
		currentPosition = newCoordinates;
	}

public:
	/**
	* Constructor- create a new picking robot
	* @param id the picking robot's id
	* @param pickup queue to read orders from
	* @param delivery queue to add completed orders to
	*/
	Robot(int id, PickupQueue& pickup, DeliveryQueue& delivery, TruckQueue& truckQueue) :

		id_(id), pickup_(pickup), delivery_(delivery), truck_(truckQueue), memory_(WAREHOUSE_MEMORY_NAME), mutex_(WAREHOUSE_MUTEX_NAME) {
		Coordinates coordinates(memory_->rinfo.startx, memory_->rinfo.starty);
		isFull = false;
	}
	
	/**
	* Returns the robot id
	* @return the robot's id
	*/
	int id() {
		return id_;
	}

	/**
	* Returns the robot's position in the warehouse
	* @return Coordinates of robot's current position
	*/
	Coordinates getPosition() {
		return currentPosition;
	}

	/**
	* Get a specfiic quantity of an item from the shelf 
	* @param item Item to be removed from the shelf
	* @param shelf Shelf to remove item from 
	* @param quantity number of items to remove from shelf 
	*/

	void getItem(Item item, Shelf& shelf, int quantity) {
		moveToCoordinates(shelf.shelfLocation());

		shelf.releaseItem(item, holdingItems, quantity);
	}
	
	/**
	* Algorithm for moving robot around maze and picking items
	* off the shelves and storing into their holding list
	* @param order Order to be fulfilled
	* @param memory_ shared memory for updating inventory
	*/
	void go(Order order, cpen333::process::shared_object<SharedData>& memory_) {

		Coordinates currentPos = getPosition();
		
		int c = currentPos.YCoordinates;
		int r = currentPos.XCoordinates;
		int prev_row = r;
		int prev_col = c; 
		
		while (memory_->winfo.maze[c][r] != EXIT_CHAR && memory_->quit == false)
		{
			if (memory_->winfo.maze[c][r + 1] == ROBOT_PATH_CHAR)
			{
				if (r + 1 != prev_row) {
					prev_row = r;
					r = r + 1;
					currentPosition.XCoordinates = r;
					pickup(c, r, order, memory_);
				}

			}
			else if (memory_->winfo.maze[c][r - 1] == ROBOT_PATH_CHAR)
			{
				if (r - 1 != prev_row) {
					prev_row = r;
					r = r - 1;
					currentPosition.XCoordinates = r;
					pickup(c, r, order, memory_);
				}

			}
			else if (memory_->winfo.maze[c + 1][r] == ROBOT_PATH_CHAR)
			{
				if (c + 1 != prev_col) {
					prev_col = c;
					c = c + 1;
					currentPosition.YCoordinates = c;
					pickup(c, r, order, memory_);
				}

			}
			else if (memory_->winfo.maze[c - 1][r] == ROBOT_PATH_CHAR)
			{
				if (c - 1 != prev_col) {
					prev_col = c;
					c = c - 1;
					currentPosition.YCoordinates = c;
					pickup(c, r, order, memory_);
				}

			}
		}

	}

	/**
	* Find's the shelf located at Coordinates [r, c]
	* @param r row in the warehouse layout 
	* @param c column in the warehouse layout 
	* @param memory_ shared memory for accessing shelf coordinates 
	* @return shelf at corresponding location [r,c]
	*/
	Shelf findShelf(int c, int r, cpen333::process::shared_object<SharedData>& memory_) {

		Coordinates coord(r, c);

		for (int i = 0; i < memory_->winfo.Shelves.size(); i++) {
			if (memory_->winfo.Shelves[i].shelfCoordinates.XCoordinates == coord.XCoordinates && memory_->winfo.Shelves[i].shelfCoordinates.YCoordinates == coord.YCoordinates) {
				return memory_->winfo.Shelves[i];
			}
		}
	}

	/**
	* Obtains product from the shelf and into robot holding items list 
	* if there is enough product in stock to fulfill the order 
	* @param r row in the warehouse layout
	* @param c column in the warehouse layout
	* @param memory_ shared memory for accessing shelf inventory list
	*/
	void pickup(int c, int r, Order order, cpen333::process::shared_object<SharedData>& memory_) {
		std::vector<Item> list = order.getOrderList();
		
		if (memory_->winfo.maze[c][r + 1] == SHELF_CHAR)
		{
			Shelf shelf = findShelf(c, (r+1), memory_);
			for (int i = 0; i < list.size(); i++) {
				std::string name = list[i].getItemName(); 
				for (int j = 0; j < shelf.inventory.size(); j++) {
					std::string inventory_name = list[j].getItemName();
					if (name == inventory_name) {
						shelf.releaseItem(list[i], holdingItems, list[i].itemQuantity);
					}
				}
				
			}

			if (list == holdingItems) {
				isFull = true;
			}
		}

		else if (memory_->winfo.maze[c][r - 1] == SHELF_CHAR)
		{
			Shelf shelf = findShelf(c, (r-1), memory_);
			for (int i = 0; i < list.size(); i++) {
				std::string name = list[i].getItemName();
				for (int j = 0; j < shelf.inventory.size(); j++) {
					std::string inventory_name = list[j].getItemName();
					if (name == inventory_name) {
						shelf.releaseItem(list[i], holdingItems, list[i].itemQuantity);
					}
				}

			}
			if (list == holdingItems) {
				isFull = true;
			}

		}
		else if (memory_->winfo.maze[c + 1][r] == SHELF_CHAR)
		{
			Shelf shelf = findShelf((c+1), r, memory_);
			for (int i = 0; i < list.size(); i++) {
				std::string name = list[i].getItemName();
				for (int j = 0; j < shelf.inventory.size(); j++) {
					std::string inventory_name = list[j].getItemName();
					if (name == inventory_name) {
						shelf.releaseItem(list[i], holdingItems, list[i].itemQuantity);
					}
				}

			}

			if (list == holdingItems) {
				isFull = true;
			}

		}
		else if (memory_->winfo.maze[c - 1][r] == SHELF_CHAR)
		{
			Shelf shelf = findShelf((c-1), r, memory_);
			for (int i = 0; i < list.size(); i++) {
				std::string name = list[i].getItemName();
				for (int j = 0; j < shelf.inventory.size(); j++) {
					std::string inventory_name = list[j].getItemName();
					if (name == inventory_name) {
						shelf.releaseItem(list[i], holdingItems, list[i].itemQuantity);
					}
				}

			}
			if (list == holdingItems) {
				isFull = true;
			}
		}
	}

	int main(void)
	{
		safe_printf("Robot %d started\n", id_);
		PickingRobot raphael(id_, pickup_, delivery_, truck_);
		memory_->rinfo.nrobots = memory_->rinfo.nrobots + 1; 

		int Try = 2;

		Order order = pickup_.removeFromPQueue();
		int Num = order.getOrderNum();
		int poisonNum = POISON_ORDER.getOrderNum();


		while (Num != poisonNum && memory_->magic == 1234) {

			if (Try == 1)
			{
				// process order
				safe_printf("Robot %d starting order {%d}\n", id_, Num);
				raphael.go(order, memory_);
				safe_printf("Robot %d completed order {%d}\n", id_, Num);

				// add to those to serve
				delivery_.addToDQueue(order);

				//move robot back to the start 
				moveToCoordinates(Coordinates(memory_->rinfo.startx, memory_->rinfo.starty));
				// next order
				Order order = pickup_.removeFromPQueue();
			}

			//if warehouse is notified that delivery truck has arrived 
			else if (Try == 2) {
				
				//get an order from the queue 
				Order order = delivery_.removeFromDQueue();
				
				safe_printf("Robot %d moving order {%d} to truck number {%d}\n", id_, truckNum);

				raphael.go(order, memory_);
				safe_printf("Robot %d completed order {%d}\n", id_, Num);

				// add to those to serve
				delivery_.addToDQueue(order);

				//move robot back to the start 
				moveToCoordinates(Coordinates(memory_->rinfo.startx, memory_->rinfo.starty));
				// next order
				Order order = pickup_.removeFromPQueue();

			}
			//if warehouse is notified that restocking truck has arrived 
			else if (Try == 3) {

			}

		}
	
		memory_->rinfo.nrobots = memory_->rinfo.nrobots + 1;
		safe_printf("Robot %d finishing for the day\n", id_);

	}
};

#endif //PROJECT_ROBOT_H