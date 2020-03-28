//lang:CwC

#pragma once

#include <assert.h>
#include <sys/socket.h>
#include "dataframe.h"
#include "message.h"

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

        /* Builds and returns an object from the given bytestream. */
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
            StrBuff* buff = new StrBuff();
            while (current() != ',' && current() != '}') {
                *x_ = step();
                buff->c(x_);
            }

            String* type_str = buff->get();
            char* type = type_str->c_str();
            delete buff;

            if (strcmp(type, "object") == 0) {
                delete type_str;
                return new Object();
            } else if (strcmp(type, "ack") == 0) {
                delete type_str;
                return new Ack();
            } else if (strcmp(type, "key") == 0) {
                delete type_str;
                return deserialize_key();
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
            } else if (strcmp(type, "int_column") == 0) {
                delete type_str;
                return deserialize_int_column();
            } else if (strcmp(type, "bool_column") == 0) {
                delete type_str;
                return deserialize_bool_column();
            } else if (strcmp(type, "float_column") == 0) {
                delete type_str;
                return deserialize_float_column();
            } else if (strcmp(type, "string_column") == 0) {
                delete type_str;
                return deserialize_string_column();
            } else if (strcmp(type, "dataframe") == 0) {
                delete type_str;
                return deserialize_dataframe();
            } else {
                exit_if_not(false, "Unknown type");
            }
        }

        /* Builds and returns a Key from the given bytestream. */
        // {type: key, key: {type: string, cstr: foo}, idx: 0}
        Key* deserialize_key() {
            assert(step() == ',');
            assert(step() == ' ');
            assert(step() == 'k');
            assert(step() == 'e');
            assert(step() == 'y');
            assert(step() == ':');
            assert(step() == ' ');
            String* key_str = dynamic_cast<String*>(deserialize());
            assert(key_str != nullptr);
            assert(step() == ',');
            assert(step() == ' ');
            assert(step() == 'i');
            assert(step() == 'd');
            assert(step() == 'x');
            assert(step() == ':');
            assert(step() == ' ');

            // Fill a buffer with 
            StrBuff buff;
            while (current() != ',' && current() != '}') {
                *x_ = step();
                buff.c(x_);
            }
            assert(step() == '}');

            String* idx_str = buff.get();
            int idx = atoi(idx_str->c_str());
            const char* key = key_str->c_str();
            Key* rtrn = new Key(key, idx);

            delete idx_str;
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
            Key* k = dynamic_cast<Key*>(deserialize());
            assert(k != nullptr);
            assert(step() == ',');
            assert(step() == ' ');
            assert(step() == 'v');
            assert(step() == 'a');
            assert(step() == 'l');
            assert(step() == 'u');
            assert(step() == 'e');
            assert(step() == ':');
            assert(step() == ' ');
            DataFrame* v = dynamic_cast<DataFrame*>(deserialize());
            assert(v != nullptr);
            assert(step() == ']');
            assert(step() == '}');
            assert(step() == '}');

            Put* rtrn = new Put(k, v);
            
            return rtrn;
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
            Key* k = dynamic_cast<Key*>(deserialize());
            assert(k != nullptr);
            assert(step() == '}');
            
            Get* rtrn = new Get(k);
            return rtrn;
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
            Key* k = dynamic_cast<Key*>(deserialize());
            assert(k != nullptr);
            assert(step() == '}');
            
            WaitAndGet* rtrn = new WaitAndGet(k);
            return rtrn;
        }

        /* Builds and returns a String from the given bytestream. */
        String* deserialize_string() {
            StrBuff* buff = new StrBuff();
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
                buff->c(x_);
            }
            assert(step() == '}');

            String* deserial_str = buff->get();
            delete buff;
            return deserial_str;
        }

        /* Builds and returns an vector representation of the given bytestream. 
        *  Even if this is an vector containing object elements, only strings can be deserialized,
        *  so we assume the return vector is an vector of strings. 
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

            // We use this vector class for all types of objects but, for the scope 
            // of this prototype, only string objects are deserializable, so we have 
            // an assert to check if it is possible to cast object elements to string.
            Vector* vec = new Vector();
            while (current() != ']') {
                String* element = dynamic_cast<String*>(deserialize());
                vec->append(element);
                if (current() == ',') { step(); } // Step over the ','
            }
            return vec;
        }

        /* Builds and returns a FloatVector from the bytestream
        *  Create a new FlaotVector(), fill it with floats from the stream and return it.
        * */
        FloatVector* deserialize_float_vector() {
            StrBuff* buff = new StrBuff();
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
                while (current() != ',' && current() != ']') {
                    *x_ = step();
                    buff->c(x_);
                }
                String* float_str = buff->get();
                float f = atof(float_str->c_str()); // Deserialized chr to a float.
                fvec->append(f);
                delete float_str;
                delete buff;
                if (current() == ',') {
                    step();
                    buff = new StrBuff();
                };
            }
            return fvec;
        }

        /* Builds and returns an IntVector from the bytestream */
        IntVector* deserialize_int_vector() {
            StrBuff* buff = new StrBuff();

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
                while (current() != ',' && current() != ']') {
                    *x_ = step();
                    buff->c(x_);
                }
                String* int_str = buff->get();
                int val = atoi(int_str->c_str()); // Deserialized chr to an int.
                ivec->append(val);
                delete int_str;
                delete buff;
                if (current() == ',') {
                    step();
                    buff = new StrBuff();
                };
            }
            return ivec;
        }

        /* Builds and returns an BoolVector from the bytestream */
        BoolVector* deserialize_bool_vector() {
            StrBuff* buff = new StrBuff();
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
                while (current() != ',' && current() != ']') {
                    *x_ = step();
                    buff->c(x_);
                }
                String* bool_str = buff->get();
                if (strcmp(bool_str->c_str(), "true") == 0) {
                    bvec->append(true);
                } else if (strcmp(bool_str->c_str(), "false") == 0) {
                    bvec->append(false);
                }
                delete bool_str;
                delete buff;
                if (current() == ',') {
                    step();
                    buff = new StrBuff();
                };
            }
            return bvec;
        }

        /* Builds and returns a BoolColumn from the bytestream */
        BoolColumn* deserialize_bool_column() {
            assert(step() == ',');
            assert(step() == ' ');
            assert(step() == 'd');
            assert(step() == 'a');
            assert(step() == 't');
            assert(step() == 'a');
            assert(step() == ':');
            assert(step() == ' ');
            BoolVector* bvec = dynamic_cast<BoolVector*>(deserialize());
            BoolColumn* bcol = new BoolColumn(bvec);
            return bcol;
        }

        /* Deserialize an IntColumn from the bytestream */
        IntColumn* deserialize_int_column() {
            assert(step() == ','); 
            assert(step() == ' '); 
            assert(step() == 'd'); 
            assert(step() == 'a'); 
            assert(step() == 't'); 
            assert(step() == 'a'); 
            assert(step() == ':'); 
            assert(step() == ' ');
            IntVector* ivec = dynamic_cast<IntVector*>(deserialize());
            IntColumn* icol = new IntColumn(ivec);
            return icol;
        }

        /* Deserialize a FloatColumn from the bytestream */
        FloatColumn* deserialize_float_column() {
            assert(step() == ','); 
            assert(step() == ' '); 
            assert(step() == 'd'); 
            assert(step() == 'a'); 
            assert(step() == 't'); 
            assert(step() == 'a'); 
            assert(step() == ':'); 
            assert(step() == ' ');
            FloatVector* fvec = dynamic_cast<FloatVector*>(deserialize());
            FloatColumn* fcol = new FloatColumn(fvec);
            return fcol;
        }

        /* Deserialize a StringColumn from the bytestream */
        StringColumn* deserialize_string_column() {
            assert(step() == ','); 
            assert(step() == ' '); 
            assert(step() == 'd'); 
            assert(step() == 'a'); 
            assert(step() == 't'); 
            assert(step() == 'a'); 
            assert(step() == ':'); 
            assert(step() == ' ');
            Vector* svec = dynamic_cast<Vector*>(deserialize());
            StringColumn* scol = new StringColumn(svec);
            return scol;
        }

        /* Deserialize a DataFrame from the bytestream. */
        DataFrame* deserialize_dataframe() {
            assert(step() == ',');
            assert(step() == ' ');
            assert(step() == 'c');
            assert(step() == 'o');
            assert(step() == 'l');
            assert(step() == 'u');
            assert(step() == 'm');
            assert(step() == 'n');
            assert(step() == 's');
            assert(step() == ':');
            assert(step() == ' ');
            assert(step() == '[');

            DataFrame* rtrn_df = new DataFrame();
            while (current() != ']') {
                Column* col = dynamic_cast<Column*>(deserialize());
                rtrn_df->add_column(col);
                assert(step() == ']');
                assert(step() == '}');
                assert(step() == '}');
                if (current() == ',') step(); // Skip the ','
            }
            return rtrn_df;
        }
};