/**
 * @file
 *
 * This file contains all message-related objects, independent of the specific API
 *
 * This middle layer allows us to abstract away many of the communication details,
 * allowing us to focus on the core functional implementation.
 *
 */
#ifndef PROJECT_LIBRARY_MESSAGES_H
#define PROJECT_LIBRARY_MESSAGES_H

#include "WarehouseObjects.h"
#include "WarehouseInventory.h"
#include <string>

/**
 * Types of messages that can be sent between client/server
 */
enum MessageType {
	ADD,
	ADD_RESPONSE,
	REMOVE,
	REMOVE_RESPONSE,
	SEARCH,
	SEARCH_RESPONSE,
	CONFIRM_ORDER,
	CONFIRM_ORDER_RESPONSE,
	CANCEL_ORDER,
	CANCEL_ORDER_RESPONSE,
	GOODBYE,
	UNKNOWN
};

// status messages for response objects
#define MESSAGE_STATUS_OK "OK"
#define MESSAGE_STATUS_ERROR "ERROR"

/**
 * Base class for messages
 */
class Message {
 public:
  virtual MessageType type() const = 0;
};

/**
* Base class for response messages
*/
class ResponseMessage : public Message {
 public:
  const std::string status;
  const std::string info;
  ResponseMessage(const std::string& status,
                  const std::string& info = "") :
      status(status), info(info){}

};

/**
 * Add an item to the Order
 */
class AddMessage : public Message {
 public:
  std::string itemName;
  int itemQuantity;
  int itemID;

  AddMessage(std::string name, int ID, int quantity)  : itemName(name), itemID(ID), itemQuantity(quantity) {}

  MessageType type() const {
    return MessageType::ADD;
  }
};

/**
 * Response to adding a Item to the library
 */
class AddResponseMessage : public ResponseMessage {
 public:
	 std::vector<ItemEntry> results;

  AddResponseMessage(std::vector<ItemEntry> results_, std::string status, std::string info ="") :
      ResponseMessage(status, info), results(results_) {}

  MessageType type() const {
    return MessageType::ADD_RESPONSE;
  }
};


/**
 * Search the library using regular expressions
 */
class SearchMessage : public Message {
 public:
  std::string itemName;
  int itemID;

  SearchMessage(std::string name, int ID) :
      itemName(name), itemID(ID) {}

  MessageType type() const {
    return MessageType::SEARCH;
  }
};

/**
 * Response to a library search
 */
class SearchResponseMessage : public ResponseMessage {
 public:
  const std::vector<ItemEntry> results;

  SearchResponseMessage(std::vector<ItemEntry>& results,
    const std::string& status, const std::string& info = "" ) :
      ResponseMessage(status, info), results(results) {}

  MessageType type() const {
    return MessageType::SEARCH_RESPONSE;
  }
};

/**
 * Confirm order
 */
class ConfirmOrder : public Message {
public:
	std::vector<ItemEntry> order;

	ConfirmOrder(std::vector<ItemEntry> order_) : order(order_) {}

	MessageType type() const {
		return MessageType::CONFIRM_ORDER;
	}
};

/**
* Response to confirmation of order
*/
class ConfirmOrderResponseMessage : public ResponseMessage {
public:
	int orderNum;

	ConfirmOrderResponseMessage(int num, const std::string& status, const std::string& info = "") :
		ResponseMessage(status, info), orderNum(num) {}

	MessageType type() const {
		return MessageType::CONFIRM_ORDER_RESPONSE;
	}
};

/**
* CANCEL order
*/
class CancelOrder : public Message {
public:
	int orderNum;

	CancelOrder(int num) : orderNum(num) {}

	MessageType type() const {
		return MessageType::CANCEL_ORDER;
	}
};

/**
* Response message to order cancellation
*/
class CancelOrderResponseMessage : public ResponseMessage {
public:

	CancelOrderResponseMessage(const std::string& status, const std::string& info = "") :
		ResponseMessage(status, info) {}

	MessageType type() const {
		return MessageType::CANCEL_ORDER_RESPONSE;
	}
};

/**
 * Goodbye message
 */
class GoodbyeMessage : public Message {
 public:
  MessageType type() const {
    return MessageType::GOODBYE;
  }
};

#endif //PROJECT_LIBRARY_MESSAGES_H
