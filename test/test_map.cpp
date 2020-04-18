#include "../src/map.h"
#include <assert.h>

int main() {
    Map* map = new Map(1); // A map with a capacity of one.
    String* k = new String("key"); // String key in our key-value pair.
    Object* val1 = new String("value"); // Object value in our key-value pair.
    Object* val2 = new String("value");
    Object* val3 = new String("value");
    Object* val4 = new String("value");

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
    map->erase(*val1); // Deletes both val1 and get_val1

    assert(map->get(*val1) == nullptr); // returns nullptr when map does not have the requested key s1.
    printf("%zu\n", map->size());
    // assert(map->size() == 0);

    // Putting a second key-value pair in the map, which would grow the map's capacity.
    map->put(*k, val2); 
    map->put(*k, val3);
    map->put(*k, val4);

    delete map; // This also deletes the values (val2, val3 and val4).
    delete k; // Must manually delete key.
    printf("Map tests passed.\n");
    return 0;
}