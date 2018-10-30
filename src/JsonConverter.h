/**
 * @file
 *
 * This file provides all the JSON encoding/decoding functionality
 *
 */

#ifndef PROJECT_JSON_H
#define PROJECT_JSON_H

#include "Message.h"

#include "json.hpp"   // json parsing
//#include <json.hpp>   // json parsing

#include <vector>
#include <memory>     // for std::unique_ptr
#include <set>

// convenience alias for json
using JSON = nlohmann::json;

// types of messages
#define MESSAGE_ADD "add"
#define MESSAGE_ADD_RESPONSE "add_response"
#define MESSAGE_SEARCH "search"
#define MESSAGE_SEARCH_RESPONSE "search_response"
#define MESSAGE_CONFIRM_ORDER "confirm"
#define MESSAGE_CONFIRM_ORDER_RESPONSE "confirm_response"
#define MESSAGE_CANCEL "cancel"
#define MESSAGE_CANCEL_RESPONSE "cancel_response"
#define MESSAGE_GOODBYE "goodbye"

// other keys
#define MESSAGE_TYPE "msg"
#define MESSAGE_STATUS "status"
#define MESSAGE_INFO "info"
#define MESSAGE_SEARCH_RESULTS "results"
#define MESSAGE_ITEM "item"
#define MESSAGE_ITEM_ID "item_ID"
#define MESSAGE_ITEM_NAME "item_name_regex"
#define MESSAGE_ITEM_QUANTITY "item_quantity"
#define MESSAGE_ITEM_PRICE "item_price"
#define MESSAGE_ITEM_WEIGHT "item_weight"
#define MESSAGE_ORDER_NUM "order_num"
#define MESSAGE_CART "cart"


/**
 * Handles all conversions to and from JSON
 */
class JsonConverter {
 public:
  /**
   * Converts the item to a JSON object
   * @param item Item to jsonify
   * @return JSON object representation
   */
  static JSON toJSON(Item item) {
    JSON j;
	j[MESSAGE_ITEM_NAME] = item.itemName;
	j[MESSAGE_ITEM_ID] = item.itemID;
	j[MESSAGE_ITEM_QUANTITY] = item.itemQuantity;
	j[MESSAGE_ITEM_WEIGHT] = item.itemWeight;

    return j;
  }

  /**
  * Converts the entry to a JSON object
  * @param entry ItemEntry to jsonify
  * @return JSON object representation
  */
  static JSON toJSON(ItemEntry entry) {
	  JSON j;
	  j[MESSAGE_ITEM_NAME] = entry.itemName;
	  j[MESSAGE_ITEM_ID] = entry.ID;
	  j[MESSAGE_ITEM_PRICE] = entry.cost;
	  j[MESSAGE_ITEM_QUANTITY] = entry.quantityAvailable;
	  j[MESSAGE_ITEM_WEIGHT] = entry.weight;
	  return j;
  }

  /**
  * Converts a vector of itemEntry to a JSON array of objects
  * @param entires vector of ItemEntry to jsonify
  * @return JSON array representation
  */
  static JSON toJSON(std::vector<ItemEntry> entries) {
	  JSON j;
	  for (auto& entry : entries) {
		  j.push_back(toJSON(entry));
	  }
	  return j;
  }

  /**
  * Converts the entry to a JSON object
  * @param order Order to jsonify
  * @return JSON object representation
  */
  static JSON toJSON(Order order) {
	  JSON j;
	  for (auto&item : order.orderList) {
		  j.push_back(toJSON(item));
	  }
	  return j;
  }

  /**
   * Converts a vector of items to a JSON array of objects
   * @param songs vector of items to jsonify
   * @return JSON array representation
   */
  static JSON toJSON(const std::vector<Item> &items) {
    JSON j;
    for (auto& item : items) {
      j.push_back(toJSON(item));
    }
    return j;
  }

  /**
   * Converts an "add" message to a JSON object
   * @param add message
   * @return JSON object representation
   */
  static JSON toJSON(const AddMessage &add) {
    JSON j;
    j[MESSAGE_TYPE] = MESSAGE_ADD;
    j[MESSAGE_ITEM_NAME] = add.itemName;
	j[MESSAGE_ITEM_QUANTITY] = add.itemQuantity;
	j[MESSAGE_ITEM_ID] = add.itemID;
    return j;
  }

  /**
   * Converts an "add" response message to a JSON object
   * @param add_response message
   * @return JSON object representation
   */
  static JSON toJSON(const AddResponseMessage &add_response) {
    JSON j;
    j[MESSAGE_TYPE] = MESSAGE_ADD_RESPONSE;
	j[MESSAGE_SEARCH_RESULTS] = toJSON(add_response.results);
    j[MESSAGE_STATUS] = add_response.status;
    j[MESSAGE_INFO] = add_response.info;
    return j;
  }

  /**
   * Converts a "search" message to a JSON object
   * @param search message
   * @return JSON object representation
   */
  static JSON toJSON(const SearchMessage &search) {
    JSON j;
    j[MESSAGE_TYPE] = MESSAGE_SEARCH;
	j[MESSAGE_ITEM_NAME] = search.itemName;
	j[MESSAGE_ITEM_ID] = search.itemID;
    return j;
  }

  /**
   * Converts a "search" response message to a JSON object
   * @param search_response message
   * @return JSON object representation
   */
  static JSON toJSON(const SearchResponseMessage &search_response) {
    JSON j;
    j[MESSAGE_TYPE] = MESSAGE_SEARCH_RESPONSE;
    j[MESSAGE_STATUS] = search_response.status;
    j[MESSAGE_INFO] = search_response.info;
    j[MESSAGE_SEARCH_RESULTS] = toJSON(search_response.results);
    return j;
  }

  /**
  * Converts a "confirm" message to a JSON object
  * @param search message
  * @return JSON object representation
  */
  static JSON toJSON(const ConfirmOrder &confirm) {
	  JSON j;
	  j[MESSAGE_TYPE] = MESSAGE_CONFIRM_ORDER;
	  j[MESSAGE_CART] = toJSON(confirm.order);
	  return j;
  }

  /**
  * Converts a "confirm" response message to a JSON object
  * @param search_response message
  * @return JSON object representation
  */
  static JSON toJSON(const ConfirmOrderResponseMessage &confirm_response) {
	  JSON j;
	  j[MESSAGE_TYPE] = MESSAGE_CONFIRM_ORDER_RESPONSE;
	  j[MESSAGE_INFO] = confirm_response.info;
	  j[MESSAGE_STATUS] = confirm_response.status;
	  j[MESSAGE_ORDER_NUM] = confirm_response.orderNum;
	  return j;
  }

  /**
  * Converts a "cancel" message to a JSON object
  * @param search message
  * @return JSON object representation
  */
  static JSON toJSON(const CancelOrder &cancel) {
	  JSON j;
	  j[MESSAGE_TYPE] = MESSAGE_CANCEL;
	  j[MESSAGE_ORDER_NUM] = cancel.orderNum;
	  return j;
  }

  /**
  * Converts a "cancel" response message to a JSON object
  * @param search_response message
  * @return JSON object representation
  */
  static JSON toJSON(const CancelOrderResponseMessage &cancel_response) {
	  JSON j;
	  j[MESSAGE_TYPE] = MESSAGE_CANCEL_RESPONSE;
	  j[MESSAGE_INFO] = cancel_response.info;
	  j[MESSAGE_STATUS] = cancel_response.status;

	  return j;
  }

  /**
   * Converts a "goodbye" message to a JSON object
   * @param goodbye message
   * @return JSON object representation
   */
  static JSON toJSON(const GoodbyeMessage &goodbye) {
    JSON j;
    j[MESSAGE_TYPE] = MESSAGE_GOODBYE;
    return j;
  }

  /**
   * Converts a message to a JSON object, automatically detecting the type
   * @param message
   * @return JSON object representation, {"status"="ERROR", "info"=...} if not recognized
   */
  static JSON toJSON(const Message &msg) {

    switch(msg.type()) {
      case ADD: {
        return toJSON((AddMessage &) msg);
      }
      case ADD_RESPONSE: {
        return toJSON((AddResponseMessage &) msg);
      }
      case SEARCH: {
        return toJSON((SearchMessage &) msg);
      }
      case SEARCH_RESPONSE: {
        return toJSON((SearchResponseMessage &) msg);
      }
	  case CONFIRM_ORDER: {
		  return toJSON((ConfirmOrder &)msg);
	  }
	  case CONFIRM_ORDER_RESPONSE: {
		  return toJSON((ConfirmOrderResponseMessage &)msg);
	  }
	  case CANCEL_ORDER: {
		  return toJSON((CancelOrder &)msg);
	  }
	  case CANCEL_ORDER_RESPONSE: {
		  return toJSON((CancelOrderResponseMessage &)msg);
	  }
      case GOODBYE: {
        return toJSON((GoodbyeMessage &) msg);
      }
      default: {

      }
    }

    // unknown message type
    JSON err;
    err[MESSAGE_STATUS] = MESSAGE_STATUS_ERROR;
    err[MESSAGE_INFO] = std::string("Unknown message type");
    return err;
  }

  /**
   * Converts a JSON object representing a item to a Item object
   * @param j JSON object
   * @return Item
   */
  static Item parseItem(const JSON &j) {
    return Item(j[MESSAGE_ITEM_NAME], j[MESSAGE_ITEM_ID], j[MESSAGE_ITEM_QUANTITY], j[MESSAGE_ITEM_WEIGHT]);
  }

  /**
   * Converts a JSON array representing a list of items to a
   * vector of Item objects
   * @param jsongs JSON array
   * @return resulting vector of Item
   */
  static std::vector<Item> parseItems(const JSON &jitems) {
    std::vector<Item> out;

    for (const auto& item : jitems) {
      out.push_back(parseItem(item));
    }

    return out;
  }

  /**
  * Converts a JSON object representing a ItemEntry to a ItemEntry object
  * @param j JSON object
  * @return ItemEntry
  */
  static ItemEntry parseEntry(const JSON &j) {
	  return ItemEntry(j[MESSAGE_ITEM_NAME], j[MESSAGE_ITEM_QUANTITY], j[MESSAGE_ITEM_ID], j[MESSAGE_ITEM_WEIGHT], j[MESSAGE_ITEM_PRICE]);
  }

  /**
  * Converts a JSON array representing a list of item entries to a
  * vector of ItemEntry objects
  * @param jsongs JSON array
  * @return resulting vector of ItemEntry
  */
  static std::vector<ItemEntry> parseEntries(const JSON &jentry) {
	  std::vector<ItemEntry> out;

	  for (const auto&entry : jentry) {
		  out.push_back(parseEntry(entry));
	  }

	  return out;
  }

  /**
  * Converts a JSON object representing a Order to a Order object
  * @param j JSON object
  * @return Order
  */
  static Order parseOrder(const JSON &jorder) {
	  Order order = Order(3);
	  for (auto&item : jorder) {
		  order.orderList.push_back(parseItem(item));
	  }
	  return order;
  }

  /**
   * Converts a JSON object representing an AddMessage to a AddMessage object
   * @param j JSON object
   * @return AddMessage
   */
  static AddMessage parseAdd(const JSON &jadd) {
    std::string item_name_regex = jadd[MESSAGE_ITEM_NAME];
	int itemID = jadd[MESSAGE_ITEM_ID];
	int itemQuantity = jadd[MESSAGE_ITEM_QUANTITY];
    return AddMessage(item_name_regex, itemID, itemQuantity);
  }

  /**
   * Converts a JSON object representing an AddResponseMessage to an AddResponseMessage object
   * @param j JSON object
   * @return AddResponseMessage
   */
  static AddResponseMessage parseAddResponse(const JSON &jaddr) {
	  std::vector<ItemEntry> results = parseEntries(jaddr[MESSAGE_SEARCH_RESULTS]);
    std::string status = jaddr[MESSAGE_STATUS];
    std::string info = jaddr[MESSAGE_INFO];
    return AddResponseMessage(results, status, info);
  }


  /**
   * Converts a JSON object representing a SearchMessage to a SearchMessage object
   * @param j JSON object
   * @return SearchMessage
   */
  static SearchMessage parseSearch(const JSON &jsearch) {
	  std::string item_name = jsearch[MESSAGE_ITEM_NAME];
	  int ID = jsearch[MESSAGE_ITEM_ID];
    return SearchMessage(item_name, ID);
  }

  /**
   * Converts a JSON object representing a SearchResponseMessage to a SearchResponseMessage object
   * @param j JSON object
   * @return SearchResponseMessage
   */
  static SearchResponseMessage parseSearchResponse(const JSON &jsearchr) {
    std::vector<ItemEntry> results = parseEntries(jsearchr[MESSAGE_SEARCH_RESULTS]);
    std::string status = jsearchr[MESSAGE_STATUS];
    std::string info = jsearchr[MESSAGE_INFO];
    return SearchResponseMessage(results, status, info);
  }

  /**
  * Converts a JSON object representing a ConfirmMessage to a ConfirmMessage object
  * @param j JSON object
  * @return ConfirmMessage
  */
  static ConfirmOrder parseConfirm(const JSON &jconfirm) {
	  std::vector<ItemEntry> order = parseEntries(jconfirm[MESSAGE_CART]);
	  return ConfirmOrder(order);
  }

  /**
  * Converts a JSON object representing a ConfirmMessageMessage to a ConfirmMessageMessage object
  * @param j JSON object
  * @return ConfirmResponseMessage
  */
  static ConfirmOrderResponseMessage parseConfirmMessage(const JSON &jconfirmr) {
	  std::string status = jconfirmr[MESSAGE_STATUS];
	  std::string info = jconfirmr[MESSAGE_INFO];
	  int num = jconfirmr[MESSAGE_ORDER_NUM];
	  return ConfirmOrderResponseMessage(num, status, info);
  }

  /**
  * Converts a JSON object representing a CancelMessage to a CancelMessage object
  * @param j JSON object
  * @return CancelMessage
  */
  static CancelOrder parseCancel(const JSON &jcancel) {
	  int num = jcancel[MESSAGE_ORDER_NUM];
	  return CancelOrder(num);
  }

  /**
  * Converts a JSON object representing a CancelMessageMessage to a CancelMessageMessage object
  * @param j JSON object
  * @return CancelResponseMessage
  */
  static CancelOrderResponseMessage parseCancelMessage(const JSON &jcancelr) {
	  std::string status = jcancelr[MESSAGE_STATUS];
	  std::string info = jcancelr[MESSAGE_INFO];
	  return CancelOrderResponseMessage(status, info);
  }

  /**
   * Converts a JSON object representing a GoodbyeMessage to a GoodbyeMessage object
   * @param j JSON object
   * @return GoodbyeMessage
   */
  static GoodbyeMessage parseGoodbye(const JSON &jbye) {
    return GoodbyeMessage();
  }

  /**
   * Detects the message type from a JSON object
   * @param jmsg JSON object
   * @return message type
   */
  static MessageType parseType(const JSON &jmsg) {
    std::string msg = jmsg[MESSAGE_TYPE];
    if (MESSAGE_ADD == msg) {
      return MessageType::ADD;
    } else if (MESSAGE_ADD_RESPONSE == msg) {
      return MessageType::ADD_RESPONSE;
    } else if (MESSAGE_SEARCH == msg) {
      return MessageType::SEARCH;
	}
	else if (MESSAGE_SEARCH_RESPONSE == msg) {
		return MessageType::SEARCH_RESPONSE;
	} else if(MESSAGE_CONFIRM_ORDER_RESPONSE == msg) {
		return MessageType::CONFIRM_ORDER_RESPONSE;
	}
	else if (MESSAGE_CONFIRM_ORDER == msg) {
		return MessageType::CONFIRM_ORDER;
	}
	else if (MESSAGE_CANCEL == msg) {
		return MessageType::CANCEL_ORDER;
	}
	else if (MESSAGE_CANCEL_RESPONSE == msg) {
		return MessageType::CANCEL_ORDER_RESPONSE;
    } else if (MESSAGE_GOODBYE == msg) {
      return MessageType::GOODBYE;
    }
    return MessageType::UNKNOWN;
  }

  /**
   * Parses a Message object from JSON, returning in a smart pointer
   * to preserve polymorphism.
   *
   * @param jmsg JSON object
   * @return parsed Message object, or nullptr if invalid
   */
  static std::unique_ptr<Message> parseMessage(const JSON &jmsg) {

    MessageType type = parseType(jmsg);
    switch(type) {
      case ADD: {
        return std::unique_ptr<Message>(new AddMessage(parseAdd(jmsg)));
      }
      case ADD_RESPONSE: {
        return std::unique_ptr<Message>(new AddResponseMessage(parseAddResponse(jmsg)));
      }
      case SEARCH: {
        return std::unique_ptr<Message>(new SearchMessage(parseSearch(jmsg)));
      }
      case SEARCH_RESPONSE: {
        return std::unique_ptr<Message>(new SearchResponseMessage(parseSearchResponse(jmsg)));
      }
	  case CONFIRM_ORDER: {
		  return std::unique_ptr<Message>(new ConfirmOrder(parseConfirm(jmsg)));
	  }
	  case CONFIRM_ORDER_RESPONSE: {
		  return std::unique_ptr<Message>(new ConfirmOrderResponseMessage(parseConfirmMessage(jmsg)));
	  }
	  case CANCEL_ORDER: {
		  return std::unique_ptr<Message>(new CancelOrder(parseCancel(jmsg)));
	  }
	  case CANCEL_ORDER_RESPONSE: {
		  return std::unique_ptr<Message>(new CancelOrderResponseMessage(parseCancelMessage(jmsg)));
	  }						
      case GOODBYE: {
        return std::unique_ptr<Message>(new GoodbyeMessage(parseGoodbye(jmsg)));
      }
    }

    return std::unique_ptr<Message>(nullptr);
  }

};

#endif //PROJECT_JSON_H
