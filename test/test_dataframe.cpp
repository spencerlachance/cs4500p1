//lang::CwC

#include "../src/dataframe.h"
#include "../src/parser_main.h"
#include "../src/helper.h"

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
    // test_dataframe_copy_constructor(df);
    // test_add_column(df);
    // test_get(df);
    // test_set(df);
    // test_fill_row(df);
    // test_add_row(df);
    // test_pad_columns(df);
    // test_nrows(df);
    // test_ncols(df);

    delete df;
    delete r;
    delete str;
    return 0;
}