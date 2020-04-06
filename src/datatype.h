//lang::CwC

#pragma once

#include "string.h"

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
    char type;

    /**
     * Constructor
     */
    DataType() {
        // Initialize the object with an unknown type
        type = 'U';
    }

    /**
     * Returns the type of this DataType.
     * */
    char get_type() { 
        return type; 
    }

    /**
     * These setters set the value of this object to the given one. Once a value is set, the
     * object's type is locked down and cannot change.
     */
    void set_int(int val) {
        exit_if_not(type == 'U', "This object's value has already been set to a different type.");
        t.i = val;
        type = 'I';
    }
    void set_bool(bool val) {
        exit_if_not(type == 'U', "This object's value has already been set to a different type.");
        t.b = val;
        type = 'B';
    }
    void set_float(float val) {
        exit_if_not(type == 'U', "This object's value has already been set to a different type.");
        t.f = val;
        type = 'F';
    }
    // Does not take ownership of the String because this is a placeholder object
    void set_string(String* val) {
        exit_if_not(type == 'U', "This object's value has already been set to a different type.");
        t.s = val;
        type = 'S';
    }

    /**
     * These getters return this object's value.
     */
    int get_int() {
        exit_if_not(type == 'I', "This object's type does not match the type requested.");
        return t.i;
    }
    bool get_bool() {
        exit_if_not(type == 'B', "This object's type does not match the type requested.");
        return t.b;
    }
    float get_float() {
        exit_if_not(type == 'F', "This object's type does not match the type requested.");
        return t.f;
    }
    String* get_string() {
        exit_if_not(type == 'S', "This object's type does not match the type requested.");
        return t.s;
    }
    bool equals(Object* o) {
        DataType* other = dynamic_cast<DataType*>(o);
        if (other == nullptr) return false;
        if (get_type() != other->get_type()) return false;
        switch (type) {
            case 'I':
                return other->get_int() == get_int();
            case 'B':
                return other->get_bool() == get_bool();
            case 'F':
                return other->get_float() == get_float();
            case 'S':
                return other->get_string()->equals(get_string());
        }
    }
};
