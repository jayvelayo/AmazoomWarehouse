#ifndef PROJECT_ROBOT_H
#define PROJECT_ROBOT_H

#include <cpen333/thread/thread_object.h>
#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <iostream>
#include <thread>
#include <random>
#include <math.h>

#include "WarehouseCommon.h"
#include "Shelf.h"
#include "Warehouse.h"
#include "WarehouseObjects.h"
#include "PickupQueue.h"
//#include "DeliveryQueue.h"
#include "TruckQueue.h"
#include "safe_printf.h"


/**
* The Robot grabs orders from the pickupQueue, searches shelves 
* for products to fulfill order, then adds the fulfilled orders to a 
* new queue for the delivery robots to delivery. The Robot also grabs orders from the truckQueue
* and takes them to a specified truck for delivery, or retrieve items from specific trucks and add them to the warehouse
*/
class Robot : public cpen333::thread::thread_object {
	cpen333::process::shared_object<SharedData> memory_;
	cpen333::process::mutex mutex_;
	
	Coordinates currentPosition;
	std::vector<Item> holdingItems;
	bool isFull;

	
	PickupQueue& pickup_;
	//DeliveryQueue& delivery_;
	TruckQueue & truck_; 
	Warehouse &warehouse;
	int id_;

	Order order;
	Order POISON_ORDER = order; 
	
	void moveToCoordinates(Coordinates newCoordinates) {
		currentPosition = newCoordinates;
	}

	void updateCoordinates(Coordinates currentCoordinates) {
		std::lock_guard<decltype(mutex_)> mylock(mutex_);
		memory_->rinfo.rloc[id_ - 1][COL_IDX] = currentCoordinates.YCoordinates;
		memory_->rinfo.rloc[id_ - 1][ROW_IDX] = currentCoordinates.XCoordinates;
	}

	void moveToDock(int docknum) {
		int r, c;
		r = currentPosition.XCoordinates;
		c = currentPosition.YCoordinates;
		int prev_row = r;
		int prev_col = c;
		double dock_r, dock_c;

		
		{
			std::lock_guard<decltype(mutex_)> mylock(mutex_);
			dock_c = memory_->winfo.docks.dloc[docknum][COL_IDX];
			dock_r = memory_->winfo.docks.dloc[docknum][ROW_IDX];
		}

		int rdir = (dock_r - r) / (abs(r - dock_r)); // + dir = moving towards the dock
		int cdir = (dock_c-c) / (abs(c - dock_c));

		while (r != dock_r && c != dock_c) {
			if (memory_->winfo.maze[c][r + rdir] != WALL_CHAR && 
				memory_->winfo.maze[c][r + rdir] != SHELF_CHAR && (r + 1) != prev_row)
			{
				prev_row = r;
				prev_col = c;
				r = r + rdir;
				currentPosition.XCoordinates = r;
				currentPosition.YCoordinates = c;
			}
			else if (memory_->winfo.maze[c + cdir][r] != WALL_CHAR && 
					memory_->winfo.maze[c+cdir][r] != SHELF_CHAR && (c + 1) != prev_col)
			{
				prev_row = r;
				prev_col = c;
				c = c + cdir;
				currentPosition.XCoordinates = r;
				currentPosition.YCoordinates = c;
			}
			else if (memory_->winfo.maze[c][r - rdir] != WALL_CHAR && 
					memory_->winfo.maze[c][r - rdir] != SHELF_CHAR && (r - 1) != prev_row)
			{
				prev_row = r;
				prev_col = c;
				r = r - rdir;
				currentPosition.XCoordinates = r;
				currentPosition.YCoordinates = c;
			}
			else if (memory_->winfo.maze[c - cdir][r] != WALL_CHAR && 
					memory_->winfo.maze[c - cdir][r] != SHELF_CHAR && (c - 1) != prev_col)
			{
				prev_row = r;
				prev_col = c ;
				c = c - cdir;
				currentPosition.XCoordinates = r;
				currentPosition.YCoordinates = c;
			}
			updateCoordinates(currentPosition);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

		}

	}

	void unloadTruck(int dockNumber) {

	}


public:
	/**
	* Constructor- create a new robot
	* @param id the  robot's id
	* @param pickup queue to read orders from
	* @param delivery queue to add completed orders to
	*/
	Robot(int id, Warehouse &warehouse, PickupQueue& pickup) :

		id_(id), warehouse(warehouse), pickup_(pickup), memory_(WAREHOUSE_MEMORY_NAME), mutex_(WAREHOUSE_MUTEX_NAME) {
		currentPosition = Coordinates(memory_->rinfo.startx, memory_->rinfo.starty);
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
	void follow_path(Order &order, cpen333::process::shared_object<SharedData>& memory_) {

		Coordinates currentPos = getPosition();
		
		int c = currentPos.YCoordinates;
		int r = currentPos.XCoordinates;
		int prev_row = r;
		int prev_col = c; 
		
		while (memory_->winfo.maze[c][r] != EXIT_CHAR && memory_->quit == false)
		{
			memory_->rinfo.rloc[id_-1][COL_IDX] = c;
			memory_->rinfo.rloc[id_-1][ROW_IDX] = r;
			if (memory_->winfo.maze[c][r + 1] == ROBOT_PATH_CHAR && (r + 1) != prev_row)
			{
				prev_row = r;
				prev_col = c;
				r = r + 1;
				currentPosition.XCoordinates = r;
				pickup(c, r, order, memory_);
			}
			else if (memory_->winfo.maze[c][r - 1] == ROBOT_PATH_CHAR && ((r - 1) != prev_row))
			{
				prev_row = r;
				prev_col = c;
				r = r - 1;
				currentPosition.XCoordinates = r;
				pickup(c, r, order, memory_);
			}
			else if (memory_->winfo.maze[c + 1][r] == ROBOT_PATH_CHAR && ((c + 1) != prev_col))
			{
				prev_row = r;
				prev_col = c;
				c = c + 1;
				currentPosition.YCoordinates = c;
				pickup(c, r, order, memory_);
			}
			else if (memory_->winfo.maze[c - 1][r] == ROBOT_PATH_CHAR && ((c - 1) != prev_col))
			{
				prev_row = r;
				prev_col = c;
				c = c - 1;
				currentPosition.YCoordinates = c;
				pickup(c, r, order, memory_);

			}
			else if (memory_->winfo.maze[c][r + 1] == EXIT_CHAR || memory_->winfo.maze[c+1][r] == EXIT_CHAR ||
				memory_->winfo.maze[c][r - 1] == EXIT_CHAR || memory_->winfo.maze[c-1][r] == EXIT_CHAR) {
				r = memory_->rinfo.endx;
				c = memory_->rinfo.endy;
				currentPosition = Coordinates(r, c);
			}
			updateCoordinates(currentPosition);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

		for (auto &shelf : warehouse.Shelves) {
			if (shelf.shelfCoordinates.YCoordinates == r && shelf.shelfCoordinates.XCoordinates == c) {
				return shelf;
			}
		}
		return Shelf(coord);
	}

	/**
	* Obtains product from the shelf and into robot holding items list 
	* if there is enough product in stock to fulfill the order 
	* @param r row in the warehouse layout
	* @param c column in the warehouse layout
	* @param memory_ shared memory for accessing shelf inventory list
	*/
	void pickup(int c, int r, Order &order, cpen333::process::shared_object<SharedData>& memory_) {
		std::vector<Item> list = order.orderList;
		
		if (memory_->winfo.maze[c][r + 1] == SHELF_CHAR)
		{
			Shelf shelf = findShelf(c, (r+1), memory_);
			for (auto &item : list) {
				std::string name = item.itemName;
				for (auto &inventory_item : shelf.inventory) {
					std::string inventory_name = inventory_item.itemName;
					if (name == inventory_name) {
						shelf.releaseItem(item, holdingItems, item.itemQuantity);
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
			for (auto &item : list) {
				std::string name = item.itemName;
				for (auto &inventory_item : shelf.inventory) {
					std::string inventory_name = inventory_item.itemName;
					if (name == inventory_name) {
						shelf.releaseItem(item, holdingItems, item.itemQuantity);
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
			for (auto &item : list) {
				std::string name = item.itemName;
				for (auto &inventory_item : shelf.inventory) {
					std::string inventory_name = inventory_item.itemName;
					if (name == inventory_name) {
						shelf.releaseItem(item, holdingItems, item.itemQuantity);
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
			for (auto &item : list) {
				std::string name = item.itemName;
				for (auto &inventory_item : shelf.inventory) {
					std::string inventory_name = inventory_item.itemName;
					if (name == inventory_name) {
						shelf.releaseItem(item, holdingItems, item.itemQuantity);
					}
				}

			}
			if (list == holdingItems) {
				isFull = true;
			}
		}
	}

	/**
	* Finds shelf location to place item 
	* @param item item to be placed on the shelves
	* @param winfo warehouse info stored in shared memory for accessing shelf inventory list
	* @return coordinates able to hold item 
	*/
	Coordinates placeItem(Item item, WarehouseInfo winfo) {

		// fill in random placements for items
		std::default_random_engine rnd(
			(unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<size_t> rdist(0, winfo.rows);
		std::uniform_int_distribution<size_t> cdist(0, winfo.cols);


		bool heavy;

			// generate until on shelf space
			size_t r, c;
			do {
				r = rdist(rnd);
				c = cdist(rnd);
				Shelf shelf_ = findShelf(c, r, memory_);
				heavy = false;
				if (shelf_.shelfWeight + (item.itemWeight)*(item.itemQuantity) > MAX_SHELF_CAPACITY)
				{
					heavy = true;
				}
			} while (winfo.maze[c][r] != SHELF_CHAR || heavy == true);

			Coordinates coord(r, c);
			return coord;
	}

	int main(void)
	{
		safe_printf("Robot %d started\n", id_);

		updateCoordinates(currentPosition);


		int truckNum = 0;

		
		int Num = order.getOrderNum();
		int poisonNum = -1;


		while (memory_->quit && memory_->magic == 1234) {
			//TODO : Fix poison order
			
			/* 
			Robot loops checking different queues:
			1. Pick up queue
			2. Truck queue
			3. Delivery queue
			*/

			// 1. Pick up Queue
			Order order = pickup_.removeFromPQueue();
			if (order.getOrderNum() != -1) 
			{
				// process order
				safe_printf("Robot %d starting order {%d}\n", id_, order.getOrderNum());
				follow_path(order, memory_);
				safe_printf("Robot %d completed order {%d}\n", id_, order.getOrderNum());

				// add to those to serve
				//delivery_.addToDQueue(order);
				//TODO: update database!!!!!!!!!!

				//move robot back to the start 
				//moveToCoordinates(Coordinates(memory_->rinfo.startx, memory_->rinfo.starty));
				// next order
				Order order = pickup_.removeFromPQueue();
				int poisonNum = POISON_ORDER.getOrderNum();
			}

			
			// 2. Restock queue			
			tCommand command = truck_.removeFromTQueue();
			if (command.getOrder().getOrderNum() != -1)
			{
				truckNum = command.getTruckNum();
				safe_printf("Robot %d moving order {%d} to truck number {%d}\n", id_, truckNum);
				//follow_path(order, memory_);
				safe_printf("Robot %d completed order {%d}\n", id_, Num);

				// add to those to serve
				//delivery_.addToDQueue(order);

				//move robot back to the start 
				moveToCoordinates(Coordinates(memory_->rinfo.startx, memory_->rinfo.starty));
				// next order
				order = pickup_.removeFromPQueue();

			}

			// 3. Delivery queue
			/*
			Order deliveryOrder = delivery_.removeFromDQueue();
			if (deliveryOrder.getOrderNum() != -1) {
				truckNum = 1;
				safe_printf("Robot %d moving order {%d} to truck number {%d}\n", id_, truckNum);
				moveToDock(truckNum);
				safe_printf("Robot %d completed order {%d}\n", id_, Num);
			}*/

		}
	
		memory_->rinfo.nrobots = memory_->rinfo.nrobots + 1;
		safe_printf("Robot %d finishing for the day\n", id_);
		return 0;
	}
};

#endif //PROJECT_ROBOT_H