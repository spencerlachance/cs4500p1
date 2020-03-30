#include "../src/application.h"
#include "../src/dataframe.h"

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
        size_t SZ = 1000;
        float* vals = new float[SZ];
        float sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        delete DataFrame::fromFloatArray(&main, &kv_, SZ, vals);
        delete DataFrame::fromFloatScalar(&check, &kv_, sum);
        delete[] vals;
    }

    void counter() {
        DataFrame* v = kv_.wait_and_get(main);
        float sum = 0;
        for (size_t i = 0; i < 1000; ++i) sum += v->get_float(0,i);
        p("The sum is ").pln(sum);
        delete DataFrame::fromFloatScalar(&verify, &kv_, sum);
        delete v;
    }

    void summarizer() {
        DataFrame* result = kv_.wait_and_get(verify);
        DataFrame* expected = kv_.wait_and_get(check);
        pln(expected->get_float(0,0)==result->get_float(0,0) ? "SUCCESS":"FAILURE");
        done();
        delete result; delete expected;
    }
};

int main(int argc, char** argv) {
    Sys s;
    size_t idx = atoi(argv[2]);
    s.exit_if_not(idx <= 2, "Invalid index from command line");
    Demo d(idx);
}