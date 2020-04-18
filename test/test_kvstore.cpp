// Lang::CwC

#include "../src/kdstore.h"

int main() {

    /* Arrays to be stored in KDStore */
    float* floats_ = new float[1000];
    bool* bools_ = new bool[1000];
    int* ints_ = new int[1000];
    Vector* v_ = new Vector();

    /* Populating the arrays */
    for (int i = 0; i < 1000; i++) {
        String* s = new String("foo");
        floats_[i] = i + 0.1f;
        bools_[i] = i % 2;
        ints_[i] = i;
        v_->append(s);
    }

    /* Scalars to be stored in KDStore */
    float f = 1.1f;
    int i = 1;
    bool b = false;
    String s("scalar");

    /* Testing KDStore with 1 node. */
    KDStore* kd_ = new KDStore(0, 1);

    // /* Testing storing scalars and arrays in KDStore */
    Key key1("float",0);
    Key key2("int",0);
    Key key3("bool",0);
    Key key4("string",0);
    DataFrame* df_f = DataFrame::fromFloatScalar(&key1, kd_, f);
    DataFrame* df_i = DataFrame::fromIntScalar(&key2, kd_, i);
    DataFrame* df_b = DataFrame::fromBoolScalar(&key3, kd_, b);
    DataFrame* df_s = DataFrame::fromStringScalar(&key4, kd_, s.clone());

    assert(df_f->ncols() == 1);
    float f_ = df_f->get_float(0, 0);
    assert(f_ == f);
    assert(df_i->ncols() == 1);
    int i_ = df_i->get_int(0, 0);
    assert(i_ == i);
    assert(df_b->ncols() == 1);
    bool b_ = df_b->get_bool(0, 0);
    assert(b_ == b);
    assert(df_s->ncols() == 1);
    String* s_ = df_s->get_string(0, 0);
    assert(s_->equals(&s));

    kd_->done();

    delete kd_; delete df_f; delete df_i; delete df_b; delete df_s; delete s_; delete[] floats_;
    delete[] ints_; delete[] bools_; delete v_;
    printf("kvstore test was SUCCESSFUL\n");
    return 0;
}