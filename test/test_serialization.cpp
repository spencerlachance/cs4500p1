// //lang::CwC

#include <assert.h>
#include "../src/serial.h"
#include "../src/dataframe.h"

/** Testing BoolVector serialization and deserialization. */
void test_bool_vector_serialization() {
    BoolVector* bvec = new BoolVector();
    bvec->append(1);
    bvec->append(0); 
    bvec->append(1); 
    bvec->append(0); 
    bvec->append(1);

    // BoolVector serialization
    const char* serialized_bvec = bvec->serialize();
    assert(strcmp(serialized_bvec, "{type: bool_vector, bools: [true,false,true,false,true]}") == 0);
    

    //BoolVector deserialization
    Deserializer* bvec_ds = new Deserializer(serialized_bvec);
    BoolVector* deserialized_bvec = dynamic_cast<BoolVector*>(bvec_ds->deserialize());
    assert(deserialized_bvec != nullptr);
    assert(deserialized_bvec->equals(bvec));

    delete bvec;
    delete[] serialized_bvec;
    delete bvec_ds;
    delete deserialized_bvec;
}

/** Testing IntVector serialization and deserialization. */
void test_int_vector_serialization() {
    IntVector* ivec = new IntVector();
    ivec->append(1);
    ivec->append(2); 
    ivec->append(3); 
    ivec->append(4); 
    ivec->append(5); 

    // IntVector serialization
    const char* serialized_ivec = ivec->serialize();
    assert(strcmp(serialized_ivec, "{type: int_vector, ints: [1,2,3,4,5]}") == 0);
    

    // IntVector deserialization
    Deserializer* ivec_ds = new Deserializer(serialized_ivec);
    IntVector* deserialized_ivec = dynamic_cast<IntVector*>(ivec_ds->deserialize());
    assert(deserialized_ivec != nullptr);
    assert(deserialized_ivec->equals(ivec));

    delete ivec;
    delete[] serialized_ivec;
    delete ivec_ds;
    delete deserialized_ivec;
}

/** Testing FloatVector serialization and deserialization. */
void test_float_vector_serialization() {
    // Deserializer serial;
    FloatVector* fvec = new FloatVector();
    fvec->append(1.1f);
    fvec->append(1.11113f); // 5 decimals
    fvec->append(1.111115f); // 6 decimals
    fvec->append(1.1111116f); // 7 decimals
    fvec->append(1.11111156f); // 8 decimals

    // FloatVector serialization
    const char* serialized_fvec = fvec->serialize();
    assert(strcmp(serialized_fvec, "{type: float_vector, floats: [1.1000000,1.1111300,1.1111150,1.1111116,1.1111115]}") == 0);

    //FloatVector deserialization
    Deserializer* fvec_ds = new Deserializer(serialized_fvec);
    FloatVector* deserialized_fvec = dynamic_cast<FloatVector*>(fvec_ds->deserialize());
    assert(deserialized_fvec != nullptr);
    assert(deserialized_fvec->equals(fvec));

    delete fvec;
    delete[] serialized_fvec;
    delete deserialized_fvec;
    delete fvec_ds;
}

/* Testings Vector serialization and deserialization */
void test_string_vector_serialization() {
    String* s1 = new String("abc");
    String* s2 = new String("abcd");
    String* s3 = new String("abcdefghi");
    String* s4 = new String("abcdefghij");

    // String serialization and deserialization 
    const char* serialized_str = s1->serialize();
    Deserializer* string_ds = new Deserializer(serialized_str);
    String* deserialized_str = dynamic_cast<String*>(string_ds->deserialize());
    assert(deserialized_str != nullptr);
    assert(s1->equals(deserialized_str));

    Vector* vec = new Vector();
    vec->append(s1);
    vec->append(s2);
    vec->append(s3);
    vec->append(s4);

    /* Vector serialization */
    const char* serialized_vec = vec->serialize();
    assert(strcmp(serialized_vec, "{type: vector, objects: [{type: string, cstr: abc},{type: string, cstr: abcd},{type: string, cstr: abcdefghi},{type: string, cstr: abcdefghij}]}") == 0);
    
    /* Vector desserialization */
    Deserializer* svec_ds = new Deserializer(serialized_vec);
    Vector* deserialized_vec = dynamic_cast<Vector*>(svec_ds->deserialize());
    assert(deserialized_vec != nullptr);
    assert(deserialized_vec->equals(vec));

    
    delete[] serialized_str;
    delete[] serialized_vec;
    delete vec;
    delete string_ds;
    delete svec_ds;
    delete deserialized_str;
    delete deserialized_vec;
}

void test_message_serialization() {
    /* Ack construction */
    Ack* ack = new Ack();

    /* Ack serialization */
    const char* serialized_ack = ack->serialize();
    assert(strcmp(serialized_ack, "{type: ack}") == 0);

    /* Ack deserialization */
    Deserializer* ack_ds = new Deserializer(serialized_ack);
    Ack* deserialized_ack = dynamic_cast<Ack*>(ack_ds->deserialize());
    assert(deserialized_ack != nullptr);
    assert(deserialized_ack->equals(ack)); // Testing acknowledge equality.

    delete ack;
    delete deserialized_ack;
    delete ack_ds;

    /* Directory construction */
    Directory* dir = new Directory();
    dir->add_client("0.0.0.0", 0);
    dir->add_client("0.1.0.0", 1);
    dir->add_client("0.2.0.0", 2);
    dir->add_client("0.3.0.0", 3);

    /* Directory serialization */
    const char* serialized_directory = dir->serialize();
    assert(strcmp(serialized_directory,
        "{type: directory, addresses: {type: vector, objects: [{type: string, cstr: 0.0.0.0},{type: string, cstr: 0.1.0.0},{type: string, cstr: 0.2.0.0},{type: string, cstr: 0.3.0.0}]}, indices: {type: int_vector, ints: [0,1,2,3]}}") == 0);

    /* Directory deserialization */
    Deserializer* directory_ds = new Deserializer(serialized_directory);
    Directory* deserialized_directory = dynamic_cast<Directory*>(directory_ds->deserialize());
    assert(deserialized_directory != nullptr);
    assert(deserialized_directory->equals(dir)); // Testing directory equality

    delete dir;
    delete[] serialized_directory;
    delete deserialized_directory;
    delete directory_ds;

    /* Register construction */
    String* ip = new String("127.0.0.3");
    Register* reg = new Register(ip, 1);

    /* Register serialization */
    const char* serialized_register = reg->serialize();
    assert(strcmp(serialized_register, "{type: register, ip: {type: string, cstr: 127.0.0.3}, sender: 1}") == 0);

    /* Register deserialization */
    Deserializer* register_ds = new Deserializer(serialized_register);
    Register* deserialized_register = dynamic_cast<Register*>(register_ds->deserialize());
    assert(deserialized_register != nullptr);
    assert(deserialized_register->equals(reg));

    delete reg;
    delete[] serialized_register;
    delete deserialized_register;
    delete register_ds;
}

void test_object_serialization() {
    Object* o = new Object();
    const char* serialized_object = o->serialize();
    assert(strcmp(serialized_object, "{type: object}") == 0);

    Deserializer* object_ds = new Deserializer(serialized_object);
    Object* deserialized_object = object_ds->deserialize();
    delete o;
    delete deserialized_object;
    delete object_ds;
}

void test_dataframe_serialization() {
    /* Testing BoolColumn serialization and deserialization. */
    BoolColumn* bcol = new BoolColumn(4,true,false,true,false);
    const char* serialized_bcol = bcol->serialize();
    assert(strcmp(serialized_bcol, 
        "{type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}}") == 0);
    Deserializer* bcol_ds = new Deserializer(serialized_bcol);
    BoolColumn* deserialized_bcol = dynamic_cast<BoolColumn*>(bcol_ds->deserialize());
    assert(deserialized_bcol != nullptr);
    assert(deserialized_bcol->equals(bcol));

    /* Testing IntColumn serialization and deserialization. */
    IntColumn* icol = new IntColumn(4,1,2,3,4);
    const char* serialized_icol = icol->serialize();
    assert(strcmp(serialized_icol, 
        "{type: int_column, data: {type: int_vector, ints: [1,2,3,4]}}") == 0);
    Deserializer* icol_ds = new Deserializer(serialized_icol);
    IntColumn* deserialized_icol = dynamic_cast<IntColumn*>(icol_ds->deserialize());
    assert(deserialized_icol != nullptr);
    assert(deserialized_icol->equals(icol));

    /* Testing FloatColumn serialization and deserialization. */
    FloatColumn* fcol = new FloatColumn(4,1.1f,2.2f,3.3f,4.4f);
    const char* serialized_fcol = fcol->serialize();
    assert(strcmp(serialized_fcol, 
        "{type: float_column, data: {type: float_vector, floats: [1.1000000,2.2000000,3.3000000,4.4000001]}}") == 0);
    Deserializer* fcol_ds = new Deserializer(serialized_fcol);
    FloatColumn* deserialized_fcol = dynamic_cast<FloatColumn*>(fcol_ds->deserialize());
    assert(deserialized_fcol != nullptr);
    assert(deserialized_fcol->equals(fcol));

    /* Testing StringColumn serialization and deserialization. */
    String* s1 = new String("hi");
    String* s2 = new String("bye");
    String* s3 = new String("hi");
    String* s4 = new String("bye");
    StringColumn* scol = new StringColumn(4,s1,s2,s3,s4);
    const char* serialized_scol = scol->serialize();
    assert(strcmp(serialized_scol, 
        "{type: string_column, data: {type: vector, objects: [{type: string, cstr: hi},{type: string, cstr: bye},{type: string, cstr: hi},{type: string, cstr: bye}]}}") == 0);
    Deserializer* scol_ds = new Deserializer(serialized_scol);
    StringColumn* deserialized_scol = dynamic_cast<StringColumn*>(scol_ds->deserialize());
    assert(deserialized_scol != nullptr);
    assert(deserialized_scol->equals(scol));

    /* Testing DataFrame serialization and deserialization. */
    DataFrame* df = new DataFrame();
    df->add_column(icol);
    df->add_column(bcol);
    df->add_column(fcol);
    df->add_column(scol);
    const char* serialized_df = df->serialize();
    assert(strcmp(serialized_df, 
        "{type: dataframe, columns: [{type: int_column, data: {type: int_vector, ints: [1,2,3,4]}},{type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}},{type: float_column, data: {type: float_vector, floats: [1.1000000,2.2000000,3.3000000,4.4000001]}},{type: string_column, data: {type: vector, objects: [{type: string, cstr: hi},{type: string, cstr: bye},{type: string, cstr: hi},{type: string, cstr: bye}]}}]}") == 0);
    Deserializer* df_ds = new Deserializer(serialized_df);
    DataFrame* deserialized_df = dynamic_cast<DataFrame*>(df_ds->deserialize());
    assert(deserialized_df != nullptr);
    assert(deserialized_df->equals(df));

    delete df;
    delete deserialized_df;
    delete df_ds;
    delete icol_ds;
    delete bcol_ds;
    delete fcol_ds;
    delete scol_ds;
    delete deserialized_icol;
    delete deserialized_bcol;
    delete deserialized_fcol;
    delete deserialized_scol;
    delete[] serialized_icol;
    delete[] serialized_bcol;
    delete[] serialized_fcol;
    delete[] serialized_scol;
    delete[] serialized_df;
}

int main() {
    test_object_serialization();
    test_bool_vector_serialization();
    test_int_vector_serialization();
    test_float_vector_serialization();
    test_string_vector_serialization();
    test_dataframe_serialization();
    test_message_serialization();
    printf("All serialization tests passed!\n");
    return 0;
}