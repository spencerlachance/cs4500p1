// lang::CwC

#include "string.h"

/**
 * An Object subclass representing a key that corresponds to data value in a KVStore on some node.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Key : public Object {
    public:
    // The string key that the data will be stored at within the map
    String* key_;
    // The index of the node on which the data is stored
    size_t idx_;

    /**
     * Constructor
     */
    Key(const char* key, size_t idx) {
        key_ = new String(key);
        idx_ = idx;
    }

    /**
     * Destructor
     */
    ~Key() {
        delete key_;
    }

    /**
     * Getter for the key string. 
     */
    String* get_keystring() {
        return key_;
    }

    /**
     * Getter for the home node index.
     */
    size_t get_home_node() {
        return idx_;
    }

    /* Returns a serialized representation of this key */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: key, key: ");
            const char* serial_k = key_->serialize();
            buff.c(serial_k);
            buff.c(", idx: ");
            buff.c(idx_);
            buff.c("}");
            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());

            delete serial_str;
            delete[] serial_k;
            return copy;
        }

        /* Return true if this key is equal to the given objects, and false if not. */
        bool equals(Object* o) {
            Key* other = dynamic_cast<Key*>(o);
            if (other == nullptr) return false;
            return (idx_ == other->get_home_node()) && (key_->equals(other->get_keystring()));
        }
};