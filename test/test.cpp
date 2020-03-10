//lang::CwC

#include "../src/dataframe.h"
#include "../src/parser_main.h"
#include "../src/helper.h"

/**
 * A Fielder that adds up every int it finds in a row.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
 */
class SumFielder : public Fielder {
    public:
        size_t total_;

        void start(size_t r) { total_ = 0; }
        void done() { }

        SumFielder(size_t total) {
            total_ = total;
        }
        ~SumFielder() { }

        void accept(bool b) { }
        void accept(float f) { }
        void accept(String* s) { }
        void accept(int i) {
            total_ += i;
        }

        size_t get_total() {
            return total_;
        }
};

/**
 * A Rower that adds up every int it finds in a row using a Fielder.
 * 
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
 */
class SumRower : public Rower {
    public:
        SumFielder* sf_;
        size_t total_;

        SumRower() {
            total_ = 0;
            sf_ = new SumFielder(total_);
        }
        ~SumRower() { delete sf_; }

        bool accept(Row& r) {
            r.visit(r.get_idx(), *sf_);
            total_ += sf_->get_total();
        }

        size_t get_total() {
            return total_;
        }

        void join_delete(Rower* other) {
            SumRower* o = dynamic_cast<SumRower*>(other);
            total_ += o->get_total();
            delete o;
        }

        Object* clone() {
            return new SumRower();
        }
};

void test1() {
    Schema s("IS");
    DataFrame df(s);
    Row r(s);
    for (int i = 1; i <= 100; i++) {
        r.set(0, i);
        r.set(1, new String("foo"));
        df.add_row(r);
    }
    SumRower* sr = new SumRower();
    df.map(*sr);
    assert(sr->get_total() == 5050);
    delete sr;
    printf("Test1 passed\n");
}

int main(int argc, char** argv) {
    // ParserMain* pf = new ParserMain(argc, argv);
    test1();
    return 0;
}