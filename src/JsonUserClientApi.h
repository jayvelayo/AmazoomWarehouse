/**
 * @file
 *
 * This file provides an implementation of the JsonUserClientAPI, encapsulating all information
 * required for communication between the client and server.
 *
 * The API currently only has one type of message: JSON_ID
 * An indicator byte is sent before each message so the receiving end knows that we
 * are not sending garbage data.  This is followed by a JSON-encoded string. All commands,
 * queries, and results are parsed from JSON.
 *
 * Communication format:
 *   JSON_ID (1 byte), string size (4 bytes - little endian), JSON ASCII string
 *
 * E.g. to send {"status": "OK"}, which has a length of 17 including the terminating
 * zero, the following bytes will be sent
 *    0x55   0x11 0x00 0x00 0x00    0x7B 0x22 0x73 0x74 0x61 0x74 0x75 0x73 0x22 0x3A 0x20 0x22 0x4F 0x4B 0x22 0x7D 0x00
 *   <JSON>    <integer: 17>                <string: {"status": "OK"} >
 *
 *
 *
 */

#ifndef PROJECT_USERCLIENT_API_JSON_H
#define PROJECT_USERCLIENT_API_JSON_H

#include "UserClientApi.h"
#include "Message.h"
#include "JsonConverter.h"

#include <cpen333/process/socket.h>

#include <algorithm> // for std::min

// fixed port for server
#define MUSIC_LIBRARY_SERVER_PORT 52134

/**
 * Handles communication between sockets
 */
class JsonUserClientApi : public UserClientApi {
 private:
  cpen333::process::socket socket_;

  // Fixed message type
  //   NOTE: constants like this don't actually have a memory address,
  //         so they can only be passed by value
  static const char JSON_ID = 0x55;

  /**
   * Writes the JSON info to the socket, NOT including the JSON byte
   * indicator (for symmetry with the read operation)
   *
   * @param j JSON content
   * @return true if write successful, false otherwise
   */
  bool sendJSON(const JSON& j) {

    // dump to string
    std::string jsonstr = j.dump();

    // encode JSON size, big endian format
    //   (most-significant byte in buff[0])
    char buff[4];
    size_t size = jsonstr.size()+1;           // one for terminating zero
    for (int i=4; i-->0;) {
      // cut off byte and shift size over by 8 bits
      buff[i] = (char)(size & 0xFF);
      size = size >> 8;
    }

    // write contents
    bool success  = socket_.write(buff, 4);   // contents size
    success &= socket_.write(jsonstr);        // contents

    return success;
  }

  /**
   * Reads a string consisting of exactly size bytes
   * @param str string to append to
   * @param size number of bytes
   * @return true if successful
   */
  bool readString(std::string& str, size_t size) {

	const int bufferSize = 256;
    char cbuff[bufferSize];
	bool success = false;

	size_t nwritten = 0;
	while (nwritten < size) {
		int blocksize = std::min<size_t>(size - nwritten, bufferSize);
		if (!socket_.read_all(cbuff, blocksize)) {
			return false;
		}		
		str.append(cbuff, blocksize);
		nwritten += blocksize;
	}

    return true;
  }

  /**
   * Reads and populates a JSON message
   * Assumes the initial JSON indicator byte has already been read, which
   * is why we are now in this method
   *
   * @param jout JSON object to populate
   * @return true if successful, false if error
   */
  bool recvJSON(JSON& jout) {

    // receive 4-byte size
    unsigned char buff[4];
    if (!socket_.read_all(buff, 4)) {
      return false;
    }

	//const int byteSize = 8;
	size_t size = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];

    // read entire JSON string
    std::string str;
    if (!readString(str, size)) {
      return false;
    }

    // parse JSON
    jout = JSON::parse(str);

    return true;
  }

  // prevent default constructor
  JsonUserClientApi();

 public:

  /**
   * Main constructor, takes ownership of socket
   * @param socket
   */
  JsonUserClientApi(cpen333::process::socket&& socket) :
    socket_(std::move(socket)) {}

  /**
   * Sends a message by writing the data to the socket
   * @param msg message to write
   * @return true if successful, false if error
   */
  bool sendMessage(const Message& msg) {
    JSON jmsg = JsonConverter::toJSON(msg);

    // write single JSON byte
    char id = JSON_ID;
    if (!socket_.write(&id, 1)) {
      return false;
    }
    // write JSON content
    return sendJSON(jmsg);
  }

  /**
   * Reads a message from the socket.  The returned message is
   * contained within a smart pointer to preserve polymorphism
   * and automatically handle freeing of memory resources.  The
   * returned smart pointer can be used similarly to a real
   * pointer, except that it cannot be copied.  You can, however,
   * access members by using the -> operator, and dereference it
   * using the * operator.
   *
   * @return parsed message, nullptr if an error occurred
   */
  std::unique_ptr<Message> recvMessage() {

    // parse first byte, ensure it is of JSON type
    char id;
    if (!socket_.read_all(&id, 1) || id != JSON_ID) {
      return nullptr;
    }

    // if it is a JSON string, parse into a message
    JSON jmsg;
    if (!recvJSON(jmsg)) {
      return nullptr;
    }

    return JsonConverter::parseMessage(jmsg);
  }

};

#endif //PROJECT_USERCLIENT_API_JSON_H
