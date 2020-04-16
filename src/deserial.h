//lang:CwC

#pragma once

#include <assert.h>
#include <sys/socket.h>
#include "message.h"
#include "datatype.h"

class DataFrame; class Column; class DistributedVector; class KVStore; class Chunk;

/**
 * Helper class that handles deserializing objects of various types.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Deserializer : public Object {
public:
    const char* stream_;
    int i_; // current location in the stream
    char* x_;
    

    Deserializer(const char* stream) {
        stream_ = stream;
        i_ = 0;
        x_ = new char[sizeof(char) + 1];
        x_[1] = '\0';
    }

    ~Deserializer() {
        delete[] x_;
    }

    /* Returns the current character in the stream. */
    char current() {
        char rtrn = stream_[i_];
        return rtrn;
    }

    /* Return the current character in the stream and step forward by 1. */
    char step() {
        char rtrn = stream_[i_];
        i_++;
        return rtrn; 
    }

    /* Builds and returns an integer from the bytestream. */
    int deserialize_int() {
        StrBuff buff;
        assert(step() == '{');
        while (current() != '}') {
            *x_ = step();
            buff.c(x_);
        }
        assert(step() == '}');
        char* c_int = buff.c_str();
        int res = atoi(c_int);
        delete[] c_int;
        return res;
    }

    /* Builds and returns a float from the bytestream. */
    float deserialize_float() {
        StrBuff buff;
        assert(step() == '{');
        while (current() != '}') {
            *x_ = step();
            buff.c(x_);
        }
        assert(step() == '}');
        char* c_float = buff.c_str();
        float res = atof(c_float);
        delete[] c_float;
        return res;
    }

    /* Builds and returns a boolean from the bytestream. */
    bool deserialize_bool() {
        StrBuff buff;
        assert(step() == '{');
        while (current() != '}') {
            *x_ = step();
            buff.c(x_);
        }
        assert(step() == '}');
        char* c_bool = buff.c_str();
        bool res = atoi(c_bool);
        delete[] c_bool;
        return res;
    }

    /* Builds and returns an object from the bytestream. */
    Object* deserialize() {
        // stream's initial 6 chars should be "{type: "
        assert(step() == '{');
        assert(step() == 't');
        assert(step() == 'y');
        assert(step() == 'p');
        assert(step() == 'e');
        assert(step() == ':');
        assert(step() == ' ');

        // Fill a buffer with the type of object to create
        // either "object", "string", "vector" or "FloatVector".
        StrBuff buff;
        while (current() != ',' && current() != '}') {
            *x_ = step();
            buff.c(x_);
        }

        String* type_str = buff.get();
        char* type = type_str->c_str();

        if (strcmp(type, "object") == 0) {
            delete type_str;
            return new Object();
        } else if (strcmp(type, "ack") == 0) {
            delete type_str;
            return new Ack();
        } else if (strcmp(type, "directory") == 0) {
            delete type_str;
            return deserialize_directory();
        } else if (strcmp(type, "register") == 0) {
            delete type_str;
            return deserialize_register();
        } else if (strcmp(type, "put") == 0) {
            delete type_str;
            return deserialize_put();
        } else if ((strcmp(type, "get") == 0)) {
            delete type_str;
            return deserialize_get();
        } else if (strcmp(type, "wait_get") == 0) {
            delete type_str;
            return deserialize_wait_get();
        } else if (strcmp(type, "reply") == 0) {
            delete type_str;
            return deserialize_reply();
        } else if (strcmp(type, "string") == 0) {
            delete type_str;
            return deserialize_string();
        } else if (strcmp(type, "vector") == 0) {
            delete type_str;
            return deserialize_string_vector();
        } else if (strcmp(type, "float_vector") == 0) {
            delete type_str;
            return deserialize_float_vector();
        } else if (strcmp(type, "int_vector") == 0) {
            delete type_str;
            return deserialize_int_vector();
        } else if (strcmp(type, "bool_vector") == 0) {
            delete type_str;
            return deserialize_bool_vector();
        } else {
            exit_if_not(false, "Unknown type");
        }
    }

    /* Builds and returns a Key from the given bytestream. */
    // {type: key, key: {type: string, cstr: foo}, idx: 0}
    Key* deserialize_key() {
        String* key_str = dynamic_cast<String*>(deserialize());
        assert(key_str != nullptr);
        int idx = deserialize_int();
        const char* key = key_str->c_str();
        Key* rtrn = new Key(key, idx);
        delete key_str;
        return rtrn;
    }        

    /* Builds and returns a Directory from the given bytestream. */
    Directory* deserialize_directory() {
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'a');
        assert(step() == 'd');
        assert(step() == 'd');
        assert(step() == 'r');
        assert(step() == 'e');
        assert(step() == 's');
        assert(step() == 's');
        assert(step() == 'e');
        assert(step() == 's');
        assert(step() == ':');
        assert(step() == ' ');
        Vector* addresses = dynamic_cast<Vector*>(deserialize());
        assert(addresses != nullptr);

        assert(step() == ']');
        assert(step() == '}');
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'i');
        assert(step() == 'n');
        assert(step() == 'd');
        assert(step() == 'i');
        assert(step() == 'c');
        assert(step() == 'e');
        assert(step() == 's');
        assert(step() == ':');
        assert(step() == ' ');
        IntVector* indices = dynamic_cast<IntVector*>(deserialize());
        assert(indices != nullptr);

        return new Directory(addresses, indices);
    }

    /* Builds and returns a Register from the given bytestream. */
    Register* deserialize_register() {
        
        // Selecting the sin_family, sin_port and sin_addr fields from the bytestream.
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'i');
        assert(step() == 'p');
        assert(step() == ':');
        assert(step() == ' ');
        String* ip = dynamic_cast<String*>(deserialize());
        assert(ip != nullptr);
        
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 's');
        assert(step() == 'e');
        assert(step() == 'n');
        assert(step() == 'd');
        assert(step() == 'e');
        assert(step() == 'r');
        assert(step() == ':');
        assert(step() == ' ');
        StrBuff buff;
        while (current() != '}') {
            *x_ = step();
            buff.c(x_);
        }
        String* sender_str = buff.get();
        size_t sender = atoi(sender_str->c_str());
        delete sender_str;
        return new Register(ip, sender);
    }

    /* Builds and returns a Put message from the given bytestream. */
    Put* deserialize_put() {
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'k');
        assert(step() == 'e');
        assert(step() == 'y');
        assert(step() == ':');
        assert(step() == ' ');
        Key* k = deserialize_key();
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'v');
        assert(step() == 'a');
        assert(step() == 'l');
        assert(step() == 'u');
        assert(step() == 'e');
        assert(step() == ':');
        assert(step() == ' ');
        // Extract the blob of serialized data
        StrBuff buff;
        while (current() != '\n') {
            *x_ = step();
            buff.c(x_);
        }
        assert(step() == '\n');
        return new Put(k, buff.c_str());
    }

    /* BUilds and returns a Get message from the given bytestream. */
    //e.g.: {type: get, key: {type: key, key: {type: string, cstr: foo}, idx: 0}}
    Get* deserialize_get() {
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'k');
        assert(step() == 'e');
        assert(step() == 'y');
        assert(step() == ':');
        assert(step() == ' ');
        Key* k = deserialize_key();
        assert(step() == '}');
        return new Get(k);
    }

    /* BUilds and returns a WaitAndGet message from the given bytestream. */
    //e.g.: {type: wait_get, key: {type: key, key: {type: string, cstr: foo}, idx: 0}}
    WaitAndGet* deserialize_wait_get() {
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'k');
        assert(step() == 'e');
        assert(step() == 'y');
        assert(step() == ':');
        assert(step() == ' ');
        Key* k = deserialize_key();
        assert(step() == '}');
        
        WaitAndGet* rtrn = new WaitAndGet(k);
        return rtrn;
    }

    /* Builds and returns a Reply message from the given bytestream. */
    Reply* deserialize_reply() {
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'r');
        assert(step() == 'e');
        assert(step() == 'q');
        assert(step() == 'u');
        assert(step() == 'e');
        assert(step() == 's');
        assert(step() == 't');
        assert(step() == ':');
        assert(step() == ' ');
        MsgKind req = (MsgKind)deserialize_int();
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'v');
        assert(step() == 'a');
        assert(step() == 'l');
        assert(step() == 'u');
        assert(step() == 'e');
        assert(step() == ':');
        assert(step() == ' ');
        // Extract the serialized data
        StrBuff buff;
        while (current() != '\n') {
            *x_ = step();
            buff.c(x_);
        }
        assert(step() == '\n');
        return new Reply(buff.c_str(), req);
    }

    /* Builds and returns a String from the given bytestream. */
    String* deserialize_string() {
        StrBuff buff;
        // Sellecing the c_str
        assert(step() == ',');
        assert(step() == ' '); 
        assert(step() == 'c'); 
        assert(step() == 's'); 
        assert(step() == 't'); 
        assert(step() == 'r'); 
        assert(step() == ':');
        assert(step() == ' ');  
        // Iterating over the string characters.
        while (current() != '}') {
            *x_ = step();
            buff.c(x_);
        }
        assert(step() == '}');

        String* deserial_str = buff.get();
        return deserial_str;
    }

    /* Builds and returns an vector representation of the given bytestream. 
    *  Our Vector class can hold objects of all kinds, but here we are deserializing one that
    *  only holds strings.
    *  Create an vector, append strings from the stream to it and return it.
    */
    Vector* deserialize_string_vector() {
        assert(step() == ',');
        assert(step() == ' '); 
        assert(step() == 'o'); 
        assert(step() == 'b'); 
        assert(step() == 'j'); 
        assert(step() == 'e'); 
        assert(step() == 'c'); 
        assert(step() == 't'); 
        assert(step() == 's'); 
        assert(step() == ':'); 
        assert(step() == ' '); 
        assert(step() == '[');
        Vector* vec = new Vector();
        while (current() != ']') {
            String* element = dynamic_cast<String*>(deserialize());
            assert(element != nullptr);
            vec->append(element);
            if (current() == ',') { step(); } // Step over the ','
        }
        return vec;
    }

    /* Builds and returns a FloatVector from the bytestream
    *  Create a new FlaotVector(), fill it with floats from the stream and return it.
    * */
    FloatVector* deserialize_float_vector() {
        StrBuff buff;
        // stream's initial 11 chars should be ", floats: ["
        assert(step() == ','); 
        assert(step() == ' '); 
        assert(step() == 'f'); 
        assert(step() == 'l'); 
        assert(step() == 'o'); 
        assert(step() == 'a'); 
        assert(step() == 't'); 
        assert(step() == 's'); 
        assert(step() == ':'); 
        assert(step() == ' '); 
        assert(step() == '['); // Now iterating over vector elements.

        // New FloatVector we will return once we have filled it with corresponding floats
        FloatVector* fvec = new FloatVector();
        while (current() != ']') {
            fvec->append(deserialize_float());
            if (current() == ',') { step(); } // Step over the ','
        }
        return fvec;
    }

    /* Builds and returns an IntVector from the bytestream */
    IntVector* deserialize_int_vector() {
        StrBuff buff;

        // stream's initial 9 chars should be ", ints: ["
        assert(step() == ','); 
        assert(step() == ' '); 
        assert(step() == 'i'); 
        assert(step() == 'n'); 
        assert(step() == 't'); 
        assert(step() == 's'); 
        assert(step() == ':'); 
        assert(step() == ' '); 
        assert(step() == '[');  // Now iterating over vector elements.

        // New IntVector we will return once we have filled it with corresponding ints
        IntVector* ivec = new IntVector();
        while (current() != ']') {
            ivec->append(deserialize_int());
            if (current() == ',') { step(); } // Step over the ','
        }
        return ivec;
    }

    /* Builds and returns an BoolVector from the bytestream */
    BoolVector* deserialize_bool_vector() {
        StrBuff buff;
        // stream's initial 9 chars should be ", bools: ["
        assert(step() == ',');
        assert(step() == ' ');
        assert(step() == 'b');
        assert(step() == 'o');
        assert(step() == 'o');
        assert(step() == 'l');
        assert(step() == 's');
        assert(step() == ':');
        assert(step() == ' ');
        assert(step() == '['); // Now iterating over vector elements.

        // New BoolVector we will return once we have filled it with corresponding bools
        BoolVector* bvec = new BoolVector();
        while (current() != ']') {
            bvec->append(deserialize_bool());
            if (current() == ',') { step(); } // Step over the ','
        }
        return bvec;
    }

    /** Builds and returns a DataType from the bytestream. */
    DataType* deserialize_datatype() {
        char type = step();
        DataType* dt = new DataType();
        switch (type) {
            case 'I':
                dt->set_int(deserialize_int()); break;
            case 'B':
                dt->set_bool(deserialize_bool()); break;
            case 'F':
                dt->set_float(deserialize_float()); break;
            case 'S':
                dt->set_string(dynamic_cast<String*>(deserialize())); break;
        }
        return dt;
    }

    /** Builds and returns a Chunk from the bytestream. */
    Chunk* deserialize_chunk();

    /** Builds and returns a DistributedVector from the bytestream. */
    DistributedVector* deserialize_dist_vector(KVStore* kv);

    /** Builds and returns a Column from the bytestream. */
    Column* deserialize_column(KVStore* kv);

    /** Builds and returns a DataFrame from the bytestream. */
    DataFrame* deserialize_dataframe(KVStore* kv, Key* k);
};