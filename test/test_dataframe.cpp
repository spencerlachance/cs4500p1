//lang::CwC

#include <unistd.h>
#include "../src/dataframe.h"
#include "../src/parser_main.h"
#include "../src/helper.h"

/*.....................................................................................................*/
/*...........................IMPLEMENTATION OF ROWERS AND FIELDERS.....................................*/
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

/**
 * A simple test that tests map() using a Rower that calculates the sum of every int in the
 * dataframe.
 */
void test_map(DataFrame* df) {
    SumRower* sr = new SumRower();
    df->map(*sr);
    assert(sr->get_total() == 5050);
    printf("Map test passed\n");
    delete sr;
}

/**
 * A simple test that tests filter() using a Rower that accepts all rows with ints greater
 * than the given value.
 */
void test_filter(DataFrame* df) {
    AboveRower* ar = new AboveRower(50);
    DataFrame* filtered_df = df->filter(*ar);
    assert(filtered_df->nrows() == 50);
    printf("Filter test passed\n");
    delete ar;
    delete filtered_df;
}

/**
 * This test builds a DataFrame from data in a file and then calculates the sum of all of its ints.
 * This test is meant to measure performance and correct data parsing, 
 * so it does not verify that the sum is correct.
 */
void test_datafile(DataFrame* df, int argc, char** argv) {
    // Upon construction, this class reads the command line arguments and builds a dataframe
    // containing fields from a datafile.
    ParserMain* pf = new ParserMain(argc, argv);

    //TODO: set this method to read from small_datafile.txt, then assert if it's equal to given df.

    DataFrame* df_ = new DataFrame(*(pf->get_dataframe()));
    // the parsed datafile should have the same data as df.
    assert(df_->equals(df));

    SumRower* sr = new SumRower();
    df_->map(*sr);
    delete pf;
    delete df_;
    delete sr;
    printf("Datafile test passed\n");
}

void test_rows_cols(DataFrame* df) {
    // Build the row, using the df's schema.
    Row* r1 = new Row(df->get_schema());
    df->fill_row(99, *r1);
    Row* r2 = new Row(df->get_schema());
    r2->set(0, 100);
    String* str = new String("foo");
    r2->set(1, str);
    assert(r1->equals(r2)); //Equality test for object vectors is messed up

    // Build a bool column with 50 values alternating between true and false.
    BoolColumn* b_col = new BoolColumn();
    for (int i = 1; i <= 50; i++) {
        bool b = i%2;
        b_col->push_back(b);
    }

    // Build a float column with 100 values from 1.1 to 100.1, increasing by 1.
    FloatColumn* f_col = new FloatColumn();
    for (int i = 1; i <= 100; i++) {
        f_col->push_back(i+0.1f);
    }

    // Testing the dataframe copy constructor
    DataFrame* df_copy = new DataFrame(*df);
    assert(df_copy->equals(df));
    printf("copy constructor test passed\n");

    // Adding a float column to an empty DataFrame
    df_copy->add_column(f_col);
    assert(df_copy->ncols() == 3);
    assert(df_copy->nrows() == 100);

    // Adding a bool column with 50 values to a DataFrame with 100 values.
    assert(b_col->size() == 50);
    df_copy->add_column(b_col);
    assert(b_col->size() == 100);

    // Retrieving a bool column from df_copy
    BoolColumn* padded_b_col = dynamic_cast<BoolColumn*>(df_copy->get_columns_()->get(3));

    // Testing padding of columns added without the same length as the DataFrame.
    assert(padded_b_col != nullptr);
    assert(padded_b_col->size() == 100); // b_col was padded with missing values when it was added to df_copy
    for (int i = 50; i < 100; i++) {
        assert(padded_b_col->get(i) == false); // missing value is represented as false in BoolColumns.
    }

    Row* r3 = new Row(df_copy->get_schema());
    df_copy->fill_row(0, *r3);

    assert(df_copy->get_int(0, 0) == r3->get_int(0));
    assert(df_copy->get_string(1, 0)->equals(r3->get_string(1)));
    assert(df_copy->get_float(2, 0) == r3->get_float(2));
    assert(df_copy->get_bool(3, 0) == r3->get_bool(3));
    
    delete r1;
    delete r2;
    delete r3;
    delete str;
    delete df_copy; // deleting df_copy will delete f_col, b_col and padded_b_col, but won't affect the original df.
    printf("rows and columns test passed\n");
}

int main(int argc, char** argv) {
    Schema s("IS");
    DataFrame* df = new DataFrame(s);
    Row* r = new Row(s);
    String* str = new String("foo");
    
    for (int i = 1; i <= 100; i++) {
        r->set(0, i);
        r->set(1, str);
        df->add_row(*r);
    }

    

    test_map(df);
    test_filter(df);
    // test_datafile(df, argc, argv);
    test_rows_cols(df);
    // test_error_handling(df);

    delete df;
    delete r;
    delete str;
    return 0;
}
