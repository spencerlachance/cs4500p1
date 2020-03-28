//lang::CwC

#include "../src/application.h"
#include "../src/dataframe.h"

/**
 * Simple eau2 application that builds a DataFrame containing floats 0-999,999 keeps track of the
 * floats' sum, puts the DataFrame into a KVStore, gets it back from the store, and verifies that
 * the sum of the DataFrame's values remains consistent.
 */
class Trivial : public Application {
    public:
    Trivial(size_t idx) : Application(idx) {
        run_();
    }
    void run_() {
        size_t SZ = 1000*1000;
        float* vals = new float[SZ];
        double sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        Key key("triv",0);
        DataFrame* df = DataFrame::fromFloatArray(&key, &kv_, SZ, vals);
        assert(df->get_float(0, 1) == 1);
        DataFrame* df2 = kv_.get(key);
        for (size_t i = 0; i < SZ; ++i) sum -= df2->get_float(0,i);
        assert(sum == 0);
        printf("KVStore Trivial test passed\n");
        delete df; delete df2; delete[] vals;
    }
};

class Demo : public Application {
public:
    Key main;
    Key verify;
    Key check;

    Demo(size_t idx) : Application(idx), main("main", 0), verify("verif", 0), check("ck", 0) {
        run_();
    }

    void run_() {
        switch(this_node()) {
            case 0:   producer();   break;
            case 1:   counter();    break;
            case 2:   summarizer();
        }
    }

    void producer() {
        size_t SZ = 100*1000;
        float* vals = new float[SZ];
        int sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        DataFrame::fromFloatArray(&main, &kv_, SZ, vals);
        DataFrame::fromIntScalar(&check, &kv_, sum);
    }

    void counter() {
        DataFrame* v = kv_.wait_and_get(main);
        int sum = 0;
        for (size_t i = 0; i < 100*1000; ++i) sum += v->get_float(0,i);
        p("The sum is  ").pln(sum);
        DataFrame::fromIntScalar(&verify, &kv_, sum);
    }

    void summarizer() {
        DataFrame* result = kv_.wait_and_get(verify);
        DataFrame* expected = kv_.wait_and_get(check);
        pln(expected->get_float(0,0)==result->get_float(0,0) ? "SUCCESS":"FAILURE");
    }
};

void construct_demo(size_t idx) {
    Demo d(idx);
}

int main() {
    Trivial t(0);
    return 0;
}