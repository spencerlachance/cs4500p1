# Introduction
The eau2 system allows the user to implement applications that can analyse and manipulate data stored in a key/value store. Under the hood, this key/value store is distributed accross a network of nodes.

# Architecture
eau2 is a program with three layers. The bottom layer is a network of KVStores running on different nodes that can talk to each other, store serialized data and share it upon request. The second layer is a dataframe which is a read-only table represented as an object. It is made up of rows and columns, with each column containing data of a specific type. Each column is implemented as a distributed array which is an array of keys used to retrieve data distributed accross multiple KVStores. The application layer is the final layer that sits on top of both previous layers. From the application layer, the user can write their programs that analyse and manipulate data.

# Implementation
^`type` is one of `(String*, int, bool, float)`.

## KVStore
Subclass of Client. This class receives keys from the layer above and either serializes data and puts it in the store or deserializes it and returns it. If the key corresponds to data from another KVStore then it will talk to that KVStore and retrieve the data. Upon request, the KVStore class also sends its own serialized data blobs to other KVStores. Upon start up, one of the KVStores becomes a primary node that maintains new node registration to the network.

**fields**:
* Map from other node indicies to their IP addresses
* `Map* map_` - A Map object that can map objects to objects. In this case, it will be used to map String containing keys to Strings containing serialized data.

**methods**:
* `void put(Key* k, type* v)` - Reads the node index from `k`, sends the string from `k` to the corresponding KVStore and a serialized version of `v`, and tells it to store `v` at that key.
* `type* get(Key* k)` - Reads the node index from `k`, sends the string from `k` to the corresponding KVStore, and tells it to retrieve the value at that key.
* `type* getAndWait(Key* k)` - Reads the node index from `k`, sends the string from `k` to the corresponding KVStore, and tells it to retrieve the value at that key. If there is no value at that key, wait until there is, then retrieve it.


## Key
Object representing a key that corresponds to data in a key/value store.

**fields**:
* `String* key_` - The actual string that will be mapped to data in the KVStore.
* `size_t idx_` - The index of the node that holds the k/v store containing the data.

**methods**:
* `String* get_keystring()` - Getter for the key field.
* `size_t get_home_node()` - Getter for the idx field.


## Vector
An array of objects split into fixed-size chunks. When it fills up, it grows, allocating more memory for new chunks.

**fields**:
* `Object*** objects_` - Array of pointers to chunks containing the data.

**methods**:
* `void append(Object* val)` - Appends the given object to the end of the vector.
* `Object* get(size_t idx)` - Returns the object in the vector at the given index.

## IntVector, BoolVector, FloatVector
Similar to the Vector class, but holds primitives instead of objects.

## DistributedArray (Column)
Subclass of Vector. Each chunk is stored as a k/v pair.

**fields**: 
* List of keys pointing to each chunk
* Cache?

## DataFrame
Table containing columns of a specific type

**fields**:
* Schema
* Array of Column objects containing all of the fields

**methods**:
* Getters for specific fields (one for each type)
* `void map(Rower& r)`
* `void pmap(Rower& r)`
* `DataFrame* filter(Rower& r)`
* `DataFrame* fromTypeArray(Key* k, KVStore* kv, size_t size, type* vals)` - Static method that generates a new DataFrame with one column containing `size` values from `vals`, serializes the dataframe and puts it in `kv` at `k`, and then returns the DataFrame.
* `DataFrame* fromTypeScalar(Key* k, KVStore* kv, type val)` - Static method that generates a new DataFrame with one column and row containing `val`, serializes the dataframe and puts it in `kv` at `k`, and then returns the DataFrame.


## Application (abstract class)
Users will subclass this class in order to write their applications that use the eau2 system. The application will be run on each node in the system. Each node can perform different tasks based on its index.

**fields**:
* `size_t idx_` - index that differentiates one node from another
* `KVStore& kv_` - each node has its own kv store with a part of the data

**methods**:
* `virtual void run_()` - runs the application
* `size_t this_node()` - returns this node's index


# Use Cases
The code below builds a DataFrame containing 100 ints and strings, and then calculates the sum of every int in the DataFrame.
```
Schema s("IS");
DataFrame* df = new DataFrame(s);
Row* r = new Row(s);
String* str = new String("foo");
for (int i = 1; i <= 100; i++) {
    r->set(0, i);
    r->set(1, str);
    df->add_row(*r);
}
SumRower* sr = new SumRower();
df->map(*sr);
assert(sr->get_total() == 5050);
delete df;
delete r;
delete str;
delete sr;
```
This code builds a DataFrame with one column of floats, adds it to a KVStore, gets it back from the store, and ensures that the floats' values remain consistent.
```
class Trivial : public Application {
    public:
    Trivial(size_t idx) : Application(idx) { }
    void run_() {
        size_t SZ = 1000*1000;
        float* vals = new float[SZ];
        float sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        Key key("triv",0);
        DataFrame* df = DataFrame::fromFloatArray(&key, &kv_, SZ, vals);
        assert(df->get_float(0,1) == 1);
        DataFrame* df2 = kv_.get(key);
        for (size_t i = 0; i < SZ; ++i) sum -= df2->get_float(0,i);
        assert(sum==0);
        delete df; delete df2;
    }
};
```

# Open Questions
* What is the distributed array's cache?
* Should a DataFrame hold actual data or keys to data stored in a KVStore?
  * How does the DataFrame interact with the KVStore when being queried with keys?

# Status
## What has been done:
* Implemented Vector classes that hold objects, ints, floats and booleans respectively. These data structures are resizable while maintaining constant look-up time and avoiding copying the payload.
* Implemented the DataFrame class. Users are able to read data from a file using an adapter and generate DataFrames from it. They can also map and filter the DataFrame.
* Implemented a working network layer that allows for client registration with a server and direct client communication.
* Implemented a protocol for serialization and deserialization.
* Implemented serialization and deserialization of Object, String, all vectors, columns, Message subclasses, and DataFrame.
* Implemented the KVStore and Application classes. They now run properly on a single node.
## What is left to do:
* Make the system distributed and integrate networking.
* Implement the Distributed Array class.
* Solidify a way for all three layers in the system to interact with each other successfully.
