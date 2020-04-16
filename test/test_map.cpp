#include "../src/map.h"
#include <assert.h>

int main() {
    Map* map = new Map();
    String* s1 = new String("key"); // String key in our key-value pair.
    String* s2 = new String("value"); // Object value in our key-value pair.

    // Testing contains(), get() and erase() before puting any key-values in the map.
    assert(!map->contains(*s1)); // returns false because there is no key s1.
    assert(map->get(*s1) == nullptr); // returns nullptr when map does not have the requested key s1.
    map->put(*s1, s2);

    // Testing containsKey() and get() after putting a key value pair (s1, s2)
    assert(map->contains(*s1)); // returns true because there is now a key s1.
    String* get_s2 = dynamic_cast<String*>(map->get(*s1));
    assert(get_s2 != nullptr);
    assert(get_s2->equals(s2));

    // Testing remove() after putting a key value pair (s1, s2)
    assert(map->size() == 1);
    map->erase(*s1);
    assert(map->get(*s1) == nullptr); // returns nullptr when map does not have the requested key s1.
    assert(map->size() == 0);

    delete map; // This also deletes the value (s2 and get_s2).
    delete s1; // Must manually delete key.
    printf("Map tests passed.\n");
    return 0;
}