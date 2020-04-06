//lang::CwC

#pragma once

#include "serial.h"

/**
 * A union describing a field that could exist in a row of a dataframe.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
union Type {
    int i;
    bool b;
    float f;
    String* s;
};

/**
 * An Object wrapper class for the Type union. It is only allowed to one value of one of the four
 * types.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class DataType : public Object {
public:
    union Type t;
    char type_;

    /**
     * Constructor
     */
    DataType() : type_('U') { }

    /**
     * Destructor
     */
    ~DataType() {
        if (type_ == 'S') delete t.s; 
    }

    /**
     * These setters set the value of this object to the given one. Once a value is set, the
     * object's type is locked down and cannot change.
     */
    void set_int(int val) {
        exit_if_not(type_ == 'U', "This object's value has already been set to a different type.");
        t.i = val;
        type_ = 'I';
    }
    void set_bool(bool val) {
        exit_if_not(type_ == 'U', "This object's value has already been set to a different type.");
        t.b = val;
        type_ = 'B';
    }
    void set_float(float val) {
        exit_if_not(type_ == 'U', "This object's value has already been set to a different type.");
        t.f = val;
        type_ = 'F';
    }
    // Takes ownership of the string.
    void set_string(String* val) {
        exit_if_not(type_ == 'U', "This object's value has already been set to a different type.");
        t.s = val;
        type_ = 'S';
    }

    /**
     * These getters return this object's value.
     */
    int get_int() {
        if (type_ == 'U') set_int(0); // Return default value if missing
        exit_if_not(type_ == 'I', "This object's type does not match the type requested.");
        return t.i;
    }
    bool get_bool() {
        if (type_ == 'U') set_bool(0); // Return default value if missing
        exit_if_not(type_ == 'B', "This object's type does not match the type requested.");
        return t.b;
    }
    float get_float() {
        if (type_ == 'U') set_float(0); // Return default value if missing
        exit_if_not(type_ == 'F', "This object's type does not match the type requested.");
        return t.f;
    }
    String* get_string() {
        if (type_ == 'U') set_string(new String("")); // Return default value if missing
        exit_if_not(type_ == 'S', "This object's type does not match the type requested.");
        return t.s;
    }

    /** Returns a copy of this DataType. */
    DataType* clone() {
        DataType* res = new DataType();
        switch (type_) {
            case 'I':
                res->set_int(t.i); break;
            case 'B':
                res->set_bool(t.b); break;
            case 'F':
                res->set_float(t.f); break;
            case 'S':
                res->set_string(t.s->clone()); break;
        }
        return res;
    }

    /** Returns a char* representation of this DataType. */
    const char* serialize() {
        StrBuff buff;
        // Serialize the type char
        Serializer ser;
        char* c_type = new char[2];
        c_type[0] = type_;
        c_type[1] = '\0';
        buff.c(c_type);
        delete[] c_type;
        // Serialize the value
        const char* serial_val;
        switch (type_) {
            case 'I': {
                serial_val = ser.serialize_int(t.i);
                buff.c(serial_val);
                delete[] serial_val;
                break;
            }
            case 'B': {
                serial_val = ser.serialize_bool(t.b);
                buff.c(serial_val);
                delete[] serial_val;
                break;
            }
            case 'F': {
                serial_val = ser.serialize_float(t.f);
                buff.c(serial_val);
                delete[] serial_val;
                break;
            }
            case 'S': {
                serial_val = t.s->serialize();
                buff.c(serial_val);
                delete[] serial_val;
                break;
            }
        }
        return buff.c_str();
    }
};