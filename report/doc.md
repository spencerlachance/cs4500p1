# Introduction
The eau2 program allows the user to implement a variety of applications that can read from a database. This database is a network of nodes that store and retrieve data in a distributed fashion, we call these kvstores. This distributed database uses keys that correspond to the location of a data blob in the network to store and retrieve the data blob.

# Architecture
eau2 is a program with three layers. The bottom layer is a network of KVStore nodes that can talk to each other, store serialized data and share it upon request. The second layer is a dataframe whose columns are distributed arrays. Distributed arrays are arrays of keys used to retrieve data distributed accross multiple KVStore nodes. The application layer is the final layer that sits on top of both previous layers. From the application layer, the user can write programs that can accomplish a variety of tasks using dataframes, KVStores and the data contained in the KVStores.

# Implementation
^`type` is one of `(String*, int, bool, float)`.

# KVStore
Subclass of Client. This class receives keys from the layer above and returns deserialized data. If the key corresponds to another KVStore then it will talk to that KVStore and retrieve the data. Upon request, the KVStore class also sends its own serialized data blobs to other KVStore nodes. Upon start up, one of the nodes becomes a primary node that maintains new node registration to the network.

**fields**:
* Map from other KVStore indicies to their IP addresses
* Map from strings to serialized data blobs

**methods**:
* `char* serialize(type`^`* df)` - Serializes the given array of data into a byte stream.
* `type* deserialize(char* stream)` - Deserializes the given byte stream into an array of data.
* `void put(Key* k, type* v)` - Reads the KVStore node index from `k`, sends the corresponding node the string from `k` and a serialized version of `v`, and tells it to store `v` at that key.
* `type* get(Key* k)` - Reads the KVStore index from `k`, sends the string from `k` to the corresponding node, and tells it to retrieve the value at that key.
* `type* getAndWait(Key* k)` - Reads the KVStore index from `k`, sends the string from `k` to the corresponding node, and tells it to retrieve the value at that key. If there is no value at that key, wait until there is, then retrieve it.


# Vector
An array of objects split into fixed-size chunks. When it fills up, it grows, allocating more memory for new chunks.

**fields**:
* `Object*** objects` - Array of pointers to chunks containing the data.

**methods**:
* `void append(Object* val)` - Appends the given object to the end of the vector.
* `Object* get(size_t idx)` - Returns the object in the vector at the given index.

# IntVector, BoolVector, FloatVector
Similar to the Vector class, but holds primitives instead of objects.

# DistributedArray (Column)
Subclass of Vector. Each chunk is stored as a k/v pair.

**fields**: 
* List of keys pointing to each chunk
* Cache?

# DataFrame
* Table containing columns of a specific type

**fields**:
* Schema
* Array of Column objects containing all of the fields

**methods**:
* Getters for specific fields (one for each type)
* `void map(Rower& r)`
* `void pmap(Rower& r)`
* `DataFrame* filter(Rower& r)`
* `DataFrame* fromArray(Key* k, KVStore* kv, size_t size, type* vals)` - Static method that generates a new DataFrame with one column containing `size` values from `vals`, serializes the dataframe, adds it to `kv` at `k` and then returns it.
* `DataFrame* fromScalar(Key* k, KVStore* kv, type val)` - Static method that generates a new DataFrame with one column and row containing `val`, serializes the dataframe, adds it to `kv` at `k` and then returns it.

# Application (interface)
Users will subclass this in order to write their applications that use the eau2 system. The application will be run on each node in the system. Each node can perform different tasks based on its index.

**fields**:
* `size_t idx` - index that differentiates one node from another
* `KVStore& kv` - each node has its own kv store with a part of the data

# Open Questions
* What type is v in put(k, v)? Should it be an already serialized byte stream or an object/primitive?
* What is the distributed array's cache?

# Status
## What has been done:
* Cleaned up our code from memory leaks and allowed for strings to be readable from sorer file to a dataframe.
## What is left to do:
* Allow for serialization and deserialization of dataframes
