/**
 * @file
 *
 * The Amazoom Client connects to a remote server and provides an interface
 * for online shopping on the server database
 *
 */

#include "WarehouseInventory.h"
#include "JsonUserClientApi.h"

#include <cpen333/process/socket.h>

#include <iostream>
#include <limits>

static const char CLIENT_PRINT = '1';
static const char CLIENT_SEARCH = '2';
static const char CLIENT_ADD = '3';
static const char CLIENT_ORDER = '4';
static const char CLIENT_CANCEL = '5';
static const char CLIENT_QUIT = '6';

// print menu options
void print_menu() {

  std::cout << "=========================================" << std::endl;
  std::cout << "=            AMAZOOM  MENU              =" << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << " (1) Print Shopping Cart" << std::endl;
  std::cout << " (2) Search Amazoom Catalog"<< std::endl;
  std::cout << " (3) Add Product to Shopping Cart" << std::endl;
  std::cout << " (4) Place Order"  << std::endl;
  std::cout << " (5) Cancel Order" << std::endl;
  std::cout << " (6) Quit " << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << "Enter number: ";
  std::cout.flush();

}

//print Order
void do_print(std::vector<ItemEntry> &cart) {
	double total = 0;
	std::cout << "Item name:" << "\t" << "item ID:" <<"\t" << "Quantity:" << "\t" << "Cost:" << std::endl;
	for (auto item : cart) {
		std::cout << item.itemName << "\t" << item.ID << "\t\t" << item.quantityAvailable << "\t\t" << "$" << item.cost << std::endl;
		total += item.cost*item.quantityAvailable;
	}
	std::cout << "Total = $" << total << std::endl;
}

// search for item in database
void do_search(UserClientApi &api) {
	std::string itemName;
	int itemID;

	// collect regular expressions for song search
	std::cout << std::endl << "Search for item by name or ID" << std::endl;
	std::cout << "Item Name: ";
	std::getline(std::cin, itemName);
	std::cout << "Product ID (optional):";
	std::cin >> itemID;


	// send search message and wait for response
	SearchMessage msg(itemName, itemID);
	if (api.sendMessage(msg)) {
		// get response
		std::unique_ptr<Message> msgr = api.recvMessage();
		SearchResponseMessage& resp = (SearchResponseMessage&)(*msgr);

		if (resp.status == MESSAGE_STATUS_OK) {
			std::cout << std::endl << "   Results:" << std::endl;
			for (const auto& entry : resp.results) {
				std::cout << "Item name:" << entry.itemName << "\tProduct ID: " << entry.ID << "\tAvailable: " << entry.quantityAvailable << "\tCost: $" << entry.cost << std::endl;
			}
		}
		else {
			std::cout << "No item found!" << resp.info << std::endl;
		}
	}

	std::cout << std::endl;
}

// add a item to an order over the remote server
void do_add(UserClientApi &api, std::vector<ItemEntry> &cart) {

  std::string item_name_regex;
  int productID;
  int quantity;

  // collect artist and title
  std::cout << std::endl << "Add Product to Shopping Cart" << std::endl;
  std::cout << "   Product Name: ";
  std::getline(std::cin, item_name_regex);
  std::cout << "   Product ID: ";
  std::cin >> productID;
  std::cout << "   Quantity: ";
  std::cin >> quantity;


  // send message to server and wait for response
  AddMessage msg(item_name_regex, productID, quantity);
  if (api.sendMessage(msg)) {
    // get response
    std::unique_ptr<Message> msgr = api.recvMessage();
    AddResponseMessage& resp = (AddResponseMessage&)(*msgr);

    if (resp.status == MESSAGE_STATUS_OK) {
      std::cout << std::endl << "   \"" << productID << "\" added successfully." << std::endl;
	  
	  for (auto &entry : resp.results) {
		  if (cart.size() == 0) {
			  entry.quantityAvailable = quantity;
			  cart.push_back(entry);
		  }
		  else 
		  {
			  bool flag = false; 

			  for (auto &itemincart : cart) {
				  // just add quantity if item is already in cart
				  if (itemincart.ID == entry.ID) {
					  itemincart.quantityAvailable += quantity;
					  flag = true; 
				  }
			  }
			  if (flag == false) {

				  entry.quantityAvailable = quantity;
				  cart.push_back(entry);
			  }
		  }
	  }
	  std::cout << "\n\n";
    } else {
		std::cout << "Do you mean these items?" << std::endl;
		for (auto &entry : resp.results) {
			std::cout << "Product name: " << entry.itemName << " Product ID: " << entry.ID << " Available: " <<  entry.quantityAvailable << std::endl;
	  }
    }
  }

  std::cout << std::endl;
}

void do_order(UserClientApi &api, std::vector<ItemEntry> cart) {
	if (cart.size() != 0) {
		char answer;
		std::cout << "You are about to buy the following items: " << std::endl;
		do_print(cart);
		std::cout << "Are you sure? (Y/N): ";
		std::cin >> answer;

		if (answer == 'N') {
			std::cout << "Order not placed." << std::endl;
		}
		else if (answer == 'Y') {
			//send command
			ConfirmOrder msg = ConfirmOrder(cart);
			if (api.sendMessage(msg)) {
				// get response
				std::unique_ptr<Message> msgr = api.recvMessage();
				ConfirmOrderResponseMessage& resp = (ConfirmOrderResponseMessage&)(*msgr);

				if (resp.status == MESSAGE_STATUS_OK) {
					cart.clear();
					std::cout << "Thank for shopping with us! Your order number is: " << resp.orderNum << std::endl;
					std::cout << "Please keep this number for your reference." << std::endl;
					std::cout << "Thank you!" << std::endl;		 
				}
				else {
					std::cout << "Error: Your order has not been placed " << resp.orderNum << std::endl;
					std::cout << "Please contact customer support." << std::endl;
				}
			}
		}
		else {
			std::cout << "Error: Your order has not been placed. Invalid Command" << std::endl;
		};
	}
	else
	{
		std::cout << "Your cart is empty!" << std::endl;
	}
}

void do_cancel(UserClientApi &api) {
	int orderNum;
	std::cout << "   Order Number: ";
	std::cin >> orderNum;

	CancelOrder msg(orderNum);
	if (api.sendMessage(msg)) {
		// get response
		std::unique_ptr<Message> msgr = api.recvMessage();
		CancelOrderResponseMessage& resp = (CancelOrderResponseMessage&)(*msgr);

		if (resp.status == MESSAGE_STATUS_OK) {
			std::cout << "Your order, order number " << orderNum << " has been succesfully cancelled" << std::endl;
		}
		else {
			std::cout << "Your order, order number " << orderNum << " is unabled to be cancelled." << std::endl;
			std::cout << "Your order has already been cancelled or is currently being delivered " << std::endl;
		}
	}
}

void sayGoodbye(UserClientApi &api) {
	GoodbyeMessage msg;
	api.sendMessage(msg);
}


int main() {

  // start client
  cpen333::process::socket socket("localhost", MUSIC_LIBRARY_SERVER_PORT);
  std::cout << "Client connecting...";
  std::cout.flush();

  // if we open the socket successfully, continue
  if (socket.open()) {
    std::cout << "connected." << std::endl;

    // create API handler
    JsonUserClientApi api(std::move(socket));

	//create shopping cart
	std::vector<ItemEntry> shoppingCart;

    // keep reading commands until the user quits
    char cmd = 0;
    while (cmd != CLIENT_QUIT) {
      print_menu();

      // get menu entry
      std::cin >> cmd;
      // gobble newline
      std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');

      switch(cmd) {
        case CLIENT_PRINT:
          do_print(shoppingCart);
          break;
        case CLIENT_SEARCH:
          do_search(api);
          break;
        case CLIENT_ADD:
          do_add(api, shoppingCart);
          break;
		case CLIENT_ORDER:
		  do_order(api, shoppingCart);
		  shoppingCart.clear();
		  break;
		case CLIENT_CANCEL:
			do_cancel(api);
			break;
        case CLIENT_QUIT:
			sayGoodbye(api);
          break;
        default:
          std::cout << "Invalid command number " << cmd << std::endl << std::endl;
      }

      cpen333::pause();
    }
  } else {
    std::cout << "failed." << std::endl;
  }

  return 0;
}