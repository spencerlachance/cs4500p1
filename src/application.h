//lang::CwC

#include "kvstore.h"

/**
 * Abstract class outlining an application that uses the eau2 system.
 * 
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Application : public Object {
    public:
    // The index of the current node running the application
    size_t idx_;
    // The current node's Key/Value store
    KVStore kv_;

    Application(size_t idx) {
        idx_ = idx;
        run_();
    }

    /**
     * Runs the application on the current node.
     */
    virtual void run_();

    /**
     * Getter for the current node index.
     */
    size_t this_node() {
        return idx_;
    }
};