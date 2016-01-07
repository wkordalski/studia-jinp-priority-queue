#include <iostream>
#include <exception>
#include <cassert>

#include "priorityqueue.hh"

PriorityQueue<int, int> f(PriorityQueue<int, int> q)
{
    return q;
}
using namespace std;
int testInt() {
    PriorityQueue<int, int> P = f(PriorityQueue<int, int>());

    assert(P.empty());
    
    P.insert(1, 42);
    P.insert(2, 13);

    assert(P.size() == 2);
    assert(P.maxKey() == 1);
    assert(P.maxValue() == 42);
    assert(P.minKey() == 2);
    assert(P.minValue() == 13);

    try {
        P.changeValue(5, 43);
        assert(!"did not throw");
    }
    catch(PriorityQueueNotFoundException&) {
    }
    catch(...)
    {
        assert(false);
    }
    // check if the same
    assert(P.size() == 2);
    assert(P.maxKey() == 1);
    assert(P.maxValue() == 42);
    assert(P.minKey() == 2);
    assert(P.minValue() == 13);




    P.changeValue(1, 43);
    assert(P.size() == 2);
    assert(P.maxKey() == 1);
    assert(P.maxValue() == 43);
    assert(P.minKey() == 2);
    assert(P.minValue() == 13);

    // back to previous
    P.changeValue(1, 42);
    assert(P.size() == 2);
    assert(P.maxKey() == 1);
    assert(P.maxValue() == 42);
    assert(P.minKey() == 2);
    assert(P.minValue() == 13);



    PriorityQueue<int, int> Q(f(P));

    Q.deleteMax();
    Q.deleteMin();
    Q.deleteMin();

    assert(Q.empty());

    PriorityQueue<int, int> R(Q);

    R.insert(1, 100);
    R.insert(2, 100);
    R.insert(3, 300);
    
    PriorityQueue<int, int> S;
    S = R;

    try
    {
        S.changeValue(4, 400);
        assert(!"did not throw");
    }
    catch (const PriorityQueueNotFoundException& pqnfex)
    {
    }
    catch (...)
    {
        assert(!"exception missing!");
    }
    S.changeValue(2, 200);

    try
    {
        while (true)
        {
            std::cout << "size: " << S.size() << std::endl;
            std::cout << S.minValue() << std::endl;
            S.deleteMin();
            std::cout << "and again!" << std::endl;
        }
        assert(!"S.minValue() on empty S did not throw!");
    }
    catch (const PriorityQueueEmptyException& pqeex)
    {
        std::cout << pqeex.what() << std::endl;
    }
    catch (...)
    {
        assert(!"exception missing!");
    }

    try
    {
        S.minKey();
        assert(!"S.minKey() on empty S did not throw!");
    }
    catch (const std::exception& ex)
    {
    }
    catch (...)
    {
        assert(!"exception missing!");
    }

    try
    {
        S.minKey();
        assert(!"S.minKey() on empty S did not throw!");
    }
    catch (const PriorityQueueEmptyException& ex)
    {
    }
    catch (...)
    {
        std::cout << "???\n";
        assert(!"exception missing!");
    }


    PriorityQueue<int, int> T;
    T.insert(1, 1);
    T.insert(2, 4);
    S.insert(3, 9);
    S.insert(4, 16);

    S.merge(T);
    assert(S.size() == 4);
    assert(S.minValue() == 1);
    assert(S.maxValue() == 16);
    assert(T.empty());

    S = R;
    swap(R, T);
    assert(T == S);
    assert(T != R);

    R = std::move(S);
    assert(T != S);
    assert(T == R);

    std::cout << "ALL OK!" << std::endl;

    return 0;
}


struct WeirdException {
    WeirdException(std::string v) : what(std::move(v)) {
    }
    std::string what;
};
bool THROW_NOW_THIS_IS_MADNESS = false;


struct CopyThrower {
    CopyThrower(bool p = true) : p(p) {
    }
    CopyThrower(const CopyThrower& other) : p(other.p){
        if (p || THROW_NOW_THIS_IS_MADNESS)
            throw WeirdException("copy fail");
    }
    CopyThrower(CopyThrower &&) noexcept {}

    bool operator<(const CopyThrower&) const { return true; }
    bool operator==(const CopyThrower&) const { return true; }

    bool p;
};

struct MoveThrower {
    MoveThrower(bool p = true) : p(p) { }
    MoveThrower(const MoveThrower&) noexcept { }
    MoveThrower(MoveThrower &&) {
        if (p || THROW_NOW_THIS_IS_MADNESS)
            throw WeirdException("move fail");
    }
    bool operator<(const MoveThrower&) const { return true; }
    bool p;
};



struct CompareThrower {
    bool operator<(const CompareThrower &) const {
        if (THROW_NOW_THIS_IS_MADNESS)
            throw WeirdException("move fail");
        return true;
    }
};



void testCompare() {
    PriorityQueue<int, CompareThrower> P;

    P.insert(4, CompareThrower{});
    assert(P.size() == 1);
    CompareThrower t;
    P.insert(5, t);
    assert(P.size() == 2);
    P.insert(42, t);
    P.insert(50, t);
    P.insert(32, t);
    assert(P.size() == 5);

    try {
        THROW_NOW_THIS_IS_MADNESS = true;
        CompareThrower t;
        P.insert(42, t);
    }
    catch (WeirdException &) {
    }
    catch (...) {
        assert(false && "no weird exception");
    }
    assert(P.size() == 5);
    P.deleteMin();
    assert(P.size() == 4);
    P.deleteMax();
    assert(P.size() == 3);


    THROW_NOW_THIS_IS_MADNESS = false;
}


void testCopy()  {
    THROW_NOW_THIS_IS_MADNESS = false;
    PriorityQueue<int, CopyThrower> P;
    P.insert(4, CopyThrower{false});

    assert(P.size() == 1);
    try {
        CopyThrower t(true);
        P.insert(5, t);
    }
    catch (WeirdException &) {
    }
    catch (...) {
        assert(false && "no weird exception");
    }
    assert(P.size() == 1);
    P.deleteMin();

    // no throws
    PriorityQueue<int, CopyThrower> P3 = P;
    P3 = P;
    P3.insert(35, CopyThrower{false});


    try {
        THROW_NOW_THIS_IS_MADNESS = true;
        PriorityQueue<int, CopyThrower> P2(P);
    }
    catch (WeirdException &) {
    }
    catch (...) {
        assert(false && "no weird exception");
    }

    try {
        THROW_NOW_THIS_IS_MADNESS = true;
        PriorityQueue<int, CopyThrower> P2;
        P2 = P;
    }
    catch (WeirdException &) {
    }
    catch (...) {
        assert(false && "no weird exception");
    }


    THROW_NOW_THIS_IS_MADNESS = false;
    PriorityQueue<int, CopyThrower> P2 = P3;

    try {
        THROW_NOW_THIS_IS_MADNESS = true;
        P2 = P; // throws
    }
    catch (WeirdException &) {
        assert(P2 == P3); // P2 is still P3
    }
    catch (...) {
        assert(false && "no weird exception");
    }

    THROW_NOW_THIS_IS_MADNESS = false;
}



void testMove() {
    PriorityQueue<int, MoveThrower> P;

    P.insert(4, MoveThrower{false});
    assert(P.size() == 1);
    try {
        P.insert(5, MoveThrower{true});
    }
    catch(WeirdException&) {
    }
    catch(...) {
        assert(false && "no weird exception");
    }
    assert(P.size() == 1);

    P.deleteMin();
}


void testConst(const PriorityQueue<int, int> &P) {
    assert(!P.empty());
    assert(P.size());

    assert(P == P);
    assert(!(P < P));
    assert(P <= P);
    assert(P >= P);
    assert(!(P != P));
    assert(P >= P);
    assert(!(P > P));
    P.minValue();
    P.minKey();
    P.maxKey();
    P.maxValue();

}

void testWeirdThings() {
    cerr << "test weird things\n";
    PriorityQueue<int, int> P = f(PriorityQueue<int, int>());
    P.insert(42, 42);
    P.insert(1, 43);
    P.insert(43, 1);
    P.insert(1, 42);
    P.insert(42, 1);

    testConst(P);
    PriorityQueue<int, int> P2 = P;
    assert(P2 == P);
    P2 = P2 = P2;
    P = P;
    assert(P2 == P);
    assert(P == P2);
    P.merge(P);
    assert(P2 == P);

    P.merge(P2);
    assert(P2.empty());
    cerr << "test weird things finished\n";
}

#include <random>

std::mt19937 twister(std::random_device{}());
std::uniform_int_distribution<int> distribution(0, 10);

struct RandomThrower {
    RandomThrower() : id(twister()) { }
    RandomThrower(const RandomThrower& p) : id(p.id) {
        throwMeMaybe();
    }

    void throwMeMaybe() const {
        if (distribution(twister) == 0)
            throw WeirdException("random exception");
    }

    bool operator<(const RandomThrower&other) const {
        throwMeMaybe();
        return id < other.id;
    }
    bool operator==(const RandomThrower&other) const {
        //throwMeMaybe(); // We have to check if containter didnt change
        return id == other.id;
    }
    int id;
};

void testRandom() {
    PriorityQueue<int, RandomThrower> P;

    PriorityQueue<int, RandomThrower> Copy;

    for (int i = 0 ; i < 10000; i++) {
        try {
            Copy = P;
        }
        catch(WeirdException&) {
            //std::cerr<< "damn";
            continue;
        }

        try {
            assert(P == Copy);
            assert(!(P < Copy));
        }
        catch(WeirdException&) {
            std::cerr << "exc";
            assert(P == Copy);
        }

        try {
            P.insert(twister(), RandomThrower());
        }
        catch(WeirdException&) {
            std::cerr << "exc";
            assert(P == Copy);
        }
    }
}
size_t wielkosc = 20;
struct BigPieceOfJunk {
    BigPieceOfJunk() : array(wielkosc){
        for (int &p : array)
            p = 42; // jp2gmd: iterujemy po wszystkim zeby pamiec zostala dotknieta
    }
    bool operator<(const BigPieceOfJunk &other) const {
        return false;
    }
    bool operator==(const BigPieceOfJunk &other) const {
        return true;
    }
    std::vector<int> array;
};

void testOutOfMemory1() {
    PriorityQueue<int, BigPieceOfJunk> P, backup;
    cerr << "out of memory test\n";
    while (true) {
        wielkosc *= 1000000;
        std::cerr << wielkosc << endl;
        try {
            backup = P;
        }
        catch (std::bad_alloc&) {
            std::cerr << "catched bad alloc on operator=  ;( \n";
            assert(backup != P);
            break;
        }
        try {
            P.insert(42, {});
            assert(P != backup);
        }
        catch (std::bad_alloc&) {
            std::cerr << "catched bad alloc\n";
            assert(backup == P);
            break;
        }
    }
}

int main() {
    testInt();
    testCopy();
    testCompare();
    testRandom();
    testWeirdThings();
    testOutOfMemory1();

    //testMove(); test nie ma chyba sensu skoro nie przenosimy wartosci do środka, a przenoszenie
    // całej kolejki nie przenosi poszczególnych elementów
}
