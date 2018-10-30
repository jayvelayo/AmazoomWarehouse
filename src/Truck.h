#ifndef PROJECT_INVENTORYTRUCK_H
#define PROJECT_INVENTORYTRUCK_H


#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <fstream>
#include <thread>

#include "WarehouseCommon.h"
#include "WarehouseObjects.h"
#include "Shelf.h"

class Truck {
protected:
	bool isDocked = false;
	bool isEmpty;
	const double truckMaxCapacity = 200.0;
	std::vector<Item> itemList;
	double currentWeight;
public:
	Truck(std::vector<Item> itemList_, bool isDocked_, bool isEmpty) : itemList(itemList_), isDocked(isDocked_), isEmpty(isEmpty) {
		currentWeight = 0;
		for (auto &item : itemList_) {
			currentWeight += item.itemQuantity*item.itemWeight;
		}
	}
};

class DeliveryTruck : protected Truck
{
public:
	/**
	* Constructor- creates an inventory truck
	* @param itemList_, list of items in the truck
	* @param isDocked_, true if the truck is currently docked
	* @param isEmpty, true if the truck contains no items
	*/
	DeliveryTruck(std::vector<Item> itemList_, bool isDocked_, bool isEmpty) : Truck(itemList_, isDocked_, isEmpty) {}

	/**
	* Load new item into the truck
	* @param newItem the new item to be added
	* return true if successful, false if not
	*/
	bool loadItem(Item newItem) {
		if (newItem.itemWeight*newItem.itemQuantity + currentWeight > truckMaxCapacity) {
			return false;
		}
		else {
			itemList.push_back(newItem);
			return true;
		}
	}

	/**
	* Checks if truck is docked
	* @return isDocked, true if truck is docked
	*/
	bool checkDock() {
		return isDocked;
	}
};

class InventoryTruck : protected Truck
{
public:
	
	/**
	* Constructor- creates an inventory truck
	* @param itemList_, list of items in the truck
	* @param isDocked_, true if the truck is currently docked
	* @param isEmpty, true if the truck contains no items 
	*/
	InventoryTruck(std::vector<Item> itemList_, bool isDocked_, bool isEmpty) : Truck(itemList_, isDocked_, isEmpty) {}

	Item unloadItem() {
		Item item = itemList.back();
		itemList.pop_back();
		
		return item;
	}

	/**
	* Checks if robot can succesfully carry item 
	* @param weight, current weight of the robot 
	*/
	bool peek(double weight) {
	 Item item = itemList.back();
	 if (weight + item.itemWeight > MAX_ROBOT_CAPACITY) {
		 return false;
	 }
	 else {
		 return true; 
	 }
	}

	/**
	* Checks if truck is docked 
	* @return isDocked, true if truck is docked
	*/
	bool checkDock() {
		return isDocked; 
	}

	/**
	* Checks if truck is empty
	* @return isEmpty, true if truck is empty
	*/
	bool checkEmpty() {
		return isEmpty;
	}
};


#endif // !PROJECT_INVENTORYTRUCK_H
