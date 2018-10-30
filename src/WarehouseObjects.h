#ifndef PROJECT_WAREHOUSE_OBJECTS 
#define PROJECT_WAREHOUSE_OBJECTS

#include <iostream>
#include <vector>
#include <string>

class Coordinates {

public:
	int XCoordinates;
	int YCoordinates;

	/**
	* Constructor- create an empty set of coordaintes
	*/
	Coordinates() : XCoordinates(), YCoordinates() {}

	/**
	* Constructor- create a set of coordinates 
	* @param XCoord x-coordinate position (row of warehouse)
	* @param YCoord y-coordinate position (column of warehouse)
	*/
	Coordinates(int XCoord, int YCoord) :
		XCoordinates(XCoord), YCoordinates(YCoord) {}

	// overloaded stream operator for printing
	friend std::ostream& operator<<(std::ostream& os, Coordinates& coordinates) {
		os << "(" << coordinates.XCoordinates << ", " << coordinates.YCoordinates << ")";
		return os;
	}
};

class Item {

public:
	int itemQuantity;
	std::string itemName;
	int itemID;
	double itemWeight;

	/**
	* Constructor- create an Item
	* @param name, name of item 
	* @param ID, id of item
	* @param quantity, quantity of item
	* @param weight, weight of item 
	*/
	Item(std::string name, int ID, int quantity, double weight) : itemName(name), itemID(ID), itemQuantity(quantity), itemWeight(weight) {}

	// less-than operator for comparisons, sort by item id
	friend bool operator<(const Item& a, const Item& b) {
		return a.itemID < b.itemID;
	}

	// equal-to operator for comparisons
	friend bool operator==(const Item& a, const Item& b) {
		return (a.itemID == b.itemID);
	}

	// not-equal-to operator for comparisons
	friend bool operator!=(const Item& a, const Item& b) {
		return !(a.itemID == b.itemID);
	}

	// overloaded stream operator for printing
	friend std::ostream& operator<<(std::ostream& os, const Item& s) {
		os << "Item id: " << s.itemID << "| ";
		return os;
	}
};


class Order {
	int orderNum;

public:
	std::vector<Item> orderList;
	
	/**
	* Constructor- create an empty Order
	*/
	Order(): orderNum(), orderList() {}

	/**
	* Constructor- create an order containing a list of items 
	* @param list,  list of items 
	*/
	Order(std::vector<Item> list) : orderList(list) {}

	/**
	* Constructor- create an order with an order number 
	* @param num, order number associated with order 
	*/
	Order(int num) : orderNum(num) {}

	/**
	* Constructor- create an order with an order number and order list
	* @param list, list of items associated with order 
	* @param num, order number associated with order
	*/
	Order(std::vector<Item> list, int num) : orderList(list), orderNum(num) {}

	/**
	* Returns the order's number
	* @return order number
	*/
	int getOrderNum() {
		return orderNum; 
	}
	
};

namespace MoveItem {

	/**
	* moveItem : moves an item from one vector to another. Once the item is empty from the old vector, it is deleted in the vector
	* PARAM:
	*	fromVector	: the vector being transferred FROM
	*	toVector	: vector of items being transferred TO. if the item doesn't exist in the new vector, it will be created
	*	itemName	: name of the item (case sensitive)
	*	quantity	: how many items are being transferred
	* RETURN:
	*	bool	: true if transfer is successful
	*			: false otherwise (when item doesnt exist in fromVector or quantity being transferred is too large)
	* POST:
	*	if fromVector's item would have zero quantity, it will be deleted from the vector
	*/

	bool moveItem(std::vector<Item> &fromVector, std::vector<Item> &toVector, std::string itemName, int quantity) {
		bool success = false;

		auto it = fromVector.begin();
		for (auto &elem : fromVector) {

			if (elem.itemName == itemName && elem.itemQuantity >= quantity) {
				//check if item exists in tovector
				bool alreadyExist = false;
				for (auto &it : toVector) {
					if (it.itemName == itemName) {
						it.itemQuantity += quantity;
						alreadyExist = true;
					}
				}
				if (!alreadyExist) {
					Item newItem = elem;
					newItem.itemQuantity = quantity;
					toVector.push_back(newItem);

				}

				elem.itemQuantity -= quantity;

				//erase if quantity = 0
				if (elem.itemQuantity == 0) {
					fromVector.erase(it);
				}
				success = true;
				break;
			}
			std::next(it); //next iterator
		}

		return success;

	}
};

#endif // !PROJECT_WAREHOUSE_OBJECTS 

