// Lang::CwC

#include "../src/kdstore.h"

int main() {

    /* Arrays to be stored in KDStore. */
    float* floats = new float[1000];
    bool* bools = new bool[1000];
    int* ints = new int[1000];
    String* strings[1000];

    /* Populating the arrays. */
    for (int i = 0; i < 1000; i++) {
        floats[i] = i + 0.1f;
        bools[i] = i % 2;
        ints[i] = i;
        strings[i] = new String("foo");
    }

    /* Scalars to be stored in KDStore. */
    float f = 1.1f;
    bool b = false;
    int i = 1;
    String s("scalar");

    /* Testing KDStore with 1 node. */
    KDStore* kd_ = new KDStore(0, 1);

    // /* Testing storing scalars and arrays in a KDStore. */
    Key key1("float",0);
    Key key2("bool",1);
    Key key3("int",0);
    Key key4("string",0);
    Key key5("float array",0);
    Key key6("bool array",0);
    Key key7("int array",0);
    Key key8("string array",0);

    DataFrame* df_f = DataFrame::fromFloatScalar(&key1, kd_, f);
    DataFrame* df_b = DataFrame::fromBoolScalar(&key2, kd_, b);
    DataFrame* df_i = DataFrame::fromIntScalar(&key3, kd_, i);
    DataFrame* df_s = DataFrame::fromStringScalar(&key4, kd_,  s.clone()); // clone the string because kd_ takes ownership of value

    DataFrame* df_floats = DataFrame::fromFloatArray(&key5, kd_, 1000, floats);
    DataFrame* df_bools = DataFrame::fromBoolArray(&key6, kd_, 1000, bools);
    DataFrame* df_ints = DataFrame::fromIntArray(&key7, kd_, 1000, ints);
    // DataFrame* df_strings = DataFrame::fromStringArray(&key8, kd_, 1000, strings);

    /* Testing if the same values are contained in the dataframe
     * and if the dataframe has the right amount of columns and rows. */
    assert(df_f->ncols() == 1);
    assert(df_f->nrows() == 1);
    float f_ = df_f->get_float(0, 0);
    assert(f_ == f);

    assert(df_i->ncols() == 1);
    assert(df_i->nrows() == 1);
    int i_ = df_i->get_int(0, 0);
    assert(i_ == i);

    assert(df_b->ncols() == 1);
    assert(df_b->nrows() == 1);
    bool b_ = df_b->get_bool(0, 0);
    assert(b_ == b);

    assert(df_s->ncols() == 1);
    assert(df_s->nrows() == 1);
    String* s_ = df_s->get_string(0, 0);
    assert(s_->equals(&s));

    assert(df_floats->ncols() == 1);
    assert(df_floats->nrows() == 1000);
    for (int i = 0; i > 1000; i++) {
        assert(df_floats->get_float(0, i) == floats[i]);
    }

    assert(df_bools->ncols() == 1);
    assert(df_bools->nrows() == 1000);
    for (int i = 0; i > 1000; i++) {
        assert(df_bools->get_bool(0, i) == bools[i]);
    }

    assert(df_ints->ncols() == 1);
    assert(df_ints->nrows() == 1000);
    for (int i = 0; i > 1000; i++) {
        assert(df_ints->get_int(0, i) == ints[i]);
    }

    // assert(df_strings->ncols() == 1);
    // assert(df_strings->nrows() == 1000);
    // for (int i = 0; i > 1000; i++) {
    //     assert(df_strings->get_string(0, i) == strings[i]);
    // }

    /* Testing methods get() and wantAndGet() in kdstore. */
    assert(kd_->get(key1)->equals(df_f));
    assert(kd_->get(key2)->equals(df_b));
    assert(kd_->get(key3)->equals(df_i));
    assert(kd_->get(key4)->equals(df_s));
    assert(kd_->get(key5)->equals(df_floats));
    assert(kd_->get(key6)->equals(df_bools)); 
    assert(kd_->get(key7)->equals(df_ints));
    // assert(kd_->get(key8)->equals(df_strings)); 

    kd_->done();

    delete kd_; 
    delete df_f; delete df_i; delete df_b; delete df_s; delete df_floats; delete df_bools;
    delete df_ints; //delete df_strings;
    delete s_; 
    delete[] floats, delete[] ints, delete[] bools; 

    // delete s;
    for (int i = 0; i < 1000; i++) {
        delete strings[i];
    }
    printf("kvstore test was SUCCESSFUL\n");
    return 0;
}