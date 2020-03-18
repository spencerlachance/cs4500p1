//lang::CwC

#include "application.h"
#include "dataframe.h"

/**
 * Simple eau2 application that builds a DataFrame containing floats 0-999,999, keeps track of the
 * floats' sum, puts the DataFrame into a KVStore, gets it back from the store, and verifies that
 * the sum of the DataFrame's values remains consistent.
 */
class Trivial : public Application {
    public:
    Trivial(size_t idx) : Application(idx) { }
    void run_() {
        size_t SZ = 1000*1000;
        float* vals = new float[SZ];
        float sum = 0;
        for (size_t i = 0; i < SZ; ++i) sum += vals[i] = i;
        Key key("triv",0);
        DataFrame* df = DataFrame::fromFloatArray(&key, &kv_, SZ, vals);
        assert(df->get_float(0,1) == 1);
        DataFrame* df2 = kv_.get(key);
        for (size_t i = 0; i < SZ; ++i) sum -= df2->get_float(0,i);
        assert(sum==0);
        delete df; delete df2;
    }
};

int main() {
    Trivial t(0);
    printf("KVStore test passed\n");
    return 0;
}