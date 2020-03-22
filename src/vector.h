#pragma once

#include "string.h"
#include <stdbool.h>
#include <assert.h>

#define INITIAL_CHUNK_CAPACITY 16
#define CHUNK_SIZE 8

/**
 * Represents an vector (Java: ArrayList) of objects.
 * In order to have constant time lookup and avoid copying the payload of the 
 * array, this data structure is an array of pointers to chunks of objects. It 
 * is essentially a 2D array. The chunks have a fixed length and when the array 
 * of chunks runs out of space, a new array is allocated with more memory and 
 * the chunk pointers are transferred to it.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Vector : public Object {
    public:
        Object*** objects_;   
        int size_;
        // Number of chunks that we have space for
        int chunk_capacity_; 
        // Number of chunks that have been initialized
        int chunk_count_;
    
        /**
         * Initialize an empty Vector.
         */
        Vector() {
            size_ = 0;
            chunk_capacity_ = INITIAL_CHUNK_CAPACITY;
            chunk_count_ = 1;
            objects_ = new Object**[chunk_capacity_];
            objects_[0] = new Object*[CHUNK_SIZE];
            // Initialize all values in the new chunk to default value nullptr
            for (int i = 0; i < CHUNK_SIZE; i++) {
                objects_[0][i] = nullptr;
            }
            // Initialize all other chunk uninitialized chunks to default value nullptr
            for (int i = 1; i < chunk_capacity_; i++) {
                objects_[i] = nullptr;
            }
        }

        /**
         * Destructor for a Vector
         */
        ~Vector() {
            // Delete each chunk
            for (int i = 0; i < chunk_capacity_; i++) {
                if (objects_[i] != nullptr) {
                    // Delete each object in the chunk
                    for (int j = 0; j < CHUNK_SIZE; j++) {
                        if (objects_[i][j] != nullptr) delete objects_[i][j];
                    }
                    delete[] objects_[i];
                }
            }
            // Delete the array that holds the chunks
            delete[] objects_;
        }

        /**
         * Private function that reallocates more space for the vector
         * once it fills up.
         */
        void reallocate_() {
            Object*** new_outer_arr = new Object**[chunk_capacity_ + 
                INITIAL_CHUNK_CAPACITY];

            Object*** old_outer_arr = objects_;
            for (int i = 0; i < chunk_count_; i++) {
                new_outer_arr[i] = old_outer_arr[i];
            }
            delete[] old_outer_arr;
            objects_ = new_outer_arr;

            chunk_capacity_ += INITIAL_CHUNK_CAPACITY;
            // Initialize all other uninitialized chunks to default value nullptr
            for (int i = chunk_count_; i < chunk_capacity_; i++) {
                objects_[i] = nullptr;
            }
        }
        
        // Appends val to the end of the vector. Takes control of the val.
        void append(Object* val) {
            // If all of the chunks are full, allocate more memory for the outer array.
            if (size_ + 1 > chunk_capacity_ * CHUNK_SIZE) reallocate_();
            // If the last chunk is full, initialize a new one and add val to it.
            if (size_ + 1 > chunk_count_ * CHUNK_SIZE) {
                objects_[chunk_count_] = new Object*[CHUNK_SIZE];
                objects_[chunk_count_][0] = val;
                // Initialize all values in the new chunk to default value nullptr
                for (int i = 1; i < CHUNK_SIZE; i++) {
                    objects_[chunk_count_][i] = nullptr;
                }
                chunk_count_++;
            } else {
                objects_[chunk_count_ - 1][size_ % CHUNK_SIZE] = val;
            }
            size_++;
        }
        
        // Appends a clone of every element of vals to the end of the vector.
        // If vals is null, does nothing.
        void append_all(Vector* vals) {
            if (vals == NULL) return;
            for (int i = 0; i < vals->size(); i++) {
                Object* val = vals->get(i);
                if (val == nullptr) {
                    append(nullptr);
                } else {
                    append(val->clone());
                }
            }
        }
        
        // Sets the element at index to val.
        // If index == size(), appends to the end of the vector.
        void set(Object* val, size_t index) {
            assert(index >= 0);
            assert(index <= size_);

            if (index == size_) {
                append(val);
                return;
            }

            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            // Delete the object at this index if there is one
            Object* replace_me = dynamic_cast<Object*>(objects_[outer_idx][inner_idx]);
            if (replace_me != nullptr) {
                delete replace_me;
            }
            objects_[outer_idx][inner_idx] = val;
        }
        
        // Gets the element at the given index.
        Object* get(size_t index) {
            assert(index >= 0);
            assert(index < size_);
            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            return objects_[outer_idx][inner_idx];
        }
        
        // Returns if this vector contains obj, using obj->equals().
        // If obj is null, uses == .
        bool contains(Object* obj) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    int inner_idx = j % CHUNK_SIZE;
                    if (obj == NULL || objects_[i][inner_idx] == NULL) {
                        if (objects_[i][inner_idx] == obj) return true;
                    } else {
                        if (objects_[i][inner_idx]->equals(obj)) return true;
                    }
                    count++;
                }
            }
            return false;
        }
        
        // Returns the first index of obj, using obj->equals().
        // If obj is null, uses == .
        // If obj does not exist in the vector, returns -1.
        size_t index_of(Object* obj) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    int inner_idx = j % CHUNK_SIZE;
                    if (obj == NULL || objects_[i][inner_idx] == NULL) {
                        if (objects_[i][inner_idx] == obj) return count;
                    } else {
                        if (objects_[i][inner_idx]->equals(obj)) return count;
                    }
                    count++;
                } 
            }
            return -1;
        }
        
        // Returns the number of elements in this vector. 
        size_t size() {
            return size_;
        }

        // Inherited from Object
        // Is this Vector equal to the given Object?
        bool equals(Object* o) {
            Vector* other = dynamic_cast<Vector*>(o);
            if (other == nullptr) return false;
            if (size_ == 0) return other->size() == 0;

            for (int i = 0; i < size_; i++) {
                String* s = dynamic_cast<String*>(get(i));
                String* s_other = dynamic_cast<String*>(other->get(i));
                if (!(s->equals(s))) return false;  // String equality, assuming Vector takes only strings.
            }

            return true;
        }

        /**
         * Returns a serialized version of this Vector as a char*
         */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: vector, objects: [");
            // Appending serialized strings in the array to buff
            for (int i = 0; i < size_; i++) {
                buff.c(get(i)->serialize());
                if (i < size_ - 1) {
                     buff.c(",");
                }
            }
            buff.c("]}");
            return buff.get()->c_str();
        }
};

/**
 * Represents an vector (Java: ArrayList) of booleans.
 * In order to have constant time lookup and avoid copying the payload of the 
 * array, this data structure is an array of pointers to chunks of objects. It 
 * is essentially a 2D array. The chunks have a fixed length and when the array 
 * of chunks runs out of space, a new array is allocated with more memory and 
 * the chunk pointers are transferred to it.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class BoolVector : public Object {
    public:
        bool** bools_;
        int size_;
        // Number of chunks that we have space for
        int chunk_capacity_; 
        // Number of chunks that have been initialized
        int chunk_count_;

        /**
         * Constructor for a BoolVector.
         * 
        */ 
        BoolVector() {
            size_ = 0;
            chunk_capacity_ = INITIAL_CHUNK_CAPACITY;
            chunk_count_ = 1;
            bools_ = new bool*[chunk_capacity_];
            bools_[0] = new bool[CHUNK_SIZE];
            // Initialize all other uninitialized chunks to default value nullptr
            for (int i = 1; i < chunk_capacity_; i++) {
                bools_[i] = nullptr;
            }
        } 

        /**
         * Destructor for a BoolVector.
         */ 
        ~BoolVector() {
            // Delete each chunk
            for (int i = 0; i < chunk_capacity_; i++) {
                if (bools_[i] != nullptr) delete[] bools_[i];
            }
            // Delete the array that holds the chunks
            delete[] bools_;
        }

        /*
         * Private function that reallocates more space for the vector
         * once it fills up.
         */
        void reallocate_() {
            bool** new_outer_arr = new bool*[chunk_capacity_ + 
                INITIAL_CHUNK_CAPACITY];

            bool** old_outer_arr = bools_;
            for (int i = 0; i < chunk_count_; i++) {
                new_outer_arr[i] = old_outer_arr[i];
            }
            delete[] old_outer_arr;
            bools_ = new_outer_arr;

            chunk_capacity_ += INITIAL_CHUNK_CAPACITY;
            // Initialize all other uninitialized chunks to default value nullptr
            for (int i = chunk_count_; i < chunk_capacity_; i++) {
                bools_[i] = nullptr;
            }
        }
    
        // Appends val onto the end of the vector
        void append(bool val) {
            // If all of the chunks are full, allocate more memory for the outer array.
            if (size_ + 1 > chunk_capacity_ * CHUNK_SIZE) reallocate_();
            // If the last chunk is full, initialize a new one and add val to it.
            if (size_ + 1 > chunk_count_ * CHUNK_SIZE) {
                bools_[chunk_count_] = new bool[CHUNK_SIZE];
                bools_[chunk_count_][0] = val;
                chunk_count_++;
            } else {
                bools_[chunk_count_ - 1][size_ % CHUNK_SIZE] = val;
            }
            size_++;
        }
        
        // Appends every element of vals to the end of the vector.
        // If vals is null, does nothing.
        void append_all(BoolVector* vals) {
            if (vals == NULL) return;
            for (int i = 0; i < vals->size(); i++) {
                append(vals->get(i));
            }
        }
        
        // Sets the element at index to val.
        // If index == size(), appends to the end of the vector.
        void set(bool val, size_t index) { 
            assert(index >= 0);
            assert(index <= size_);

            if (index == size_) {
                append(val);
                return;
            }

            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            bools_[outer_idx][inner_idx] = val;
        }
        
        // Gets the element at the given index.
        bool get(size_t index) { 
            assert(index >= 0);
            assert(index < size_);
            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            return bools_[outer_idx][inner_idx];
        }
        
        // Returns if the vector contains val.
        bool contains(bool val) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    if (bools_[i][j % CHUNK_SIZE] == val) return true;
                }
                count++;
            }
            return false;
        }
        
        // Returns the first index of the val.
        // If val does not exist in the vector, returns -1.
        size_t index_of(bool val) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    if (bools_[i][j % CHUNK_SIZE] == val) return count;
                }
                count++;
            }
            return -1;
        }
        
        // Returns the number of elements.
        size_t size() {
            return size_;
        }

        // Inherited from Object
        // Is this BoolVector equal to the given Object?
        bool equals(Object* o) {
            BoolVector* other = dynamic_cast<BoolVector*>(o);
            if (other == nullptr) return false;
            if (size_ == 0) return other->size() == 0;

            for (int i = 0; i < size_; i++) {
                bool a = get(i);
                bool b = other->get(i);
                if (!(a== b)) return false;
            }

            return true;
        }

        /** 
         * Returns a serialized representation of this bool vector.
         */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: bool_vector, bools: [");
            for (int i = 0; i < size_; i++) {
                bool b = get(i);
                if (b) {
                    buff.c("true");
                } else {
                    buff.c("false");
                }
                if (i < size_ - 1) {
                     buff.c(",");
                }
            }
            buff.c("]}");
            return buff.get()->c_str();
        }
};

/**
 * Represents an vector (Java: ArrayList) of integers.
 * In order to have constant time lookup and avoid copying the payload of the 
 * array, this data structure is an array of pointers to chunks of objects. It 
 * is essentially a 2D array. The chunks have a fixed length and when the array 
 * of chunks runs out of space, a new array is allocated with more memory and 
 * the chunk pointers are transferred to it.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class IntVector : public Object {
    public:
        int** ints_;
        int size_;
        // Number of chunks that we have space for
        int chunk_capacity_; 
        // Number of chunks that have been initialized
        int chunk_count_;

        /**
         * Constructor for an IntVector.
         * 
        */ 
        IntVector() {
            size_ = 0;
            chunk_capacity_ = INITIAL_CHUNK_CAPACITY;
            chunk_count_ = 1;
            ints_ = new int*[chunk_capacity_];
            ints_[0] = new int[CHUNK_SIZE];
            // Initialize all other uninitialized chunks to default value nullptr
            for (int i = 1; i < chunk_capacity_; i++) {
                ints_[i] = nullptr;
            }
        } 

        /**
         * Destructor for an IntVector.
         */ 
        ~IntVector() {
            // Delete each chunk
            for (int i = 0; i < chunk_capacity_; i++) {
                if (ints_[i] != nullptr) delete[] ints_[i];
            }
            // Delete the array that holds the chunks
            delete[] ints_;
        }

        /*
         * Private function that reallocates more space for the IntVector
         * once it fills up.
         */
        void reallocate_() {
            int** new_outer_arr = new int*[chunk_capacity_ + 
                INITIAL_CHUNK_CAPACITY]; 

            int** old_outer_arr = ints_;
            for (int i = 0; i < chunk_count_; i++) {
                new_outer_arr[i] = old_outer_arr[i];
            }
            delete[] old_outer_arr;
            ints_ = new_outer_arr;

            chunk_capacity_ += INITIAL_CHUNK_CAPACITY;
            // Initialize all other uninitialized chunks to default value nullptr
            for (int i = chunk_count_; i < chunk_capacity_; i++) {
                ints_[i] = nullptr;
            }
        }
        
        // Appends val onto the end of the vector
        void append(int val) {
            // If all of the chunks are full, allocate more memory for the outer array.
            if (size_ + 1 > chunk_capacity_ * CHUNK_SIZE) reallocate_();
            // If the last chunk is full, initialize a new one and add val to it.
            if (size_ + 1 > chunk_count_ * CHUNK_SIZE) {
                ints_[chunk_count_] = new int[CHUNK_SIZE];
                ints_[chunk_count_][0] = val;
                chunk_count_++;
            } else {
                ints_[chunk_count_ - 1][size_ % CHUNK_SIZE] = val;
            }
            size_++;
        }
        
        // Appends every element of vals to the end of the vector.
        // If vals is null, does nothing.
        void append_all(IntVector* vals) {
            if (vals == NULL) return;
            for (int i = 0; i < vals->size(); i++) {
                append(vals->get(i));
            }
        }
        
        // Sets the element at index to val.
        // If index == size(), appends to the end of the vector.
        void set(int val, size_t index) {
            assert(index >= 0);
            assert(index <= size_);

            if (index == size_) {
                append(val);
                return;
            }

            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            ints_[outer_idx][inner_idx] = val;
        }
        
        // Gets the element at index.
        // If index is >= size(), does nothing and returns undefined.
        int get(size_t index) {
            assert(index >= 0);
            assert(index < size_);
            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            return ints_[outer_idx][inner_idx];
        }
        
        // Returns if the vector contains val.
        bool contains(int val) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    if (ints_[i][j % CHUNK_SIZE] == val) return true;
                }
                count++;
            }
            return false;
        }
        
        
        // Returns the first index of the val.
        // If val does not exist in the vector, returns -1.
        size_t index_of(int val) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    if (ints_[i][j % CHUNK_SIZE] == val) return count;
                }
                count++;
            }
            return -1;
        }
        
        // Returns the number of elements.
        size_t size() {
            return size_;
        }

        // Inherited from Object
        // Is this IntVector equal to the given Object?
        bool equals(Object* o) {
            IntVector* other = dynamic_cast<IntVector*>(o);
            if (other == nullptr) return false;
            if (size_ == 0) return other->size() == 0;

            for (int i = 0; i < size_; i++) {
                if (!(get(i) == other->get(i))) return false;
            }

            return true;
        }

        /** 
         * Returns a serialized representation of this int vector.
         */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: int_vector, ints: [");
            for (int i = 0; i < size_; i++) {
                int x = get(i);
                buff.c(int_to_char(x));
                if (i < size_ - 1) {
                     buff.c(",");
                }
            }
            buff.c("]}");
            return buff.get()->c_str();
        }

        /** 
         * Converts an int to a char*
         */
        char* int_to_char(int i) {
            // The size of the buffer that will hold the float
            size_t len = 5;
            char* c_int = new char[len]; 
            int ret = snprintf(c_int, len, "%d", i);
            exit_if_not(ret >= 0, "snprintf failed");
            while (ret >= len) {
                // The float was too large for the buffer, so increase its size and try again
                len += 5;
                ret = snprintf(c_int, len, "%d", i);
                exit_if_not(ret >= 0, "snprintf failed");
            }
            return c_int;
        }
};

/**
 * Represents an vector (Java: ArrayList) of floats.
 * In order to have constant time lookup and avoid copying the payload of the 
 * array, this data structure is an array of pointers to chunks of objects. It 
 * is essentially a 2D array. The chunks have a fixed length and when the array 
 * of chunks runs out of space, a new array is allocated with more memory and 
 * the chunk pointers are transferred to it.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class FloatVector : public Object {
    public:
        float** floats_;
        int size_;
        // Number of chunks that we have space for
        int chunk_capacity_; 
        // Number of chunks that have been initialized
        int chunk_count_;

        /**
         * Constructor for a FloatVector.
         * 
        */ 
        FloatVector() {
            size_ = 0;
            chunk_capacity_ = INITIAL_CHUNK_CAPACITY;
            chunk_count_ = 1;
            floats_ = new float*[chunk_capacity_];
            floats_[0] = new float[CHUNK_SIZE];
            // Initialize all other uninitialized chunks to default value nullptr
            for (int i = 1; i < chunk_capacity_; i++) {
                floats_[i] = nullptr;
            }
        }

        /**
         * Destructor for a FloatVector.
         * 
        */ 
        ~FloatVector() {
            // Delete each chunk
            for (int i = 0; i < chunk_capacity_; i++) {
                if (floats_[i] != nullptr) delete[] floats_[i];
            }
            // Delete the array that holds the chunks
            delete[] floats_;
        }
        
        /*
         * Private function that reallocates more space for the FloatVector
         * once it fills up.
         */
        void reallocate_() {
            float** new_outer_arr = new float*[chunk_capacity_ + INITIAL_CHUNK_CAPACITY];

            float** old_outer_arr = floats_;
            for (int i = 0; i < chunk_count_; i++) {
                new_outer_arr[i] = old_outer_arr[i];
            }
            delete[] old_outer_arr;
            floats_ = new_outer_arr;

            chunk_capacity_ += INITIAL_CHUNK_CAPACITY;
            // Initialize all other uninitialized chunks to default value nullptr
            for (int i = chunk_count_; i < chunk_capacity_; i++) {
                floats_[i] = nullptr;
            }
        }

        // Appends val onto the end of the vector
        void append(float val) {
            // If all of the chunks are full, allocate more memory for the outer array.
            if (size_ + 1 > chunk_capacity_ * CHUNK_SIZE) reallocate_();
            // If the last chunk is full, initialize a new one and add val to it.
            if (size_ + 1 > chunk_count_ * CHUNK_SIZE) {
                floats_[chunk_count_] = new float[CHUNK_SIZE];
                floats_[chunk_count_][0] = val;
                chunk_count_++;
            } else {
                floats_[chunk_count_ - 1][size_ % CHUNK_SIZE] = val;
            }
            size_++;
        }
        
        // Appends every element of vals to the end of the vector.
        // If vals is null, does nothing.
        void append_all(FloatVector* vals) {
            if (vals == NULL) return;
            for (int i = 0; i < vals->size(); i++) {
                append(vals->get(i));
            }
        }
        
        // Sets the element at index to val.
        // If index == size(), appends to the end of the vector.
        void set(float val, size_t index) {
            assert(index >= 0);
            assert(index <= size_);

            if (index == size_) {
                append(val);
                return;
            }

            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            floats_[outer_idx][inner_idx] = val;
        }
        
        // Gets the element at index.
        float get(size_t index) {
            assert(index >= 0);
            assert(index < size_);
            int outer_idx = index / CHUNK_SIZE;
            int inner_idx = index % CHUNK_SIZE;
            return floats_[outer_idx][inner_idx];
        }
        
        // Returns if the vector contains val.
        bool contains(float val) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    if (floats_[i][j % CHUNK_SIZE] == val) return true;
                }
                count++;
            }
            return false;
        }
        
        // Returns the first index of the val.
        // If val does not exist in the vector, 
        //    returns a value greater than size().
        size_t index_of(float val) {
            // count is the total number of items read in the vector so far
            int count = 0;
            for (int i = 0; i < chunk_count_ && count < size_; i++) {
                for (int j = 0; j < CHUNK_SIZE && count < size_; j++) {
                    if (floats_[i][j % CHUNK_SIZE] == val) return count;
                }
                count++;
            }
            return -1;
        }
        
        // Returns the number of elements.
        size_t size() {
            return size_;
        }

        // Inherited from Object
        // Is this FloatVector equal to the given Object?
        bool equals(Object* o) {
            FloatVector* other = dynamic_cast<FloatVector*>(o);
            if (other == nullptr) return false;
            if (size_ == 0) return other->size() == 0;

            for (int i = 0; i < size_; i++) {
                float x = get(i);
                float y = other->get(i);
                if (!(get(i) == other->get(i))) return false;
            }

            return true;
        }

        /** 
         * Returns a serialized representation of this float vector.
         */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: float_vector, floats: [");
            for (int i = 0; i < size_; i++) {
                float f = get(i);
                buff.c(float_to_char(f));
                if (i < size_ - 1) {
                     buff.c(",");
                }
            }
            buff.c("]}");

            return buff.get()->c_str();
        }

        /** 
         * Converts a float to a char*
         */
        char* float_to_char(float f) {
            // The size of the buffer that will hold the float
            size_t len = 10;
            char* c_float = new char[len]; 
            int ret = snprintf(c_float, len, "%.7f", f);
            exit_if_not(ret >= 0, "snprintf failed");
            while (ret >= len) {
                // The float was too large for the buffer, so increase its size and try again
                len += 10;
                ret = snprintf(c_float, len, "%.7f", f);
                exit_if_not(ret >= 0, "snprintf failed");
            }
            return c_float;
        }
};