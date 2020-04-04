//lang::CwC

#pragma once

#include "object.h"
#include "string.h"

class Row;

/*****************************************************************************
 * Fielder::
 * A field vistor invoked by Row.
 */
class Fielder : public Object {
public:
 
  /** Called before visiting a row, the argument is the row offset in the
    dataframe. */
  virtual void start(size_t r) = 0;
 
  /** Called for fields of the argument's type with the value of the field. */
  virtual void accept(bool b) = 0;
  virtual void accept(float f) = 0;
  virtual void accept(int i) = 0;
  virtual void accept(String* s) = 0;
 
  /** Called when all fields have been seen. */
  virtual void done() = 0;
};
 
/*******************************************************************************
 *  Rower::
 *  An interface for iterating through each row of a data frame. The intent
 *  is that this class should subclassed and the accept() method be given
 *  a meaningful implementation. Rowers can be cloned for parallel execution.
 */
class Rower : public Object {
 public:
  /** This method is called once per row. The row object is on loan and
      should not be retained as it is likely going to be reused in the next
      call. The return value is used in filters to indicate that a row
      should be kept. */
  virtual bool accept(Row& r) = 0;
 
  /** Once traversal of the data frame is complete the rowers that were
      split off will be joined.  There will be one join per split. The
      original object will be the last to be called join on. The join method
      is reponsible for cleaning up memory. */
  virtual void join_delete(Rower* other) = 0;
};


/*.....................................................................................................*/
/*...........................IMPLEMENTATION OF ROWERS AND FILTERS......................................*/

/**
 * Fielder that prints each field.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 */
class PrintFielder : public Fielder {
    public:
        void start(size_t r) { }
        void accept(bool b) { printf("<%d>", b); }
        void accept(float f) { printf("<%f>", f); }
        void accept(int i) { printf("<%d>", i); }
        void accept(String* s) { printf("<%s>", s->c_str()); }
        void done() { }
};

/**
 * Rower that prints each row.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 */
class PrintRower : public Rower {
    public:
        PrintFielder* pf_;

        PrintRower() {
            pf_ = new PrintFielder();
        }

        ~PrintRower() {
            delete pf_;
        }

        bool accept(Row& r) { 
            r.visit(r.get_idx(), *pf_);
            printf("\n");
        }

        void join_delete(Rower* other) { }
};

/**
 * A Fielder that adds up every int it finds in a row.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 */
class SumFielder : public Fielder {
    public:
        long total_;

        void start(size_t r) { total_ = 0; }
        void done() { }

        SumFielder(long total) {
            total_ = total;
        }
        ~SumFielder() { }

        void accept(bool b) { }
        void accept(float f) { }
        void accept(String* s) { }
        void accept(int i) {
            total_ += i;
        }

        long get_total() {
            return total_;
        }
};

/**
 * A Rower that adds up every int it finds in a row using a Fielder.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 */
class SumRower : public Rower {
    public:
        SumFielder* sf_;
        long total_;

        SumRower() {
            total_ = 0;
            sf_ = new SumFielder(total_);
        }
        ~SumRower() { delete sf_; }

        bool accept(Row& r) {
            r.visit(r.get_idx(), *sf_);
            total_ += sf_->get_total();
        }

        long get_total() {
            return total_;
        }

        void join_delete(Rower* other) {
            SumRower* o = dynamic_cast<SumRower*>(other);
            total_ += o->get_total();
            delete o;
        }

        Object* clone() {
            return new SumRower();
        }
};

/**
 * A Fielder that accepts every int in a row that is above a given threshhold.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 */
class AboveFielder : public Fielder {
    public:
        bool passes_;
        int thresh_;

        void start(size_t r) {passes_ = true;}
        void done() { }

        AboveFielder(int thresh) {
            thresh_ = thresh;
        }
        ~AboveFielder() { }

        void accept(bool b) { }
        void accept(float f) { }
        void accept(String* s) { }
        void accept(int i) {
            if (i <= thresh_) passes_ = false;
        }

        bool check_pass() {
            return passes_;
        }
};

/**
 * A Rower that accepts every row whose ints are above a given threshhold.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 */
class AboveRower : public Rower {
    public:
        AboveFielder* af_;

        AboveRower(int thresh) {
            af_ = new AboveFielder(thresh);
        }

        ~AboveRower() { delete af_; }

        bool accept(Row& r) {
            r.visit(r.get_idx(), *af_);
            return af_->check_pass();
        }

        void join_delete(Rower* other) { }
};