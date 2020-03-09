# Implementation
^`type` is one of `(String*, int, bool, float)`

# Node
Subclass of Client.

# KVStore
Subclass of Server. This class acts as the middle-man between the layer above (distributed arrays) and the nodes that contain the actual data.

**fields**:
* Map from node indicies to their IP addresses

**methods**:
* `char* serialize(type`^`* df)` - Serializes the given array of data into a byte stream.
* `type* deserialize(char* stream)` - Deserializes the given byte stream into an array of data.
* `void put(Key* k, type* v)` - Reads the node index from `k`, sends the corresponding node the string from `k` and a serialized version of `v`, and tells it to store `v` at that key.
* `type* get(Key* k)` - Reads the node index from `k`, sends the string from `k` to the corresponding node, and tells it to retrieve the value at that key.
* `type* getAndWait(Key* k)` - Reads the node index from `k`, sends the string from `k` to the corresponding node, and tells it to retrieve the value at that key. If there is no value at that key, wait until there is.


# Vector
An array of objects split into fixed-size chunks. When it fills up, it grows, allocating more memory for new chunks.

**fields**:
* `Object*** objects` - Array of pointers to chunks containing the data.

**methods**:
* `void append(Object* val)` - Appends the given object to the end of the vector.
* `Object* get(size_t idx)` - Returns the object in the vector at the given index.


# IntVector, BoolVector, FloatVector


# DistributedArray (Column)
Subclass of Vector. Each chunk is stored as a k/v pair.

**fields**: 
* List of keys pointing to each chunk
* Cache?

**methods**: 
* 


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
Users will subclass this in order to write their applications that use the eau2 system. The application will be run on each node in the system. Each node will perform different tasks based on its index.

**fields**:
* `size_t idx` - index that differentiates one node from another
* `KVStore& kv` - each node has its own kv store with a part of the data

# Open Questions
* What type is v in put(k, v)? Should it be an already serialized byte stream or an object/primitive?
* What is the distributed array's cache?