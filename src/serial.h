//lang:CwC

#pragma once

#include <assert.h>

#include "message.h"

/**
 * Helper class that handles deserializing objects of various types.
 * 
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Serial : public Object {
    public:
        
        /* Builds and returns an object from the given bytestream. */
        static Object* deserialize(const char* stream) {
            // stream's initial 6 chars should be "{type: "
            assert(stream[0] == '{');
            assert(stream[1] == 't');
            assert(stream[2] == 'y');
            assert(stream[3] == 'p');
            assert(stream[4] == 'e');
            assert(stream[5] == ':');
            assert(stream[6] == ' ');

            // Fill a buffer with the type of object to create
            // either "object", "string", "array" or "FloatVector".
            StrBuff* buff = new StrBuff();
            int i = 7;
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            for (i; stream[i] != ','; i++) {
                *x = stream[i];
                buff->c(x);
            }

            char* type = buff->get()->c_str();
            buff = new StrBuff(); // Clear buffer for future use.

            if (strcmp(type, "object") == 0) {
                return new Object();
            } else if (strcmp(type, "ack") == 0) {
                return deserialize_acknowledge(i, buff, stream);
            } else if (strcmp(type, "status") == 0) {
                return deserialize_status(i, buff, stream);
            } else if (strcmp(type, "directory") == 0) {
                return desrialize_directory(i, buff, stream);
            } else if (strcmp(type, "register") == 0) {
                return deserialize_register(i, buff, stream);
            } else if (strcmp(type, "string") == 0) {
                return deserialize_string(i, buff, stream);
            } else if (strcmp(type, "array") == 0) {
                return deserialize_string_array(i, buff, stream);
            } else if (strcmp(type, "float_array") == 0) {
                return deserialize_float_array(i, buff, stream);
            }

        }

        /** Returns an IntVector containing the fields of a Message from the given bytestream. */
        static IntVector* deserialize_message(int i, StrBuff* buff, const char* stream) {
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            // Selecting the sender fields from the bytestream.
            i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == 'n'); i++;
            assert(stream[i] == 'd'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == 'r'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            for (i; stream[i] != ','; i++) {
                *x = stream[i];
                buff->c(x);
            }
            i++;
            // deserializing sender to a size_t.
            size_t sender = atoi(buff->get()->c_str()); 
            buff = new StrBuff();
            // Selecting the target field from the bytestream.
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 't'); i++;
            assert(stream[i] == 'a'); i++;
            assert(stream[i] == 'r'); i++;
            assert(stream[i] == 'g'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == 't'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            for (i; stream[i] != ','; i++) {
                *x = stream[i];
                buff->c(x);
            }
            i++;
            // deserializing target field value to a size_t.
            size_t target = atoi(buff->get()->c_str()); 
            buff = new StrBuff();
            // Selecting the id fields from the bytestream.
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'i'); i++;
            assert(stream[i] == 'd'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            for (i; stream[i] != ',' && stream[i] != '}'; i++) {
                *x = stream[i];
                buff->c(x);
            }
            i++;
            // deserializing it to a size_t.
            int id = atoi(buff->get()->c_str());
            // Reassign buff to a new string buffer for further use.
            buff = new StrBuff();
            // delete x to save memory.
            delete[] x;
            // Appending sender, target and id to the return array
            IntVector* return_arr = new IntVector();
            return_arr->append(sender);
            return_arr->append(target);
            return_arr->append(id);
            // Last element is i for further reading of the bytestream.
            return_arr->append(i);

            return return_arr;
        }

        /* Builds and returns a Register from the given bytesetram. */
        static Register* deserialize_register(int i, StrBuff* buff, const char* stream) {
            IntVector* msg_fields = deserialize_message(i, buff, stream);
            buff = new StrBuff();
            i = msg_fields->get(msg_fields->size() - 1);
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            // Selecting the sin_family, sin_port and sin_addr fields from the bytestream.
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'n'); i++;
            assert(stream[i] == 'o'); i++;
            assert(stream[i] == 'd'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == '['); i++;
            struct sockaddr_in sa; // this sockaddr_in struct will be the node field in the register.
            int count = 0;
            for (; stream[i - 1] != ']'; i++) {
                for (; stream[i] != ',' && stream[i] != ']'; i++) {
                    *x = stream[i];
                    buff->c(x);
                }
                count++;
                if (count == 1) {
                    sa.sin_family = atoi(buff->get()->c_str()); // Setting the sin_family field.
                    buff = new StrBuff();
                } else if (count == 2) {
                    sa.sin_port = atoi(buff->get()->c_str()); // Setting the sin_port field.
                    buff = new StrBuff();
                } else if (count == 3) {
                    inet_pton(AF_INET, buff->get()->c_str(), &(sa.sin_addr)); // Setting the sin_addr field.
                    buff = new StrBuff();
                }
            }
            // Selecting the port field from the bytestream and deserializing it to size_t.
            assert(stream[i] == ','); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'p'); i++;
            assert(stream[i] == 'o'); i++;
            assert(stream[i] == 'r'); i++;
            assert(stream[i] == 't'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            for (; stream[i] != '}'; i++) {
                *x = stream[i];
                buff->c(x);
            }
            size_t port = atoi(buff->get()->c_str()); 

            return new Register(msg_fields->get(0), msg_fields->get(1), msg_fields->get(2), sa, port);
        }

        /** Builds and returns an Acknowledge from the given bytestream. */
        static Acknowledge* deserialize_acknowledge(int i, StrBuff* buff, const char* stream) {
            IntVector* msg_fields = deserialize_message(i, buff, stream);
            return new Acknowledge(msg_fields->get(0), msg_fields->get(1), msg_fields->get(2));
        }

        /* Builds and returns a Status from the given bytestream. */
        static Status* deserialize_status(int i, StrBuff* buff, const char* stream) {
            IntVector* msg_fields = deserialize_message(i, buff, stream);
            buff = new StrBuff();
            // We use x as a char* to pass to the string buffer one by one from the stream[i] 
            // and avoid a missing '\0'.
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            // Reasigning i to continue reading from stream.
            i = msg_fields->get(msg_fields->size() - 1);
            // Selecting the message field from the bytestream.
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'm'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == 'g'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;

            for (i; stream[i] != '}'; i++) {
                *x = stream[i];
                buff->c(x);
            }
            return new Status(msg_fields->get(0), msg_fields->get(1), msg_fields->get(2), buff->get());
        }

        /* Builds and returns a Directory from the given bytestream. */
        static Directory* desrialize_directory(int i, StrBuff* buff, const char* stream) {
            IntVector* msg_fields = deserialize_message(i, buff, stream);
            buff = new StrBuff();
            // We use x as a char* to pass to the string buffer one by one from the stream[i] 
            // and avoid a missing '\0'.
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            // Reassigning i to continue reading from stream.
            i = msg_fields->get(msg_fields->size() - 1);  
            // Selecting the nodes field from the bytestream and deserializing it to size_t.
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'n'); i++;
            assert(stream[i] == 'o'); i++;
            assert(stream[i] == 'd'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            for (i; stream[i] != ','; i++) {
                *x = stream[i];
                buff->c(x);
            }
            size_t nodes = atoi(buff->get()->c_str());
            buff = new StrBuff();
            // Selecting the ports fields from the bytestream and deserializing them into an IntVector.
            i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'p'); i++;
            assert(stream[i] == 'o'); i++;
            assert(stream[i] == 'r'); i++;
            assert(stream[i] == 't'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == '['); i++;
            IntVector* ports = new IntVector();
            while (true) {
                while (true) {
                    *x = stream[i];
                    buff->c(x);
                    i++;
                    if (stream[i] == ']' || stream[i] == ',') { break; }
                }
                size_t port = atoi(buff->get()->c_str());
                ports->append(port);
                buff = new StrBuff();
                if (stream[i] == ']') { break; }
                i++;
            }
            // Selecting the addresses fields from the bytestream and deserializing them into size_t.
            i++; 
            assert(stream[i] == ','); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'a'); i++;
            assert(stream[i] == 'd'); i++;
            assert(stream[i] == 'd'); i++;
            assert(stream[i] == 'r'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == '['); i++;
            Vector* addresses = new Vector();
            while (true) {
                while (true) {
                    *x = stream[i];
                    buff->c(x);
                    i++;
                    if (stream[i] == ']' || stream[i] == ',') { break; }
                }
                String* address = buff->get();
                addresses->append(address);
                buff = new StrBuff();
                if (stream[i] == ']') { break; }
                i++;
            }

            return new Directory(msg_fields->get(0), msg_fields->get(1), msg_fields->get(2), nodes, ports, addresses);

        }

        /* Builds and returns a String from the given bytestream. */
        static String* deserialize_string(int i, StrBuff* buff, const char* stream) {
            buff = new StrBuff();
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            // Sellecing the c_str
            assert(stream[i] == ','); i++; 
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'c'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == 't'); i++;
            assert(stream[i] == 'r'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++; 
            // Iterating over the string characters.
            for (i; stream[i] != '}'; i++) {
                *x = stream[i];
                buff->c(x);
            }

            delete[] x;
            return new String(*(buff->get()));
        }

        /* Builds and returns an Array representation of the given bytestream. 
        *  Even if this is an Array containing object elements, only strings can be deserialized,
        *  so we assume the return Array is an array of strings. 
        *  Create an Array, append strings from the stream to it and return it.
        */
        static Vector* deserialize_string_array(int i, StrBuff* buff, const char* stream) {
            buff = new StrBuff();
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            i++; 
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'o'); i++;
            assert(stream[i] == 'b'); i++;
            assert(stream[i] == 'j'); i++;
            assert(stream[i] == 'e'); i++;
            assert(stream[i] == 'c'); i++;
            assert(stream[i] == 't'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == '['); i++;

            // We use this array class for all types of objects but, for the scope 
            // of this prototype, only string objects are deserializable, so we have 
            // an assert to check if it is possible to cast object elements to string.
            Vector* arr = new Vector();

            while (true) {
                assert(stream[i] == '{');
                while (stream[i - 1] != '}') {
                    *x = stream[i];
                    buff->c(x);
                    i++;
                }
                String* element = dynamic_cast<String*>(deserialize(buff->get()->c_str()));
                assert(element != nullptr);
                buff = new StrBuff();
                arr->append(element);
                if (stream[i] == ']') { break; } // Exit if you see end of float array.
                i++;
            }

            delete[] x;
            return arr;
        }

        /* Builds and returns a FloatVector from the given bytestream
        *  Create a new FlaotVector(), fill it with floats from the stream and return it.
        * */
        static FloatVector* deserialize_float_array(int i, StrBuff* buff, const char* stream) {
            buff = new StrBuff();
            char* x = new char[sizeof(char) + 1];
            x[1] = '\0';
            // stream's initial 11 chars should be ", floats: ["
            assert(stream[i] == ','); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == 'f'); i++;
            assert(stream[i] == 'l'); i++;
            assert(stream[i] == 'o'); i++;
            assert(stream[i] == 'a'); i++;
            assert(stream[i] == 't'); i++;
            assert(stream[i] == 's'); i++;
            assert(stream[i] == ':'); i++;
            assert(stream[i] == ' '); i++;
            assert(stream[i] == '['); i++; // Now iterating over array elements.

            // New FloatVector we will return once we have filled it with corresponding floats
            FloatVector* f_arr = new FloatVector();
            
            while (true) {
                assert(stream[i] == '{'); i++;
                while (true) {
                    *x = stream[i];
                    buff->c(x);
                    i++;
                    if (stream[i] == '}') { break; }
                }
                char* chr = buff->get()->c_str();
                buff = new StrBuff();
                float f = atof(chr); // Deserialized chr to a float.
                f_arr->append(f);
                i++;
                if (stream[i] == ']') { break; } // Break loop if stream[i] is ']'
                assert(stream[i] == ',');
                i++;
            }

            delete[] x;
            return f_arr;
        }
};