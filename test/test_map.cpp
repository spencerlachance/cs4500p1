#include "../src/map.h"
#include <assert.h>

int main() {
    Map* map = new Map(1); // A map with an initial capacity of one.
    String* k = new String("key"); // String key in our key-value pair.
    String* k2 = new String("key2");
    String* k3 = new String("key3");
    String* k4 = new String("key4");
    String* val1 = new String("value"); // Object value in our key-value pair.
    String* val2 = new String("value");
    String* val3 = new String("value");
    String* val4 = new String("value");

    // Testing contains(), get() and erase() before puting any key-values in the map.
    assert(!map->contains(*k)); // returns false because there is no key k.
    assert(map->get(*k) == nullptr); // returns nullptr when map does not have the requested key k.

    // Putting a key and value in the map.
    map->put(*k, val1); // k is cloned, but val1 is now owned by the map.

    // Testing containsKey() and get() after putting a key value pair (k, val1)
    assert(map->contains(*k)); // returns true because there is now a key k.
    assert(map->size() == 1);
    String* get_val1 = dynamic_cast<String*>(map->get(*k)); // get_val1 is still owned by map.
    assert(get_val1 != nullptr);
    assert(get_val1->equals(val1));

    // Testing remove() after putting a key value pair (k, val1)
    assert(map->size() == 1);

    // Erasing val1 from the map
    map->erase(*k); // Deletes both val1 and get_val1

    assert(map->get(*k) == nullptr); // returns nullptr when map does not have the requested key s1.
    assert(map->size() == 0);

    // Putting a second key-value pair in the map, which would grow the map's capacity.
    map->put(*k2, val2); 
    map->put(*k3, val3);
    map->put(*k4, val4);
    assert(map->size() == 3);

    delete map;
    delete k; delete k2; delete k3; delete k4;
    printf("Map tests passed.\n");
    return 0;
}