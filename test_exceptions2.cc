#include "priorityqueue.hh"

#include <exception>
#include <set>
#include <iostream>

class WrongKeyValueException : public std::exception {
    virtual const char* what() const throw() {
        return "Throwing from Key/Value";
    }
};

// Rzuca wyjatek przez copy ctor
struct ThrowingKey {
    int val;
    ThrowingKey(int val) : val(val){};
    ThrowingKey(const ThrowingKey& other) {
        std::cerr << "Copy constructor" << std::endl;
        // this->val = other.val;
        throw WrongKeyValueException();
    }
    friend bool operator<(const ThrowingKey& v1, const ThrowingKey& v2) {
        return v1.val < v1.val;
    }
};

struct ThrowingValue : ThrowingKey {
    using ThrowingKey::ThrowingKey;
};

struct NonThrowingKey {
    int val;
    NonThrowingKey(int val) : val(val){};
    friend bool operator<(const NonThrowingKey& v1, const NonThrowingKey& v2) {
        return v1.val < v1.val;
    }
};

struct NonThrowingValue : NonThrowingKey {
    using NonThrowingKey::NonThrowingKey;
};

int main(int argc, char* argv[]) {

    // insert() tests
    NonThrowingKey keyNon(42);
    NonThrowingValue valNon(42);
    ThrowingKey key(42);
    ThrowingValue val(42);
    PriorityQueue<NonThrowingKey, NonThrowingValue> qNon;
    PriorityQueue<ThrowingKey, ThrowingValue> q;

    // Mixy (rzuca/nie-rzuca)
    PriorityQueue<NonThrowingKey, ThrowingValue> qMixed1;
    PriorityQueue<ThrowingKey, NonThrowingValue> qMixed2;

    try {
        // std::set<ThrowingKey> throw_checker;
        // throw_checker.insert(key); // Test dzialania w kont. STL (ma rzucac)

        // qMixed1.insert(keyNon, val);  // Ma rzucac
        qMixed2.insert(key, valNon);  // Ma rzucac
        // qNon.insert(keyNon, valNon);  // Nie ma rzucac
        // q.insert(key, val); // Ma rzucac (strong)
        // q.insert(1, 0); // Ma rzucac (strong)
    } catch (std::exception& ex) {
        std::cout << ex.what();
    }
    return 0;
}
