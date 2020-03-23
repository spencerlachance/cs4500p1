// //lang::CwC

#include "../src/vector.h"
#include "../src/serial.h"
#include "../src/dataframe.h"
#include <assert.h>
#include "../src/string.h"

void test_object_serialization() {
    /* Object serialization */
    Object* o = new Object();
    const char* serialized_object = o->serialize();
    assert(strcmp(serialized_object, "{type: object}") == 0);

    /* Object deserialization */
    Serial* stream = new Serial(serialized_object);
    Object* deserialized_object = stream->deserialize();
    assert(deserialized_object != nullptr);

    delete o;
    // delete serialized_object;
    delete stream;
    // delete deserialized_object;
}

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
    // printf("%s\n", serialized_bvec);
    assert(strcmp(serialized_bvec, "{type: bool_vector, bools: [true,false,true,false,true]}") == 0);
    

    //BoolVector deserialization
    Serial* stream = new Serial(serialized_bvec);
    BoolVector* deserialized_bvec = dynamic_cast<BoolVector*>(stream->deserialize());
    assert(deserialized_bvec != nullptr);
    assert(deserialized_bvec->equals(bvec));

    delete bvec;
    delete[] serialized_bvec;
    delete stream;
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
    // printf("%s\n", serialized_ivec);
    assert(strcmp(serialized_ivec, "{type: int_vector, ints: [1,2,3,4,5]}") == 0);
    

    // IntVector deserialization
    Serial* stream = new Serial(serialized_ivec);
    IntVector* deserialized_ivec = dynamic_cast<IntVector*>(stream->deserialize());
    assert(deserialized_ivec != nullptr);
    assert(deserialized_ivec->equals(ivec));

    delete ivec;
    delete[] serialized_ivec;
    delete stream;
    delete deserialized_ivec;
}

/** Testing FloatVector serialization and deserialization. */
void test_float_vector_serialization() {
    // Serial serial;
    FloatVector* fvec = new FloatVector();
    fvec->append(1.1f);
    fvec->append(1.11113f); // 5 decimals
    fvec->append(1.111115f); // 6 decimals
    fvec->append(1.1111116f); // 7 decimals
    fvec->append(1.11111156f); // 8 decimals

    // FloatVector serialization
    const char* serialized_fvec = fvec->serialize();
    // printf("%s\n", serial_fvec);
    assert(strcmp(serialized_fvec, "{type: float_vector, floats: [1.1000000,1.1111300,1.1111150,1.1111116,1.1111115]}") == 0);

    //FloatVector deserialization
    Serial* stream = new Serial(serialized_fvec);
    FloatVector* deserialized_fvec = dynamic_cast<FloatVector*>(stream->deserialize());
    assert(deserialized_fvec != nullptr);
    assert(deserialized_fvec->equals(fvec));

    delete fvec;
    delete[] serialized_fvec;
    delete deserialized_fvec;

}

/* Testings Vector serialization and deserialization */
void test_string_vector_serialization() {
    String* s1 = new String("abc");
    String* s2 = new String("abcd");
    String* s3 = new String("abcdefghi");
    String* s4 = new String("abcdefghij");

    // String serialization and deserialization 
    const char* serialized_str = s1->serialize();
    Serial* stream = new Serial(serialized_str);
    String* deserialized_str = dynamic_cast<String*>(stream->deserialize());
    assert(deserialized_str != nullptr);
    assert(s1->equals(deserialized_str));

    Vector* vec = new Vector();
    vec->append(s1);
    vec->append(s2);
    vec->append(s3);
    vec->append(s4);

    /* Vector serialization */
    const char* serialized_vec = vec->serialize();
    // printf("%s\n", serial_vec);
    assert(strcmp(serialized_vec, "{type: vector, objects: [{type: string, cstr: abc},{type: string, cstr: abcd},{type: string, cstr: abcdefghi},{type: string, cstr: abcdefghij}]}") == 0);
    
    /* Vector desserialization */
    Serial* stream2 = new Serial(serialized_vec);
    Vector* deserialized_vec = dynamic_cast<Vector*>(stream2->deserialize());
    assert(deserialized_vec != nullptr);
    assert(deserialized_vec->equals(vec));

    
    delete[] serialized_str;
    delete stream;
    delete deserialized_str;

    delete vec;
    delete[] serialized_vec;
    delete stream2;
    // delete deserialized_vec;
}

void test_dataframe_serialization() {
    BoolColumn* bcol = new BoolColumn(4,true,false,true,false);
    // IntColumn* icol = new IntColumn(4,1,2,3,4);
    // FloatColumn* fcol = new FloatColumn(4,1.1f,2.2f,3.3f,4.4f);
    // String* s1 = new String("hi");
    // String* s2 = new String("bye");
    // String* s3 = new String("hi");
    // String* s4 = new String("bye");
    // StringColumn* scol = new StringColumn(4,s1,s2,s3,s4);

    /* Testing BoolColumn serialization and deserialization. */
    const char* serialized_bcol = bcol->serialize();
    // printf("%s\n", serialized_bcol);
    assert(strcmp(serialized_bcol, 
        "{type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}}") == 0);
    Serial* bcol_stream = new Serial(serialized_bcol);
    BoolColumn* deserialized_bcol = dynamic_cast<BoolColumn*>(bcol_stream->deserialize());
    assert(deserialized_bcol != nullptr);
    assert(deserialized_bcol->equals(bcol));

    /* Testing IntColumn serialization and deserialization. */
    // const char* serialized_icol = icol->serialize();
    // printf("%s\n", serialized_icol);
    // assert(strcmp(serialized_icol, 
    //     "{type: int_column, data: {type: int_vector, ints: [1,2,3,4]}}") == 0);
    // Serial* icol_stream = new Serial(serialized_icol);
    // IntColumn* deserialized_icol = dynamic_cast<IntColumn*>(icol_stream->deserialize());
    // assert(deserialized_icol != nullptr);
    // assert(deserialized_icol->equals(icol));

    // /* Testing FloatColumn serialization and deserialization. */
    // const char* serial_fcol = fcol->serialize();
    // assert(strcmp(serial_fcol, 
    //     "{type: float_column, data: {type: float_vector, floats: [1.1000000,2.2000000,3.3000000,4.4000001]}}") == 0);
    // Serial* float_stream = new Serial(serial_fcol);
    // FloatColumn* deserialized_fcol = dynamic_cast<FloatColumn*>(float_stream->deserialize());

    // /* Testing StringColumn serialization and deserialization. */
    // const char* serial_scol = scol->serialize();
    // assert(strcmp(serial_scol, 
    //     "{type: string_column, data: {type: vector, objects: [{type: string, cstr: hi},{type: string, cstr: bye},{type: string, cstr: hi},{type: string, cstr: bye}]}}") == 0);
    // Serial* string_stream = new Serial(serial_scol);
    // StringColumn* deserialized_scol = dynamic_cast<StringColumn*>(string_stream->deserialize());

    // DataFrame* df = new DataFrame();
    // df->add_column(icol);
    // df->add_column(bcol);
    // df->add_column(fcol);
    // df->add_column(scol);

    
    // const char* serial_df = df->serialize();
    // printf("%s\n", serial_df);
    // Serial s(serial_df);
    // assert(strcmp(serial_df, 
    //     "{type: dataframe, columns: [{type: int_column, data: {type: int_vector, ints: [1,2,3,4]}}]}") == 0);
    //     // ,{type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}},{type: float_column, data: {type: float_vector, floats: [1.1000000,2.2000000,3.3000000,4.4000001]}},{type: string_column, data: {type: vector, objects: [{type: string, cstr: hi},{type: string, cstr: bye},{type: string, cstr: hi},{type: string, cstr: bye}]}}]}") == 0);
    // DataFrame* deserialized_df = dynamic_cast<DataFrame*>(s.deserialize(serial_df));
    // assert(deserialized_df != nullptr);
    // assert(deserialized_df->equals(df));

    delete bcol;
    delete[] serialized_bcol;
    delete bcol_stream;

    // delete icol;
    // delete[] serialized_icol;
    // delete icol_stream;

    // delete fcol;
}

int main() {
    test_object_serialization();
    test_bool_vector_serialization();
    test_int_vector_serialization();
    test_float_vector_serialization();
    test_string_vector_serialization();
    //test_message_serialization();
    
    // test_dataframe_serialization();
    printf("All serialization tests passed!\n");
    return 0;
}