//lang::CwC

#pragma once

#include "object.h"
#include "vector.h"
#include "schema.h"
#include "visitors.h"

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
};

/*************************************************************************
 * Row::
 *
 * This class represents a single row of data constructed according to a
 * dataframe's schema. The purpose of this class is to make it easier to add
 * read/write complete rows. Internally a dataframe hold data in columns.
 * Rows have pointer equality.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Row : public Object {
    public:
        IntVector* col_types_;
        Vector* fields_;
        size_t idx_;
 
        /** Build a row following a schema. */
        Row(Schema& scm) {
            col_types_ = new IntVector();
            col_types_->append_all(scm.get_types());
            fields_ = new Vector();
            idx_ = -1;
        }

        /** Destructor */
        ~Row() {
            delete col_types_;
            delete fields_;
        }
        
        /** Setters: set the given column with the given value. Setting a column with
            * a value of the wrong type is undefined. */
        void set(size_t col, int val) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'I', "Column index corresponds to the wrong type.");
            DataType* dt = new DataType();
            dt->set_int(val);
            fields_->set(dt, col);
        }
        void set(size_t col, float val) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'F', "Column index corresponds to the wrong type.");
            DataType* dt = new DataType();
            dt->set_float(val);
            fields_->set(dt, col);
        }
        void set(size_t col, bool val) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'B', "Column index corresponds to the wrong type.");
            DataType* dt = new DataType();
            dt->set_bool(val);
            fields_->set(dt, col);
        }
        /** Acquire ownership of the string. */
        void set(size_t col, String* val) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'S', "Column index corresponds to the wrong type.");
            DataType* dt = new DataType();
            dt->set_string(val);
            fields_->set(dt, col);
        }
        
        /** Set/get the index of this row (ie. its position in the dataframe. This is
         *  only used for informational purposes, unused otherwise */
        void set_idx(size_t idx) {
            idx_ = idx;
        }
        size_t get_idx() {
            return idx_;
        }
        
        /** Getters: get the value at the given column. If the column is not
            * of the requested type, the result is undefined. */
        int get_int(size_t col) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'I', "Column index corresponds to the wrong type.");
            DataType* type = dynamic_cast<DataType*>(fields_->get(col));
            return type->get_int();
        }
        bool get_bool(size_t col) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'B', "Column index corresponds to the wrong type.");
            DataType* type = dynamic_cast<DataType*>(fields_->get(col));
            return type->get_bool();
        }
        float get_float(size_t col) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'F', "Column index corresponds to the wrong type.");
            DataType* type = dynamic_cast<DataType*>(fields_->get(col));
            return type->get_float();
        }
        String* get_string(size_t col) {
            exit_if_not(col < width(), "Column index out of bounds.");
            exit_if_not(col_types_->get(col) == 'S', "Column index corresponds to the wrong type.");
            DataType* type = dynamic_cast<DataType*>(fields_->get(col));
            return type->get_string();
        }
        
        /** Number of fields in the row. */
        size_t width() {
            return col_types_->size();
        }
        
        /** Type of the field at the given position. An idx >= width is  undefined. */
        char col_type(size_t idx) {
            exit_if_not(idx < width(), "Column type index out of bounds.");
            return col_types_->get(idx);
        }
        
        /** Given a Fielder, visit every field of this row. The first argument is
            * index of the row in the dataframe.
            * Calling this method before the row's fields have been set is undefined. */
        void visit(size_t idx, Fielder& f) {
            f.start(idx);
            for (int i = 0; i < col_types_->size(); i++) {
                char type = col_types_->get(i);
                switch (type) {
                    case 'I':
                        f.accept(get_int(i));
                        break;
                    case 'B':
                        f.accept(get_bool(i));
                        break;
                    case 'F':
                        f.accept(get_float(i));
                        break;
                    case 'S':
                        f.accept(get_string(i));
                        break;
                    default:
                        exit_if_not(false, "Invalid type found.");
                }
            }
            f.done();
        }

        /** Getter for this row's schema's types. */
        IntVector* get_types() {
            return col_types_;
        }
};