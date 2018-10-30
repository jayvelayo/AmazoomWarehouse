/**
 * @file
 *
 * This contains the data structure for storing the warehouse inventory locally in memory.
 *
 */
#ifndef PROJECT_WAREHOUSE_INVENTORY_H
#define PROJECT_WAREHOUSE_INVENTORY_H

#include "WarehouseObjects.h"
#include "Shelf.h"
#include <vector>
#include <set>
#include <regex>
#include <utility>

#pragma region ItemEntries
class ItemEntry {

public:
	std::string itemName;
	int ID;
	double weight;
	int quantityOnHold;
	int quantityAvailable;
	double cost;
	std::vector<Coordinates> shelfLocations;

	/**
	* Constructor- create an ItemEntry with a name and quantity available
	* @param itemName_ name of item to create entry of 
	* @param quantityAvailable_ quantity of item 
	*/
	ItemEntry(std::string itemName_, int quantityAvailable_) : itemName(itemName_), quantityAvailable(quantityAvailable_) { quantityOnHold = 0; }
	
	/**
	* Constructor- create an ItemEntry with a name, quantity available, item ID, and item weight
	* @param item Item object of item in warehouse
	* @param quantity quantity of item
	*/
	ItemEntry(Item item, int quantity) { itemName = item.itemName; ID = item.itemID; weight = item.itemWeight; 
		quantityAvailable = quantity; quantityOnHold = 0; shelfLocations.push_back(Coordinates(0,0)); }
	
	/**
	* Constructor- create an ItemEntry with a name, quantity available, item ID, and item weight, and cost
	* @param itemName_ name of item to create entry of
	* @param quantityAvailable_ quantity of item
	* @param ID itemID of item
	* @param weight weight of item
	* @param cost cost of item 
	*/
	ItemEntry(std::string itemName_, int quantityAvailable_, int ID, double weight, int cost) : 
		itemName(itemName_), quantityAvailable(quantityAvailable_), ID(ID), weight(weight), cost(cost) {
		quantityOnHold = 0;
	}

	//for comparisons
	friend bool operator<(const ItemEntry& a, const ItemEntry& b) {
		if (a.ID < b.ID) {
			return true;
		}
		else if (a.ID > b.ID) {
			return false;
		}
		return a.ID < b.ID;
	}

	// equal-to operator for comparisons, both artist and title must match
	friend bool operator==(const ItemEntry& a, const ItemEntry& b) {
		return (a.ID == b.ID) && (a.itemName == b.itemName);
	}

	// not-equal-to operator for comparisons
	friend bool operator!=(const ItemEntry& a, const ItemEntry& b) {
		return !(a == b);
	}

	// overloaded stream operator for printing
	friend std::ostream& operator<<(std::ostream& os, ItemEntry& s) {
		os << s.itemName << "\t\t" << s.ID << "\t" << s.quantityAvailable << "\t\t\t" << s.quantityOnHold << "\t\t\t";
		bool first = true;
		for (Coordinates &coord : s.shelfLocations) {
			if (!first) { os << ", "; first = false; }
			os << coord;
		}
		return os;
	}

};


// Stores a list of items
class WarehouseInventory {
  // private vector
  std::vector<ItemEntry> inventory;

 public:

	 /** 
	 * Constructor - creates an empty warehouse inventory
	 */
	 WarehouseInventory() {};

	 /** 
	 * Constructor - creates a warehouse with a vector of item entries
	 * @param newEntries - vector of ItemEntry to store in warehouse inventory
	 */
	 WarehouseInventory(std::vector<ItemEntry> newEntries) : inventory(newEntries) {};

  /**
   * Adds a item to the warehouse inventory
   * @param item item info to add
   * @param quantity quantity of item to add 
   */
	 void add(Item& item, int quantity) {
		 bool found = false;
		 for (ItemEntry &entry : inventory) {
			 if (entry.itemName == item.itemName) {
				 entry.quantityAvailable += quantity;
				 found = true;
			 }
		 }
		 // make a new one if doesnt exist
		 if (!found) {
			 inventory.push_back(ItemEntry(item, quantity));
		 }
	 }

  /**
   * Adds item to the warehouse inventory
   * @param items item info to add
   * @return number of items added
   */
  void add(std::vector<std::pair<Item, int>>& itempair) {
    size_t count = 0;

    for (auto& item : itempair) {
		add(item.first, item.second);
    }
  }

  /**
   * Hold item on stock
   * @param itemID item's ID #
   * @param quantity number of reserved item
   * @return if successful
   */
  bool holdItem(int itemID, int quantity) {
	  bool success = false;
	  for (ItemEntry &entry : inventory) {
		  //search if item exists in database
		  if (entry.ID == itemID) {
			  if (entry.quantityAvailable >= quantity) {
				  entry.quantityAvailable -= quantity;
				  entry.quantityOnHold += quantity;
				  success = true;
				  break;
			  }
		  }
	  }
	  return success;
  }

  /**
   * Removes a item from the warehouse inventory
   * @param item item info to remove
   * @param quantityonhold number of items desired to remove from hold 
   * @param quantityavailable number of items desired to remove from inventory
   * @return true if removed, false if not in library
   */
  bool remove(Item& item, int quantityonhold, int quantityavailable = 0) {
	  bool success;
	  for (ItemEntry &entry : inventory) {
		  if (entry.itemName == item.itemName) {
			  if (entry.quantityAvailable < quantityavailable || entry.quantityOnHold < quantityonhold) {
				  success = false;
			  }
			  else {
				  entry.quantityOnHold -= quantityonhold;
				  entry.quantityAvailable -= quantityavailable;
				  success = true;
			  }
		  }
	  }
	  return success;
	
  }

  /**
   * Finds items in the database matching item name
   * @param name_regex item name regular expression 
   * @param itemID itemID associated with item 
   * @return set of items matching expression
   */
  std::vector<ItemEntry> find(const std::string& name_regex, int itemID) const {
    std::vector<ItemEntry> out;

    // compile regular expressions
    std::regex nregex(name_regex);

    // search through items for names matching search expression
    for (auto& entry : inventory) {
      if (std::regex_search(entry.itemName, nregex) || entry.ID == itemID) {
        out.push_back(entry);
      }
    }

    return out;
  }

  /**
  * Finds items in the database matching item ID (for warehouse manager)
  * @param itemID itemID associated with item
  * @return item matching expression, else returns -1
  */
  ItemEntry find_id(int itemID) const {

	  // search through items for names matching search expression
	  for (auto& entry : inventory) {
		  if (entry.ID == itemID) {
			  return entry; 
		  }
	  }
	  return ItemEntry("empty", -1);
  }

  /**
  * Prints the database
  */
  void printDatabase() {
	  std::cout << "Item Name\tItemID\tQuantity Available\tQuantity on Hold\tShelf locations\tItem Weight" << std::endl;
	  for (ItemEntry &entry : inventory) {
		  std::cout << entry << std::endl;
	  }

  }
  

  /**
   * Retrieves the unmodifiable list of items
   * @return internal set of items
   */
  std::vector<ItemEntry> &items() {
    return inventory;
  }
};

#pragma endregion ItemEntries

#pragma region OrderEntries
class OrderEntry {
public:
	const int orderNum;
	std::string status = "Confirmed";
	std::vector<ItemEntry> itemList;
	int shippingID;

	/**
	* Constructor - creates an OrderEntry with a order number and list of items
	* @param orderNum order number
	* @param items vector of ItemEntry 
	*/
	OrderEntry(int orderNum, std::vector<ItemEntry> items) : orderNum(orderNum), itemList(items) {}

	/**
	* Converts the vector of ItemEntry in an OrderEntry to a vector of Item
	* @param order, OrderEntry containing vector of ItemEntry to be modified 
	* @return out, equivalent vector of Item
	*/
	std::vector<Item> entrytoitem(OrderEntry order) {
		std::vector<Item> out;

		for (int i = 0; i < order.itemList.size(); i++) {
			Item item(order.itemList[i].itemName, order.itemList[i].ID, order.itemList[i].quantityAvailable, order.itemList[i].weight);
			out.push_back(item);
		}
		return out;
	}
};

class OrderList {
	std::vector<OrderEntry> orderList;
	int orderID = 1001;
public:

	/**
	* Constructor - creates an empty OrderList (vector of OrderEntry)
	*/
	OrderList() {};

	/**
	* Adds an order entry into the database
	* @param itemList the list of items the order contains
	* @return unique order ID to identify the order
	*/
	int addEntry(std::vector<ItemEntry> itemList) {
		int ID = orderID;
		orderList.push_back(OrderEntry(orderID, itemList));
		orderID++;
		return ID;
	}

	/**
	* Searches for a specific order using the order num
	* @param the ID of the order
	* @return an object of OrderEntry with the corresponding Id number
	*		  if it doesn't exist, returns an order with an ID of -1
	*/
	OrderEntry searchOrder(int orderNum) {
		for (auto &entry : orderList) {
			if (entry.orderNum == orderNum) {
				return entry;
			}
		}
		std::vector<ItemEntry> items;
		return OrderEntry(-1, items);

	}

	/**
	* Change the status of an existing order
	* @param orderNum the orderID
	* @param newStatus string of order status
	* @return true is successful, false if order doesn't exist
	*/
	bool changeStatus(int orderNum, std::string newStatus) {
		bool success = false;
		for (auto &entry : orderList) {
			if (entry.orderNum == orderNum) {
				entry.status = newStatus;
				success = true;
				break;
			}
		}
		return success;
	}


};

#pragma endregion OrderEntries

#endif //PROJECT_WAREHOUSE_INVENTORY_H
