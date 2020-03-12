//lang::CwC

#include "../src/dataframe.h"
#include "../src/parser_main.h"
#include "../src/helper.h"

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
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
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
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
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

/**
 * A simple test that builds a dataframe with one hundred ints and strings, and then tests
 * map() and filter()
 */
void test1() {
    Schema s("IS");
    DataFrame* df = new DataFrame(s);
    Row* r = new Row(s);
    String* str = new String("foo");
    for (int i = 1; i <= 100; i++) {
        r->set(0, i);
        r->set(1, str);
        df->add_row(*r);
    }
    SumRower* sr = new SumRower();
    df->map(*sr);
    assert(sr->get_total() == 5050);
    printf("Map test passed\n");

    AboveRower* ar = new AboveRower(50);
    DataFrame* filtered_df = df->filter(*ar);
    assert(filtered_df->nrows() == 50);
    printf("Filter test passed\n");
    printf("Test1 passed\n");

    delete df;
    delete r;
    delete str;
    delete sr;
    delete ar;
    delete filtered_df;
}

/**
 * This test builds a DataFrame from data in a file and then calculates the sum of all of its ints.
 * This test is meant to measure performance, so it does not verify that the sum is correct.
 */
void test2(int argc, char** argv) {
    // Upon construction, this class reads the command line arguments and builds a dataframe
    // containing fields from a datafile.
    ParserMain* pf = new ParserMain(argc, argv);

    DataFrame* df = new DataFrame(*(pf->get_dataframe()));
    SumRower* sr = new SumRower();
    df->map(*sr);
    delete pf;
    delete df;
    delete sr;
    printf("Test2 passed\n");
}

int main(int argc, char** argv) {
    test1();
    test2(argc, argv);
    return 0;
}