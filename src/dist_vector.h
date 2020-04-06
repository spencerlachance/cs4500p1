//lang::CwC

#pragma once

#include "datatype.h"
#include "kvstore.h"

// The number of items that each chunk holds
#define CHUNK_SIZE 1024

/**
 * This class represents a unit of the DistributedVector, i.e. a fixed-size array of fields.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Chunk : public Object {
public:
    // Array of fields, all fields are owned
    DataType** fields_;
    // The number of fields currently in the array
    size_t size_;
    // The index of this Chunk within the DVector
    size_t idx_;

    /** Constructor */
    Chunk(size_t idx) : fields_(new DataType*[CHUNK_SIZE]), size_(0), idx_(idx) { }

    /** Destructor */
    ~Chunk() {
        for (int i = 0; i < size_; i++)
            delete fields_[i];
        delete[] fields_;
    }

    /** Adds a field to the end of this Chunk */
    void append(DataType* dt) {
        exit_if_not(size_ < CHUNK_SIZE, "This Chunk is full");
        fields_[size_] = dt;
        size_++;
    }

    /** Returns the field at the given index */
    DataType* get(size_t index) {
        exit_if_not(index < size_, "Chunk: Index out of bounds");
        return fields_[index];
    }

    /** Getter for the size */
    size_t size() { return size_; }

    /** Getter for the index */
    size_t idx() { return idx_; }

    /** Returns a char* representation of this Chunk */
    const char* serialize() {
        StrBuff buff;
        // Serialize the index
        Serializer ser;
        const char* serial_idx = ser.serialize_int(idx_);
        buff.c(serial_idx);
        delete[] serial_idx;
        // Serialize the fields
        buff.c("[");
        for (int i = 0; i < size_; i++) {
            const char* serial_dt = fields_[i]->serialize();
            buff.c(serial_dt);
            delete[] serial_dt;
        }
        buff.c("]");
        return buff.c_str();
    }
};

/**
 * A vector of DataFrame fields. The fields are split into chunks of a fixed size and each chunk
 * is serialized and stored in the KVStore. So this is essentially just a vector of keys that point
 * to the chunks.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class DistributedVector : public Object {
public:
    // The number of fields in this vector
    size_t size_;
    // The current chunk that is being added to, owned
    Chunk* current_;
    // Vector of keys pointing to this DVector's chunks
    Vector* keys_;
    // The current node's KVStore, external
    KVStore* kv_;
    // The key to this DVector's column
    Key* k_;
    // KeyBuff used to build each chunk's key
    KeyBuff* kbuf_;
    // Have all fields been added to this DVector?
    bool is_locked_;

    /** Initialize an empty DistributedVector. The given Key is that of the column that owns this
     *  DVector, the keys for each chunk are built off of it. */
    DistributedVector(KVStore* kv, Key* k) : 
        size_(0), current_(new Chunk(0)), keys_(new Vector()), kv_(kv), k_(k), 
        kbuf_(new KeyBuff(k_)), is_locked_(false) { }

    /** Initialize a DistributedVector containing the given keys. The given Key is that of the 
     *  column that owns this DVector, the keys for each chunk are built off of it. */
    DistributedVector(KVStore* kv, size_t size, Vector* keys) : 
        size_(size), current_(nullptr), keys_(keys), kv_(kv), k_(nullptr), kbuf_(nullptr),
        is_locked_(true) { }

    /** Destructor */
    ~DistributedVector() { 
        if (kbuf_ != nullptr) delete kbuf_;
        if (k_ != nullptr) delete k_;
        if (current_ != nullptr) delete current_;
        delete keys_;
    }

    /** Serializes the current chunk and puts it into the KVStore */
    void store_chunk_(size_t idx) {
        kbuf_->c("-");
        kbuf_->c(idx);
        Key* k = kbuf_->get(kv_->this_node());
        kv_->put(*k, current_->serialize());
        keys_->append(k);
        delete current_;
        current_ = nullptr;
    }
    
    // Appends val to the end of the vector.
    void append(DataType* val) {
        exit_if_not(!is_locked_, "This DVector is locked, no more fields can be added to it.");
        if (current_->size() == CHUNK_SIZE) {
            size_t idx = current_->idx();
            // The current chunk is full, so serialize it and put it in the KVStore
            store_chunk_(idx);
            // start a new chunk
            current_ = new Chunk(idx + 1);
        }
        current_->append(val);
        size_++;
    }
    
    // Gets the field at the given index.
    DataType* get(size_t index) {
        exit_if_not(is_locked_, "DVectors can only be queryed once all fields have been added.");
        assert(index < size_);
        // The index of the chunk in the vector
        size_t chunk_idx = index / CHUNK_SIZE;
        // The index of the field in the chunk
        size_t field_idx = index % CHUNK_SIZE;
        if (current_ == nullptr || current_->idx() != chunk_idx) {
            delete current_;
            // Retrieve the chunk from the KVStore
            Key* k = dynamic_cast<Key*>(keys_->get(chunk_idx));
            const char* serial_chunk = kv_->get(*k);
            Deserializer ds(serial_chunk);
            // The chunk is cached because it will likely be needed for the next get()
            current_ = ds.deserialize_chunk();
            delete[] serial_chunk;
        }
        // Retrieve the field from the chunk and clone it so we can delete the chunk later.
        DataType* res = current_->get(field_idx)->clone();
        return res;
    }
    
    // Returns the number of fields in this vector. 
    size_t size() {
        return size_;
    }

    /** Called when all fields have been added to this DVector */
    void done() {
        exit_if_not(!is_locked_, "DistVector is already locked");
        // Put the last chunk in the KVStore if it has any fields
        if (current_->size() > 0) store_chunk_(current_->idx());
        is_locked_ = true;
    }

    /** Returns a char* representation of this DVector */
    const char* serialize() {
        exit_if_not(is_locked_, "DistVector can only be serialized once all fields have been added");
        StrBuff sbuf;
        // Serialize the size
        Serializer ser;
        const char* serial_size = ser.serialize_int(size_);
        sbuf.c(serial_size);
        delete[] serial_size;
        // Serialize the keys
        sbuf.c("[");
        for (int i = 0; i < keys_->size(); i++) {
            const char* serial_key = keys_->get(i)->serialize();
            sbuf.c(serial_key);
            delete[] serial_key;
        }
        sbuf.c("]");
        return sbuf.c_str();
    }

    /** Getter for the list of keys */
    Vector* get_keys() { return keys_; }

    /** Is this DVector equal to the given object? */
    bool equals(Object* other) {
        exit_if_not(is_locked_, "DistVector can only be compared once all fields have been added");
        DistributedVector* o = dynamic_cast<DistributedVector*>(other);
        if (o == nullptr) return false;
        return size_ == o->size() && keys_->equals(o->get_keys());
    }
};