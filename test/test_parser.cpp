//lang::CwC

#include "../src/dataframe.h"
#include "../src/parser_main.h"
#include "../src/helper.h"

/**
 * This test builds a DataFrame from data in a file and then calculates the sum of all of its ints.
 * This test is meant to measure performance, so it does not verify that the sum is correct.
 */
void test_datafile(int argc, char** argv) {
    // Upon construction, this class reads the command line arguments and builds a dataframe
    // containing fields from a datafile.
    ParserMain* pf = new ParserMain(argc, argv);

    DataFrame* df = new DataFrame(*(pf->get_dataframe()));
    SumRower* sr = new SumRower();
    df->map(*sr);
    delete pf;
    delete df;
    delete sr;
    printf("Datafile test passed\n");
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

    test_datafile(argc, argv);

    delete df;
    delete r;
    delete str;
    return 0;
}