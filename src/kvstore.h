// //lang::CwC

// #include <unistd.h>

// #include "map.h"
// #include "dataframe.h"
// #include "serial.h"

// /**
//  * An Object subclass representing a key that corresponds to data value in a KVStore on some node.
//  * 
//  * @author Spencer LaChance <lachance.s@northeastern.edu>
//  * @author David Mberingabo <mberingabo.d@husky.neu.edu>
//  */
// class Key : public Object {
//     public:
//     // The string key that the data will be stored at within the map
//     String* key_;
//     // The index of the node on which the data is stored
//     size_t idx_;

//     /**
//      * Constructor
//      */
//     Key(const char* key, size_t idx) {
//         key_ = new String(key);
//         idx_ = idx;
//     }

//     /**
//      * Destructor
//      */
//     ~Key() {
//         delete key_;
//     }

//     /**
//      * Getter for the key string.
//      */
//     String* get_keystring() {
//         return key_;
//     }

//     /**
//      * Getter for the home node index.
//      */
//     size_t get_home_node() {
//         return idx_;
//     }
// };

// /**
//  * A class describing an implementation of a key/value store.
//  * This class represents a store kept on one node from a larger distributed system.
//  * 
//  * @author Spencer LaChance <lachance.s@northeastern.edu>
//  * @author David Mberingabo <mberingabo.d@husky.neu.edu>
//  */
// class KVStore : public Object {
//     public:
//     // The map from string keys to deserialized data blobs
//     Map* map_;

//     /**
//      * Constructor that initializes an empty KVStore.
//      */
//     KVStore() {
//         map_ = new Map();
//     }

//     /**
//      * Serializes the given data and puts it into the map at the given key.
//      * 
//      * @param k The key at which the data will be stored
//      * @param v The data that will be stored in the k/v store
//      */
//     void put(Key& k, DataFrame& v) {
//         // map_->put(k->get_keystring(), new String(v->serialize()));
//     }

//     /**
//      * Gets the data stored at the given key, deserializes it, and returns it.
//      * 
//      * @param k The key at which the reqested data is stored
//      * 
//      * @return The deserialized data
//      */
//     DataFrame* get(Key& k) {
//         String* serialized_df = dynamic_cast<String*>(map_->get(k.get_keystring()));
//         DataFrame* deserialized_df = 
//             dynamic_cast<DataFrame*>(Deserializer::deserialize(serialized_df->c_str()));
//         assert(deserialized_df != nullptr);
//         return deserialized_df;
//     }

//     /**
//      * Waits until there is data in the store at the given key, and then gets it, deserializes it,
//      *  and returns it.
//      * 
//      * @param k The key at which the reqested data is stored
//      * 
//      * @return The deserialized data
//      */
//     DataFrame* get_and_wait(Key& k) {
//         bool contains_key = map_->containsKey(&k);
//         while (!contains_key) {
//             sleep(1);
//             contains_key = map_->containsKey(&k);
//         }
//         return get(k);
//     }
// };