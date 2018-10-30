#ifndef PROJECT_SHELF_H
#define PROJECT_SHELF_H

#define MAX_SHELF_CAPACITY 200

#include "WarehouseObjects.h"
#include <vector>
#include <list>
#include <iostream>
#include <string>

class Shelf
{

public:
	std::vector<Item> inventory;
	Coordinates shelfCoordinates;
	double shelfWeight;
	
	/**
	* Constructor- creates a new shelf
	* @param newCoord, coordinates of shelf in warehouse layout
	* @param capacity, maximum weight capacity for shelf (defaults to 200)
	*/
	Shelf(Coordinates newCoord, double capacity = MAX_SHELF_CAPACITY) : shelfCoordinates(newCoord) { shelfWeight = 0; }

	/**
	* Stores item into shelf
	* @param newItem, Item to store on the shelf 
	* @return true if the item is succesfully stored on the shelf 
	*/
	bool storeItem(Item newItem) {
		if (shelfWeight + newItem.itemWeight > MAX_SHELF_CAPACITY) {
			return false;
		}
		else {
			inventory.push_back(newItem);
			shelfWeight += newItem.itemWeight;
			return true;
		}
	}

	/**
	* Releases item from the shelf
	* @param removedItem, Item to remove from the shelf
	* @param newVector, new location of the item 
	* @param quantity, number of products to remove from the shelf 
	* @return true if the item is succesfully removed from the shelf
	*/
	bool releaseItem(Item removedItem, std::vector<Item> &newVector, int quantity) {
		return MoveItem::moveItem(inventory, newVector, removedItem.itemName, quantity);
	}
	
	/**
	* Location of shelf 
	* @return Coordinates of the shelf location 
	*/
	Coordinates shelfLocation() {
		return shelfCoordinates;
	}

	/**
	* Prints inventory - for testing purposes
	*/
	void printInventory() {
		for (Item item : inventory) {
			std::cout << item << '\n';
		}
	}

};


#endif // !PROJECT_SHELF_H
