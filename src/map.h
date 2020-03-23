//lang::CwC

#pragma once

#include <assert.h>

#include "object.h"

#define INITIAL_MAP_CAPACITY 8

/**
 * Private Node class that represents one entry in the Map's Array.
 * 
 * Contains a next Node field which creates a linked list of Map
 * entries that have the same index.
 */
class Node : public Object {
  public:
    Object* key_;
    Object* val_;
    Node* next_;

    /**
     * Constructor for Node class
     * 
     * @param key This Node's key
     * @param val This Node's value
     */
    Node(Object* key, Object* val) {
      key_ = key;
      val_ = val;
      next_ = nullptr;
    }

    /**
     * Deconstructor for Node class
     */
    ~Node() {
      delete key_;
      delete val_;
      if (next_ != nullptr) {
        delete next_;
      }
    }

    /**
     * Gets this Node's key.
     * 
     * @return the key
     */
    Object* get_key() {
      return key_;
    }

    /**
     * Gets this Node's value.
     * 
     * @return the value
     */
    Object* get_value() {
      return val_;
    }

    /**
     * Gets this Node's next Node.
     * 
     * @return the Node
     */
    Node* get_next() {
      return next_;
    }

    /**
     * Gives this Node a new value.
     * 
     * @param val The new value
     */
    void set_value(Object* val) {
      val_ = val;
    }

    /**
     * Sets this Node's next Node.
     * 
     * @param n The Node to set as this Node's next.
     */
    void set_next(Node* n) {
      next_ = n;
    }

    /**
     * The length of the linked list starting at this Node.
     * 
     * @return the length
     */
    int length() {
      int length = 1;
      if (has_next()) {
        length += next_->length();
      }
      return length;
    }

    /**
     * Removes this Node's next Node.
     */
    void remove_next() {
      assert(next_ != nullptr);
      delete next_;
      next_ = nullptr;
    }

    /**
     * Does this Node have a next Node?
     * 
     * @return True if this Node has a next, False if it does not.
     */
    bool has_next() {
      return next_ != nullptr;
    }

    /**
     * Generates a unique hash code for this Node.
     * 
     * @return the hash code
     */
    size_t hash() {
      size_t hash = 0;
      hash += key_->hash() + val_->hash();
      hash /= 4;
      if (has_next()) {
        hash += next_->hash();
      }
      return hash;
    }

    /**
     * Is this Node equal to the given Object?
     */
    bool equals(Object* o) {
      Node* other = dynamic_cast<Node*>(o);
      if (other == nullptr) return false;

      if (this->has_next()) {
        if (other->has_next()) {
          return  key_->equals(other->get_key()) && 
                  val_->equals(other->get_value()) && 
                  next_->equals(other->get_next());
        } else {
          return false;
        }
      } else {
        return  key_->equals(other->get_key()) && 
                val_->equals(other->get_value()) && 
                !other->get_next();
      }
    }
};

/**
 * Implementation of a Map data structure.
 * 
 * Maintains an array of Node objects containing each key, value pair.
 * Stores key, value pairs that hash to the same index in a linked list.
 */
class Map : public Object {
  public:
    // Internal array of Nodes containing the key value pairs
    Node** objects_;
    int capacity_;
    int size_;

    // Constructor
    Map() {
      capacity_ = INITIAL_MAP_CAPACITY;
      objects_ = new Node*[capacity_];
      for (int i = 0; i < capacity_; i++) objects_[i] = nullptr;
      size_ = 0;
    }

    // Constructor with specified inital capacity
    Map(int init_cap) {
      capacity_ = init_cap;
      objects_ = new Node*[capacity_];
      for (int i = 0; i < capacity_; i++) objects_[i] = nullptr;
      size_ = 0;
    }

    // Destructor
    ~Map() {
      for (int i = 0; i < capacity_; i++) {
        if (objects_[i] != nullptr) {
          delete objects_[i];
        }
      }
      delete[] objects_;
    }

    /**
     * Getter for this Map's Array of Nodes
     * 
     * @return the Array
     */
    Node** get_nodes() {
      return objects_;
    }

    /**
     * Gets the Node from this Map's internal Array at the given index.
     * 
     * @param index The index of the requested Node.
     */
    Node* get_node(int index) {
      return dynamic_cast<Node*>(objects_[index]);
    }

    /**
     * Private function that increases the capacity
     * and recalculates the Map if too many key, value pairs
     * have the same index.
     */
    void rehash_() {
      Map* new_map = new Map(capacity_ + 8);
      Node* current;
      for (int i = 0; i < capacity_; i++) {
        current = get_node(i);
        if (current != nullptr) {
          new_map->put(current->get_key(), current->get_value());
          while (current->has_next()) {
            current = current->get_next();
            new_map->put(current->get_key(), current->get_value());
          }
        }
      }

      capacity_ += 8;
      objects_ = new_map->get_nodes();
    }

    // Add an Object to the map with an Object key
    // Takes ownership of the key and val
    void put(Object* key, Object* val) {
      int index = key->hash() % capacity_;
      
      Node* current = get_node(index);
      if (current == nullptr) {
        objects_[index] = new Node(key, val);
      } else {
        if (current->length() >= 3) {
          rehash_();
          put(key, val);
          return;
        }
        
        if (current->get_key()->equals(key)) {
          // There is already an entry in the map with this key, so replace 
          // the existing value with the given one.
          current->set_value(val);
          return;
        } else {
          // Traverse the linked list until you find the right entry
          while (current->has_next()) {
            current = current->get_next();
            if (current->get_key()->equals(key)) {
              // There is already an entry in the map with this key, so replace 
              // the existing value with the given one.
              current->set_value(val);
              return;
            }
          }
          current->set_next(new Node(key, val));
        }
      }
      size_++;
    }

    // Remove the value tied to the key from the map. Removes the key as well.
    Object* remove(Object* key) {
      int index = key->hash() % capacity_;

      Node* current = get_node(index);
      if (current == nullptr) return nullptr;

      if (current->get_key()->equals(key)) {
        // The matching node is the first one in the linked list
        if (current->has_next()) {
          objects_[index] = current->get_next();
        }
        size_--;

        Object* return_me = current->get_value();
        delete current;
        objects_[index] = nullptr;
        return return_me;
      } else {
        // Start traversing the linked list
        Node* current_next;
        while (current->has_next()) {
          current_next = current->get_next();
          if (current_next->get_key()->equals(key)) {
            // current_next is the node to be removed, so replace its
            // previous node's next with its next
            if (current_next->has_next()) {
              current->set_next(current_next->get_next());
            } else {
              current->remove_next();
            }
            size_--;  

            Object* return_me = current_next->get_value();
            delete current_next;
            return return_me;
          }
          current = current->get_next();
        }
        return nullptr;
      }
    }

    // Gets the value associated with the key
    Object* get(Object* key) {
      int index = key->hash() % capacity_;

      Node* current = get_node(index);
      if (current == nullptr) return nullptr;

      if (current->get_key()->equals(key)) {
        // The matching node is the first one in the linked list.
        return current->get_value();
      } else {
        // Look for it in the linked list.
        while (current->has_next()) {
          current = current->get_next();
          if (current->get_key()->equals(key)) {
            return current->get_value();
          }
        }
        return nullptr;
      }
    }

    // Clears all the keys and values from the map
    void clear() {
      Node* current;
      Node* tmp;
      for (int i = 0; i < capacity_; i++) {
        current = get_node(i);
        if (current != nullptr) {
          while (current->has_next()) {
            tmp = current;
            current = current->get_next();
            delete tmp;
            size_--;
          }
          delete current;
          size_--;
          objects_[i] = nullptr;
        }
      }
    }

    // Is the key in the map?
    bool containsKey(Object* key) {
      return get(key) != nullptr;
    }

    // Get the number of keys in the map
    size_t size() {
      return size_;
    }

    // Is this map equal to the Object?
    bool equals(Object* o) {
      Map* other = dynamic_cast<Map*>(o);
      if (other == nullptr) return false;

      Node* current;
      Node* o_current;
      for (int i = 0; i < capacity_; i++) {
        current = get_node(i);
        o_current = other->get_node(i);
        if (current == nullptr || o_current == nullptr) {
          if (current != o_current) return false;
        } else {
          if (!current->equals(o_current)) return false;
        }
      }
      return true;
    }

    // Get the hash of the map object
    size_t hash() {
      size_t hash = 0;
      Node* current;
      for (int i = 0; i < capacity_; i++) {
        current = get_node(i);
        if (current != nullptr) {
          hash += current->hash();
        }
      }
      return hash;
    }
};