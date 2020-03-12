# Introduction
The eau2 system allows the user to implement applications that can analyse and manipulate data stored in a key/value store. Under the hood, this key/value store is distributed accross a network of nodes.

# Architecture
eau2 is a program with three layers. The bottom layer is a network of KVStores running on different nodes that can talk to each other, store serialized data and share it upon request. The second layer is a dataframe is a read-only table represented as an object. It is made up of rows and columns, with each column containing data of a specific type. Each column is implemented as a distributed array which is an array of keys used to retrieve data distributed accross multiple KVStores. The application layer is the final layer that sits on top of both previous layers. From the application layer, the user can write their programs that analyse and manipulate data.

# Implementation
^`type` is one of `(String*, int, bool, float)`.

## KVStore
Subclass of Client. This class receives keys from the layer above and either serializes data and puts it in the store or deserializes it and returns it. If the key corresponds to data from another KVStore then it will talk to that KVStore and retrieve the data. Upon request, the KVStore class also sends its own serialized data blobs to other KVStores. Upon start up, one of the KVStores becomes a primary node that maintains new node registration to the network.

**fields**:
* Map from other node indicies to their IP addresses
* Map from strings to serialized data blobs

**methods**:
* `char* serialize(type`^`* df)` - Serializes the given array of data into a byte stream.
* `type* deserialize(char* stream)` - Deserializes the given byte stream into an array of data.
* `void put(Key* k, type* v)` - Reads the node index from `k`, sends the string from `k` to the corresponding KVStore and a serialized version of `v`, and tells it to store `v` at that key.
* `type* get(Key* k)` - Reads the node index from `k`, sends the string from `k` to the corresponding KVStore, and tells it to retrieve the value at that key.
* `type* getAndWait(Key* k)` - Reads the node index from `k`, sends the string from `k` to the corresponding KVStore, and tells it to retrieve the value at that key. If there is no value at that key, wait until there is, then retrieve it.


## Vector
An array of objects split into fixed-size chunks. When it fills up, it grows, allocating more memory for new chunks.

**fields**:
* `Object*** objects` - Array of pointers to chunks containing the data.

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
* `DataFrame* fromArray(Key* k, KVStore* kv, size_t size, type* vals)` - Static method that generates a new DataFrame with one column containing `size` values from `vals`, serializes the dataframe and puts it in `kv` at `k`, and then returns the DataFrame.
* `DataFrame* fromScalar(Key* k, KVStore* kv, type val)` - Static method that generates a new DataFrame with one column and row containing `val`, serializes the dataframe and puts it in `kv` at `k`, and then returns the DataFrame.

## Application (interface)
Users will subclass this in order to write their applications that use the eau2 system. The application will be run on each node in the system. Each node can perform different tasks based on its index.

**fields**:
* `size_t idx` - index that differentiates one node from another
* `KVStore& kv` - each node has its own kv store with a part of the data

**methods**:
* `virtual void run_()` - runs the application
* `size_t this_node()` - returns this node's index

# Use Cases

# Open Questions
* What type is v in put(k, v)? Should it be an already serialized byte stream or an object/primitive?
* What is the distributed array's cache?
* Should a DataFrame hold actual data or keys to data stored in a KVStore?
  * How does the DataFrame interact with the KVStore when being queried with keys?

# Status
## What has been done:
* Implemented Vector classes that hold objects, ints, floats and booleans respectively. These data structures are resizable while maintaining constant look-up time and avoiding copying the payload.
* Implemented the DataFrame class. Users are able to read data from a file using an adapter and generate DataFrames from it. They can also map and filter the DataFrame.
* Implemented a working network layer that allows for client registration with a server and direct client communication.
* Implemented a protocol for serialization and deserialization.
## What is left to do:
* Implement serialization and deserialization of dataframes.
* Design the Application interface.
* Implement the KVStore and Distributed Array classes.
* Figure out a way for all three layers in the system to interact with each other.
