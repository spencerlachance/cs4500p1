//lang:CwC

#pragma once

#include <assert.h>
#include "column.h"

/**
 * Helper class that handles deserializing objects of various types.
 * 
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Serial : public Object {
    public:
        const char* stream_;
        int i_; // current location in the stream
        char* x_;
        

        Serial(const char* stream) {
            stream_ = stream;
            i_ = 0;
            x_ = new char[sizeof(char) + 1];
            x_[1] = '\0';
        }

        ~Serial() {
            delete[] x_;
        }

        /* Returns the current location in the stream. */
        char current() {
            char rtrn = stream_[i_];
            return rtrn;
        }

        /* Return the current location in the stream and step forward by 1. */
        char step() {
            char rtrn = stream_[i_];
            i_++;
            return rtrn; 
        }

        /* Builds and returns an object from the given bytestream. */
        Object* deserialize() {
            StrBuff* buff = new StrBuff();
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
            
            while (current() != ',' && current() != '}') {
                *x_ = step();
                buff->c(x_);
            }
            char* type = buff->get()->c_str();

            if (strcmp(type, "object") == 0) {
                return new Object();
            } else if (strcmp(type, "string") == 0) {
                return deserialize_string(*buff);
            } else if (strcmp(type, "vector") == 0) {
                return deserialize_string_vector(*buff);
            } else if (strcmp(type, "float_vector") == 0) {
                return deserialize_float_vector(*buff);
            } else if (strcmp(type, "int_vector") == 0) {
                return deserialize_int_vector(*buff);
            } else if (strcmp(type, "bool_vector") == 0) {
                return deserialize_bool_vector(*buff);
            } else if (strcmp(type, "bool_column") == 0) {
                return deserialize_bool_column(*buff);
            } else if (strcmp(type, "int_column") == 0) {
                return deserialize_int_column(*buff);
            } //else if (strcmp(type, "float_column") == 0) {
            //     return deserialize_float_column(*buff);
            // } else if (strcmp(type, "string_column") == 0) {
            //     return deserialize_string_column(*buff);
            // } else if (strcmp(type, "dataframe") == 0) {
            //     return deserialize_dataframe();
            // }

            delete[] type;
            delete buff;
        }

        /* Builds and returns a String from the given bytestream. */
        String* deserialize_string(StrBuff buff) {
            buff.clear();
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

            return buff.get();
        }

        /* Builds and returns an vector representation of the given bytestream. 
        *  Even if this is an vector containing object elements, only strings can be deserialized,
        *  so we assume the return vector is an vector of strings. 
        *  Create an vector, append strings from the stream to it and return it.
        */
        Vector* deserialize_string_vector(StrBuff buff) {
            buff.clear();

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
                assert(step() == '{');
                assert(step() == 't');
                assert(step() == 'y');
                assert(step() == 'p');
                assert(step() == 'e');
                assert(step() == ':');
                assert(step() == ' ');
                assert(step() == 's');
                assert(step() == 't');
                assert(step() == 'r');
                assert(step() == 'i');
                assert(step() == 'n');
                assert(step() == 'g');
                String* element = deserialize_string(buff);
                buff.clear();

                vec->append(element);
                assert(step() == '}'); 
                if (current() == ',') { step(); } // Step over the ','
            }
            return vec;
        }

        /* Builds and returns a FloatVector from the bytestream
        *  Create a new FlaotVector(), fill it with floats from the stream and return it.
        * */
        FloatVector* deserialize_float_vector(StrBuff buff) {
            buff.clear();

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
                    buff.c(x_);
                }
                char* fchr = buff.get()->c_str();
                buff.clear();

                float f = atof(fchr); // Deserialized chr to a float.
                delete[] fchr;
                fvec->append(f);
                if (current() == ',') { step(); };
            }

            return fvec;
        }

        /* Builds and returns an IntVector from the bytestream */
        IntVector* deserialize_int_vector(StrBuff buff) {
            buff.clear();

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
                    buff.c(x_);
                }
                char* ichr = buff.get()->c_str();
                buff.clear();

                int val = atoi(ichr); // Deserialized chr to an int.
                delete[] ichr;
                ivec->append(val);
                if (current() == ',') { step(); };
            }

            return ivec;
        }

        /* Builds and returns an BoolVector from the bytestream */
        BoolVector* deserialize_bool_vector(StrBuff buff) {
            buff.clear();

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
                    buff.c(x_);
                }
                char* bchr = buff.get()->c_str();
                buff.clear();

                if (strcmp(bchr, "true") == 0) {
                    bvec->append(1);
                } else if (strcmp(bchr, "false") == 0) {
                    bvec->append(0);
                }
                delete[] bchr;
                if (current() == ',') { step(); };
            }
    
            return bvec;
        }

        /* Builds and returns a BoolColumn from the bytestream */
        // {type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}}
        BoolColumn* deserialize_bool_column(StrBuff buff) {
            buff.clear();

            assert(step() == ',');
            assert(step() == ' ');
            assert(step() == 'd');
            assert(step() == 'a');
            assert(step() == 't');
            assert(step() == 'a');
            assert(step() == ':');
            assert(step() == ' ');
            assert(step() == '{');
            assert(step() == 't');
            assert(step() == 'y');
            assert(step() == 'p');
            assert(step() == 'e');
            assert(step() == ':');
            assert(step() == ' ');
            assert(step() == 'b');
            assert(step() == 'o');
            assert(step() == 'o');
            assert(step() == 'l');
            assert(step() == '_');
            assert(step() == 'v');
            assert(step() == 'e');
            assert(step() == 'c');
            assert(step() == 't');
            assert(step() == 'o');
            assert(step() == 'r');
            
            BoolVector* bvec = dynamic_cast<BoolVector*>(deserialize_bool_vector(buff));
            BoolColumn* bcol = new BoolColumn(bvec);
            return bcol;
        }

        /* Deserialize IntColumn from the given bytestream */
        IntColumn* deserialize_int_column(StrBuff buff) {
            buff.clear();

            assert(step() == ','); 
            assert(step() == ' '); 
            assert(step() == 'd'); 
            assert(step() == 'a'); 
            assert(step() == 't'); 
            assert(step() == 'a'); 
            assert(step() == ':'); 
            assert(step() == ' '); 
            assert(step() == '{'); 
            assert(step() == 't'); 
            assert(step() == 'y'); 
            assert(step() == 'p'); 
            assert(step() == 'e'); 
            assert(step() == ':'); 
            assert(step() == ' ');

            IntVector* ivec = deserialize_int_vector(buff);
            IntColumn* icol = new IntColumn(ivec);
            return icol;
        }

//         // /* Returns an DataFrame containing the columns and fields from the given bytestream. */
//         // /* {
//         //     type: dataframe, columns: [
//         //                                {type: int_column, data: {type: int_vector, ints: [1,2,3,4]}},
//         //                                {type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}},
//         //                                {type: float_column, data: {type: float_vector, floats: [1.1000000,2.2000000,3.3000000,4.4000001]}},
//         //                                {type: string_column, data: {type: vector, objects: [{type: string, cstr: hi},{type: string, cstr: bye},{type: string, cstr: hi},{type: string, cstr: bye}]}}
//         //                               ]
//         //    } 
//         // */
//         // static DataFrame* deserialize_dataframe(int i, StrBuff* buff, const char* stream) {
//         //     DataFrame* rtrn_df = new DataFrame();
//         //     buff = new StrBuff();
//         //     char* x = new char[sizeof(char) + 1];
//         //     x[1] = '\0';
//         //     assert(stream[i] == ','); i++; 
//         //     assert(stream[i] == ' '); i++;
//         //     assert(stream[i] == 'c'); i++;
//         //     assert(stream[i] == 'o'); i++;
//         //     assert(stream[i] == 'l'); i++;
//         //     assert(stream[i] == 'u'); i++;
//         //     assert(stream[i] == 'm'); i++;
//         //     assert(stream[i] == 'n'); i++;
//         //     assert(stream[i] == 's'); i++;
//         //     assert(stream[i] == ':'); i++;
//         //     assert(stream[i] == ' '); i++;
//         //     assert(stream[i] == '['); i++;
//         //     assert(stream[i] == '{'); i++;
//         //     assert(stream[i] == 't'); i++;
//         //     assert(stream[i] == 'y'); i++;
//         //     assert(stream[i] == 'p'); i++;
//         //     assert(stream[i] == 'e'); i++;
//         //     assert(stream[i] == ':'); i++;
//         //     assert(stream[i] == ' '); i++;

//         //     for (i; stream[i] != ','; i++) {
//         //         *x = stream[i];
//         //         buff->c(x);
//         //     }
//         //     char* col_type = buff->get()->c_str();
//         //     buff = new StrBuff(); // clear buff

//         //     if (strcmp(col_type, "int_column") == 0) {
//         //         IntColumn icol = deserialize_int_column(i, buff, stream);
//         //         rtrn_df->add_column(icol);
//         //     }

//         //     delete[] x;
//         //     return rtrn_df;
//         // }


};