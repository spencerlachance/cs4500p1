//lang::CwC

#include <unistd.h>

#include "map.h"
// #include "dataframe.h"
#include "serial.h"

/**
 * A class describing an implementation of a key/value store.
 * This class represents a store kept on one node from a larger distributed system.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class KVStore : public Object {
    public:
    // The map from string keys to deserialized data blobs
    Map* map_;

    /**
     * Constructor that initializes an empty KVStore.
     */
    KVStore() {
        map_ = new Map();
    }

    /**
     * Destructor
     */
    ~KVStore() {
        delete map_;
    }

    /**
     * Serializes the given data and puts it into the map at the given key.
     * 
     * @param k The key at which the data will be stored
     * @param v The data that will be stored in the k/v store
     */
    void put(Key& k, DataFrame& v) {
        const char* serial_df = v.serialize();
        map_->put(k.get_keystring(), new String(serial_df));
        delete[] serial_df;
    }

    /**
     * Gets the data stored at the given key, deserializes it, and returns it.
     * 
     * @param k The key at which the reqested data is stored
     * 
     * @return The deserialized data
     */
    DataFrame* get(Key& k) {
        String* serialized_df = dynamic_cast<String*>(map_->get(k.get_keystring()));
        assert(serialized_df != nullptr);
        Deserializer* ds = new Deserializer(serialized_df->c_str());
        DataFrame* deserialized_df = dynamic_cast<DataFrame*>(ds->deserialize());
        assert(deserialized_df != nullptr);
        delete ds;
        return deserialized_df;
    }

    /**
     * Waits until there is data in the store at the given key, and then gets it, deserializes it,
     *  and returns it.
     * 
     * @param k The key at which the reqested data is stored
     * 
     * @return The deserialized data
     */
    DataFrame* wait_and_get(Key& k) {
        bool contains_key = map_->containsKey(k.get_keystring());
        while (!contains_key) {
            sleep(1);
            contains_key = map_->containsKey(k.get_keystring());
        }
        return get(k);
    }
};

/**
 * Builds a DataFrame with one column containing the data in the given int array, adds the
 * DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromIntArray(Key* k, KVStore* kv, size_t size, int* vals) {
    Column* col = new IntColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the data in the given bool array, adds the
 * DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromBoolArray(Key* k, KVStore* kv, size_t size, bool* vals) {
    Column* col = new BoolColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the data in the given float array, adds the
 * DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromFloatArray(Key* k, KVStore* kv, size_t size, float* vals) {
    Column* col = new FloatColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the data in the given string array, adds
 * the DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromStringArray(Key* k, KVStore* kv, size_t size, String** vals) {
    Column* col = new StringColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given int, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromIntScalar(Key* k, KVStore* kv, int val) {
    Column* col = new IntColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given bool, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromBoolScalar(Key* k, KVStore* kv, bool val) {
    Column* col = new BoolColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given float, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromIntScalar(Key* k, KVStore* kv, float val) {
    Column* col = new FloatColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given string, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromStringScalar(Key* k, KVStore* kv, String* val) {
    Column* col = new StringColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}