//lang::CwC

#include "vector.h"
#include "serial.h"
#include "message.h"

// /** Testing FloatVector serialization and deserialization. */
// void test_float_array_serialization() {
//     Serial serial;
//     FloatVector* f_arr = new FloatVector();
//     f_arr->append(1.1f);
//     f_arr->append(1.11113f); // 5 decimals
//     f_arr->append(1.111115f); // 6 decimals
//     f_arr->append(1.1111116f); // 7 decimals
//     f_arr->append(1.11111156f); // 8 decimals

//     // FloatVector serialization
//     const char* serial_f_arr = f_arr->serialize();
//     assert(strcmp(serial_f_arr, "{type: float_array, floats: [{1.1000000},{1.1111300},{1.1111150},{1.1111116},{1.1111115}]}") == 0);
//     // printf(serial_f_arr)

//     //FloatVector deserialization
//     FloatVector* deserialized_f_arr = dynamic_cast<FloatVector*>(serial.deserialize(serial_f_arr));
//     assert(deserialized_f_arr != nullptr);
//     assert(deserialized_f_arr->equals(f_arr));
// }

// /* Testings Vector serialization and deserialization */
// void test_string_array_serialization() {
//     Serial serial;
//     String* s1 = new String("abc");
//     String* s2 = new String("abcd");
//     String* s3 = new String("abcdefghi");
//     String* s4 = new String("abcdefghij");

//     // String serialization and deserialization 
//     const char* serial_str = s1->serialize();
//     String* deserialized_str = dynamic_cast<String*>(serial.deserialize(serial_str));
//     assert(deserialized_str != nullptr);
//     assert(s1->equals(deserialized_str));

//     Vector* arr = new Vector();
//     arr->append(s1);
//     arr->append(s2);
//     arr->append(s3);
//     arr->append(s4);

//     // Array serialization 
//     const char* serial_arr = arr->serialize();
//     assert(strcmp(serial_arr, "{type: array, objects: [{type: string, cstr: abc},{type: string, cstr: abcd},{type: string, cstr: abcdefghi},{type: string, cstr: abcdefghij}]}") == 0);
//     //printf("%s\n", serial_arr);
    

//     /* Array desserialization */
//     Vector* deserialized_arr = dynamic_cast<Vector*>(serial.deserialize(serial_arr));
//     assert(deserialized_arr != nullptr);
//     assert(deserialized_arr->equals(arr));

//     delete s1;
//     delete s2;
//     delete s3;
//     delete s4;

//     delete arr;
//     delete[] serial_arr;

//     delete serial_str;
//     delete deserialized_str;
// }

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
// }

// void test_object_serialization() {
//     Serial s;
//     Object* o = new Object();
//     const char* serial_object = o->serialize();
//     assert(strcmp(serial_object, "{type: object}") == 0);

//     Object* deserialized_object = s.deserialize(serial_object); // All objects are the same, no need to test reference equality
// }

void test_dataframe_serialization() {
    IntColumn* icol = new IntColumn(4,1,2,3,4);
    BoolColumn* bcol = new BoolColumn(4,1,0,1,0);
    FloatColumn* fcol = new FloatColumn(4,1.1f,2.2f,3.3f,4.4f);
    String* s1 = new String("hi");
    String* s2 = new String("bye");
    String* s3 = new String("hi");
    String* s4 = new String("bye");
    StringColumn* scol = new StringColumn(4,s1,s2,s3,s4);

    const char* serial_icol = icol->serialize();
    printf("%s\n", serial_icol);
    assert(strcmp(serial_icol, "{I: [1,2,3,4]}") == 0);

    const char* serial_bcol = bcol->serialize();
    printf("%s\n", serial_bcol);
    assert(strcmp(serial_bcol, "{B: [1,0,1,0]}") == 0);

    const char* serial_fcol = fcol->serialize();
    printf("%s\n", serial_fcol);
    assert(strcmp(serial_fcol, "{F: [1.1000000,2.2000000,3.3000000,4.4000001]}") == 0);

    const char* serial_scol = scol->serialize();
    printf("%s\n", serial_scol);
    assert(strcmp(serial_scol, "{S: [hi,bye,hi,bye]}") == 0);

    DataFrame* df = new DataFrame();
    df->add_column(icol);
    df->add_column(bcol);
    df->add_column(fcol);
    df->add_column(scol);

    const char* serial_df = df->serialize();
    printf("%s\n", serial_df);
    assert(strcmp(serial_df, "{type: dataframe, columns: [{I: [1,2,3,4]}, {B: [1,0,1,0]}, {F: [1.1000000,2.2000000,3.3000000,4.4000001]}, {S: [hi,bye,hi,bye]}]"));
}

int main() {
    // test_float_array_serialization();
    // test_string_array_serialization();
    // test_message_serialization();
    // test_object_serialization();
    test_dataframe_serialization();
    printf("All tests passed!\n");
    return 0;
}