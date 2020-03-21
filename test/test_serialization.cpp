//lang::CwC

#include "../src/vector.h"
#include "../src/serial.h"
#include "../src/message.h"

/** Testing FloatVector serialization and deserialization. */
void test_float_vector_serialization() {
    FloatVector* f_arr = new FloatVector();
    f_arr->append(1.1f);
    f_arr->append(1.11113f); // 5 decimals
    f_arr->append(1.111115f); // 6 decimals
    f_arr->append(1.1111116f); // 7 decimals
    f_arr->append(1.11111156f); // 8 decimals

    // FloatVector serialization
    const char* serialized_f_arr = f_arr->serialize();
    Serial* float_serial = new Serial(serialized_f_arr);
    assert(strcmp(serialized_f_arr, "{type: float_vector, floats: [1.1000000,1.1111300,1.1111150,1.1111116,1.1111115]}") == 0);
    // printf(serial_f_arr)

    //FloatVector deserialization
    FloatVector* deserialized_f_arr = dynamic_cast<FloatVector*>(float_serial->deserialize());
    assert(deserialized_f_arr != nullptr);
    assert(deserialized_f_arr->equals(f_arr));

    delete f_arr;
    delete float_serial;
    delete[] serialized_f_arr;
    delete deserialized_f_arr;
}

// /** Testing BoolVector serialization and deserialization. */
// void test_bool_vector_serialization() {
//     Serial serial;
//     BoolVector* bvec = new BoolVector();
//     bvec->append(1);
//     bvec->append(0); 
//     bvec->append(1); 
//     bvec->append(0); 
//     bvec->append(1);

//     // BoolVector serialization
//     const char* serial_bvec = bvec->serialize();
//     assert(strcmp(serial_bvec, "{type: bool_vector, bools: [true,false,true,false,true]}") == 0);
//     //printf("%s\n", serial_bvec);

//     //BoolVector deserialization
//     BoolVector* deserialized_bvec = dynamic_cast<BoolVector*>(serial.deserialize(serial_bvec));
//     assert(deserialized_bvec != nullptr);
//     assert(deserialized_bvec->equals(bvec));

//     delete bvec;
//     delete[] serial_bvec;
//     delete deserialized_bvec;
// }

// /** Testing IntVector serialization and deserialization. */
// void test_int_vector_serialization() {
//     Serial serial;
//     IntVector* ivec = new IntVector();
//     ivec->append(1);
//     ivec->append(2); 
//     ivec->append(3); 
//     ivec->append(4); 
//     ivec->append(5); 

//     // IntVector serialization
//     const char* serial_ivec = ivec->serialize();
//     assert(strcmp(serial_ivec, "{type: int_vector, ints: [1,2,3,4,5]}") == 0);
//     // printf(serial_ivec)

//     //IntVector deserialization
//     IntVector* deserialized_ivec = dynamic_cast<IntVector*>(serial.deserialize(serial_ivec));
//     assert(deserialized_ivec != nullptr);
//     assert(deserialized_ivec->equals(ivec));

//     delete ivec;
//     delete[] serial_ivec;
//     delete deserialized_ivec;
// }

/* Testings Vector serialization and deserialization */
void test_string_vector_serialization() {
    
    String* s1 = new String("abc");
    String* s2 = new String("abcd");
    String* s3 = new String("abcdefghi");
    String* s4 = new String("abcdefghij");

    // String serialization and deserialization 
    const char* serialized_str = s1->serialize();
    Serial* string_serial = new Serial(serialized_str);
    String* deserialized_str = dynamic_cast<String*>(string_serial->deserialize());
    assert(deserialized_str != nullptr);
    assert(s1->equals(deserialized_str));

    Vector* arr = new Vector();
    arr->append(s1);
    arr->append(s2);
    arr->append(s3);
    arr->append(s4);

    /* Vector serialization */
    const char* serial_vec = arr->serialize();
    assert(strcmp(serial_vec, "{type: vector, objects: [{type: string, cstr: abc},{type: string, cstr: abcd},{type: string, cstr: abcdefghi},{type: string, cstr: abcdefghij}]}") == 0);
    //printf("%s\n", serial_vec);
    

    /* Vector desserialization */
    Serial* string_vector_serial = new Serial(serial_vec);
    Vector* deserialized_arr = dynamic_cast<Vector*>(string_vector_serial->deserialize());
    assert(deserialized_arr != nullptr);
    assert(deserialized_arr->equals(arr));

    delete[] serialized_str;
    delete string_serial;
    delete deserialized_str;
    delete arr;
    delete[] serial_vec;
    delete string_vector_serial;
    delete deserialized_arr;
}

// void test_message_serialization() {
//     Serial serial;

//     /* Acknowledge construction */
//     Acknowledge* ack1 = new Acknowledge(1, 2, 3);
//     Acknowledge* ack2 = new Acknowledge(1, 2, 4);

//     /* Acknowledge serialization */
//     const char* serial_ack = ack1->serialize();
//     // assert(strcmp(serial_ack, "{type: ack, sender: 1, target: 2, id: 3}") == 0);
//     // printf("%s\n", serial_ack);

//     /* Ackknowledge deserialization */
//     Acknowledge* deserialized_ack = dynamic_cast<Acknowledge*>(serial.deserialize(serial_ack));
//     assert(deserialized_ack != nullptr);
//     assert(deserialized_ack->equals(ack1)); // Testing acknowledge equality.

//     delete ack1;
//     delete ack2;
//     delete[] serial_ack;
//     delete deserialized_ack;

//     /* Status construction */
//     String* msg = new String("ALERT! PANTS ON FIRE!");
//     Status* status = new Status(1, 2, 3, msg);

//     /* Status serialization */
//     const char* serial_status = status->serialize();
//     assert(strcmp(serial_status, "{type: status, sender: 1, target: 2, id: 3, msg: ALERT! PANTS ON FIRE!}") == 0);
//     // printf("%s\n", serial_status);

//     /* Status deserialization */
//     Status* deserialized_status = dynamic_cast<Status*>(serial.deserialize(serial_status));
//     assert(deserialized_status != nullptr);
//     assert(deserialized_status->equals(status)); // Testing status equality.

//     delete msg;
//     delete status;
//     delete[] serial_status;
//     delete deserialized_status;

//     /* Directory construction */
//     Directory* dir = new Directory(1, 2, 3, 3);
//     dir->add_adr("0.0.0.0");
//     dir->add_adr("0.1.0.0");
//     dir->add_adr("0.2.0.0");
//     dir->add_adr("0.3.0.0");
//     dir->add_port(8080);
//     dir->add_port(8084);
//     dir->add_port(8081);

//     /* Directory serialization */
//     const char* serial_directory = dir->serialize();
//     assert(strcmp(serial_directory,"{type: directory, sender: 1, target: 2, id: 3, nodes: 3, ports: [8080,8084,8081], addresses: [0.0.0.0,0.1.0.0,0.2.0.0,0.3.0.0]}") == 0);
//     // printf("%s\n", serial_directory);

//     /* Directory deserialization */
//     Directory* deserialized_directory = dynamic_cast<Directory*>(serial.deserialize(serial_directory));
//     assert(deserialized_directory != nullptr);
//     assert(deserialized_directory->equals(dir)); // Testing directory equality

//     delete dir;
//     delete serial_directory;
//     delete deserialized_directory;

//     /* Register construction */
//     struct sockaddr_in node;
//     node.sin_family = AF_INET;
//     node.sin_port = 8080;
//     inet_pton(AF_INET, "127.0.0.3", &(node.sin_addr));
//     Register* reg = new Register(1, 2, 3, node, 8080);

//     /* Register serialization */
//     const char* serial_register = reg->serialize();
//     // printf("%s\n", serial_register);
//     assert(strcmp(serial_register, "{type: register, sender: 1, target: 2, id: 3, node: [2,8080,127.0.0.3], port: 8080}") == 0);

//     /* Register deserialization */
//     Register* deserialized_register = dynamic_cast<Register*>(serial.deserialize(serial_register));
//     assert(deserialized_register != nullptr);
//     assert(deserialized_register->equals(reg));

//     delete reg;
//     delete serial_register;
//     delete deserialized_register;
// }

// void test_object_serialization() {
//     Serial s;
//     Object* o = new Object();
//     const char* serial_object = o->serialize();
//     assert(strcmp(serial_object, "{type: object}") == 0);

//     Object* deserialized_object = s.deserialize(serial_object);
// }

// void test_dataframe_serialization() {
//     IntColumn* icol = new IntColumn(4,1,2,3,4);
//     BoolColumn* bcol = new BoolColumn(4,true,false,true,false);
//     FloatColumn* fcol = new FloatColumn(4,1.1f,2.2f,3.3f,4.4f);
//     String* s1 = new String("hi");
//     String* s2 = new String("bye");
//     String* s3 = new String("hi");
//     String* s4 = new String("bye");
//     StringColumn* scol = new StringColumn(4,s1,s2,s3,s4);

//     const char* serial_icol = icol->serialize();
//     assert(strcmp(serial_icol, 
//         "{type: int_column, data: {type: int_vector, ints: [1,2,3,4]}}") == 0);

//     const char* serial_bcol = bcol->serialize();
//     assert(strcmp(serial_bcol, 
//         "{type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}}") == 0);

//     const char* serial_fcol = fcol->serialize();
//     assert(strcmp(serial_fcol, 
//         "{type: float_column, data: {type: float_vector, floats: [1.1000000,2.2000000,3.3000000,4.4000001]}}") == 0);

//     const char* serial_scol = scol->serialize();
//     assert(strcmp(serial_scol, 
//         "{type: string_column, data: {type: vector, objects: [{type: string, cstr: hi},{type: string, cstr: bye},{type: string, cstr: hi},{type: string, cstr: bye}]}}") == 0);

//     DataFrame* df = new DataFrame();
//     df->add_column(icol);
//     // df->add_column(bcol);
//     // df->add_column(fcol);
//     // df->add_column(scol);

    
//     const char* serial_df = df->serialize();
//     printf("%s\n", serial_df);
//     Serial s(serial_df);
//     assert(strcmp(serial_df, 
//         "{type: dataframe, columns: [{type: int_column, data: {type: int_vector, ints: [1,2,3,4]}}]}") == 0);
//         // ,{type: bool_column, data: {type: bool_vector, bools: [true,false,true,false]}},{type: float_column, data: {type: float_vector, floats: [1.1000000,2.2000000,3.3000000,4.4000001]}},{type: string_column, data: {type: vector, objects: [{type: string, cstr: hi},{type: string, cstr: bye},{type: string, cstr: hi},{type: string, cstr: bye}]}}]}") == 0);
//     DataFrame* deserialized_df = dynamic_cast<DataFrame*>(s.deserialize(serial_df));
//     assert(deserialized_df != nullptr);
//     assert(deserialized_df->equals(df));
// }

int main() {
    test_float_vector_serialization();
    // test_int_vector_serialization();
    // test_bool_vector_serialization();
    test_string_vector_serialization();
    //test_message_serialization();
    //test_object_serialization();
    //test_dataframe_serialization();
    printf("All serialization tests passed!\n");
    return 0;
}