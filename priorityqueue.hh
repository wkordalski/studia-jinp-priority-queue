#ifndef _JNP1_PRIORITYQUEUE_HH_
#define _JNP1_PRIORITYQUEUE_HH_

#include <cassert>
#include <iostream>
#include <exception>
#include <map>
#include <memory>
#include <set>

// TODO: Add some messages in ctors to exceptions objects

// Założenia:
// 1. Nie obsługujemy wyjątku std::bad_alloc, który może zostać rzucony przez
// std::make_shared()
// 2.

class PriorityQueueEmptyException : public std::exception {
   public:
    PriorityQueueEmptyException() = default;
    virtual const char* what() const noexcept(true) {
        return "Priority queue is empty.";
    }
};

class PriorityQueueNotFoundException : public std::exception {
   public:
    PriorityQueueNotFoundException() = default;
    virtual const char* what() const noexcept(true) {
        return "Could not find element in priority queue with specified key.";
    }
};

template <typename K, typename V>
class PriorityQueue {
   public:
    using key_type = K;
    using value_type = V;

   protected:
    using key_ptr = std::shared_ptr<K>;
    using value_ptr = std::shared_ptr<V>;
    using element = std::pair<key_ptr, value_ptr>;

    template <class Compare = std::less<element>>
    using element_set = std::multiset<element, Compare>;

   public:
    using size_type = typename element_set<>::size_type;

   protected:
    // Sorter classes
    // Konstruktory i operatory przypisania muszą być nothrow
    class KeyComparer {
       public:
        bool operator()(const key_ptr& lhs, const key_ptr& rhs) {
            return *lhs < *rhs;
        }
    };

    class ValueComparer {
       public:
        bool operator()(const value_ptr& lhs, const value_ptr& rhs) {
            return *lhs < *rhs;
        }
    };

    class ValueKeyComparer {
       public:
        bool operator()(const element& lhs, const element& rhs) {
            if (*(lhs.second) < *(rhs.second)) return true;
            if (*(rhs.second) < *(lhs.second)) return false;
            return *(lhs.first) < *(rhs.first);
        }
    };

   protected:
    using elements = element_set<ValueKeyComparer>;
    using value_map = std::map<value_ptr, element_set<>, ValueComparer>;
    using key_map = std::map<key_ptr, value_map, KeyComparer>;
    using value_set = std::set<value_ptr, ValueComparer>;

    // sortowanie po wartości, a potem po kluczu
    elements sorted_by_value;
    // sortowanie po kluczu, a potem po wartości, a na koniec po adresach
    // (domyślnie)
    key_map sorted_by_key;

    value_set all_values;

   protected:
    element find_element(const key_ptr& k, const value_ptr& v) {
        using namespace std;

        // jeśli rzucą wyjątki, to trudno...
        auto kit = sorted_by_key.find(k);
        auto vit = all_values.find(v);

        auto kk = (kit == sorted_by_key.end()) ? k : (kit->first);
        auto vv = (vit == all_values.end()) ? v : (*vit);

        return make_pair(kk, vv);
    }
    element find_element(const K& key, const V& value) {
        // w razie czego usuwanie polega na nic nie robieniu,
        // bo nie posiadamy key i value na własność
        auto k = std::make_shared<K>(key);
        auto v = std::make_shared<V>(value);

        return find_element(k, v);
    }

   public:
    // TODO: czy konstruktory na prawdę potrzebują jakiegoś exception-safety?

    // Konstruktor bezparametrowy tworzący pustą kolejkę [O(1)]
    PriorityQueue() = default;

    // Konstruktor kopiujący [O(queue.size())]
    // konstruktory kontenerów zapewniają silną gwarancję
    // jak coś się rzuci, to i tak PriorityQueue się nie utworzy
    // TODO: zweryfikować
    PriorityQueue(const PriorityQueue<K, V>& queue) = default;

    // Konstruktor przenoszący [O(1)]
    PriorityQueue(PriorityQueue<K, V>&& queue) noexcept
        : sorted_by_value(std::move(queue.sorted_by_value)),
          sorted_by_key(std::move(queue.sorted_by_key)),
          all_values(std::move(queue.all_values)) {}

    // Operator przypisania [O(queue.size()) dla użycia P = Q, a O(1) dla użycia
    // P = move(Q)]
    // TODO: w treści była inna definicja operatora przypisania,
    // ale chyba trzeba rozbić ją na dwa konstruktory
    PriorityQueue<K, V>& operator=(const PriorityQueue<K, V>& queue) {
        if (this == &queue) return *this;
        PriorityQueue<K, V> tmp(queue);  // może rzucać tylko bad_alloc
        this->swap(tmp);                 // powinno być noexcept
        return *this;
    }

    PriorityQueue<K, V>& operator=(PriorityQueue<K, V>&& queue) noexcept(true) {
        this->sorted_by_value =
            std::move(queue.sorted_by_value);  // powinno być noexcept (move)
        this->sorted_by_key =
            std::move(queue.sorted_by_key);  // powinno być noexcept (move)
        this->all_values = std::move(queue.all_values);
        // jest noexcept bo kopiowanie shared_ptr jest noexcept
        return *this;
    }

    // Metoda zwracająca true wtedy i tylko wtedy, gdy kolejka jest pusta [O(1)]
    bool empty() const noexcept(true) { return sorted_by_value.empty(); }

    // Metoda zwracająca liczbę par (klucz, wartość) przechowywanych w kolejce
    // [O(1)]
    size_type size() const noexcept(true) { return sorted_by_value.size(); }

    // Metoda wstawiająca do kolejki parę o kluczu key i wartości value
    // [O(log size())] (dopuszczamy możliwość występowania w kolejce wielu
    // par o tym samym kluczu)
    void insert(const K& key, const V& value) {
        key_ptr k;
        value_ptr v;
        std::tie(k, v) = find_element(key, value);

        auto pair_by_value = make_pair(k, v);

        // Iteratory
        typename elements::iterator it1;
        typename key_map::iterator it2;
        typename value_map::iterator it3;
        typename element_set<>::iterator it4;
        typename value_set::iterator it5;

        bool al1 = false, al2 = false, al3 = false, al4 = false, al5 = false;

        // Polegamy na silnej gwarancji kontenerów STL (map, set)
        try {
            it1 = sorted_by_value.insert(pair_by_value);
            al1 = true;

            std::tie(it2, al2) =
                sorted_by_key.insert(std::make_pair(k, value_map()));

            std::tie(it3, al3) =
                it2->second.insert(std::make_pair(v, element_set<>()));

            it4 = it3->second.insert(pair_by_value);
            al4 = true;

            std::tie(it5, al5) = all_values.insert(v);
        } catch (...) {
            if (al5) all_values.erase(it5);
            if (al4) it3->second.erase(it4);
            if (al3) it2->second.erase(it3);
            if (al2) sorted_by_key.erase(it2);
            if (al1) sorted_by_value.erase(it1);
            throw;
        }
    }

    // Metody zwracające odpowiednio najmniejszą i największą wartość
    // przechowywaną
    // w kolejce [O(1)]; w przypadku wywołania którejś z tych metod na pustej
    // strukturze powinien zostać zgłoszony wyjątek PriorityQueueEmptyException
    const V& minValue() const {
        if (empty()) throw PriorityQueueEmptyException();
        // begin() i * - noexcept(true)
        return *(sorted_by_value.begin()->second);
    }
    const V& maxValue() const {
        if (empty()) throw PriorityQueueEmptyException();
        // rbegin() i * - noexcept(true)
        return *(sorted_by_value.rbegin()->second);
    }

    // Metody zwracające klucz o przypisanej odpowiednio najmniejszej lub
    // największej wartości [O(1)]; w przypadku wywołania którejś z tych metod
    // na pustej strukturze powinien zostać zgłoszony wyjątek
    // PriorityQueueEmptyException
    const K& minKey() const {
        if (empty()) throw PriorityQueueEmptyException();
        // begin() i * - noexcept(true)
        return *(sorted_by_value.begin()->first);
    }
    const K& maxKey() const {
        if (empty()) throw PriorityQueueEmptyException();
        // rbegin() i * - noexcept(true)
        return *(sorted_by_value.rbegin()->first);
    }

    // Metody usuwające z kolejki jedną parę o odpowiednio najmniejszej lub
    // największej wartości [O(log size())]
    void deleteMin() {
        if (empty()) return;                            // noexcept
        const element& e = *(sorted_by_value.begin());  // noexcept
        value_ptr v = e.second;

        auto kit =
            sorted_by_key.find(e.first);  // może rzucać operator porównania
        assert(kit != sorted_by_key.end());
        auto vit =
            kit->second.find(e.second);  // może rzucać operator porównania
        assert(vit != kit->second.end());
        auto ait =  // nie rzuca
            vit->second.find(e);
        assert(ait != vit->second.end());
        auto bit = all_values.find(e.second);
        assert(bit != all_values.end());

        // Modyfikacje
        vit->second.erase(ait);                             // noexcept
        if (vit->second.empty()) kit->second.erase(vit);    // noexcept
        if (kit->second.empty()) sorted_by_key.erase(kit);  // noexcept
        sorted_by_value.erase(sorted_by_value.begin());     // noexcept
        // porównuję wskaźniki - noexcept
        if (sorted_by_value.size() == 0 ||
            sorted_by_value.begin()->second != v) {
            all_values.erase(bit);
        }
    }

    void deleteMax() {
        if (empty()) return;                              // noexcept
        const element& e = *prev(sorted_by_value.end());  // noexcept
        value_ptr v = e.second;

        auto kit =
            sorted_by_key.find(e.first);  // może rzucać operator porównania
        assert(kit != sorted_by_key.end());
        auto vit =
            kit->second.find(e.second);  // może rzucać operator porównania
        assert(vit != kit->second.end());
        auto ait =  // nie rzuca
            vit->second.find(e);
        assert(ait != vit->second.end());
        auto bit = all_values.find(e.second);
        assert(bit != all_values.end());

        // Modyfikacje
        vit->second.erase(ait);                              // noexcept
        if (vit->second.empty()) kit->second.erase(vit);     // noexcept
        if (kit->second.empty()) sorted_by_key.erase(kit);   // noexcept
        sorted_by_value.erase(prev(sorted_by_value.end()));  // noexcept
        if (sorted_by_value.size() == 0 ||
            prev(sorted_by_value.begin())->second != v) {
            all_values.erase(bit);
        }
    }

    // Metoda zmieniająca dotychczasową wartość przypisaną kluczowi key na nową
    // wartość value [O(log size())]; w przypadku gdy w kolejce nie ma pary
    // o kluczu key, powinien zostać zgłoszony wyjątek
    // PriorityQueueNotFoundException(); w przypadku kiedy w kolejce jest kilka
    // par
    // o kluczu key, zmienia wartość w dowolnie wybranej parze o podanym kluczu

    // sprawdza, czy do danej pary odwołuje się tylko jeden zestaw wskaźników
    // inaczej musi zaalokować nową parę (by modyfikacja nie dosięgła innych
    // par)
    void changeValue(const K& key, const V& value) {
        auto k = std::make_shared<K>(key);

        auto es_it = sorted_by_key.find(k);
        if (es_it == sorted_by_key.end())
            throw PriorityQueueNotFoundException();

        assert(!es_it->second.empty());
        auto ov = *(es_it->second.begin());

        auto deleted_pair = make_pair(k, ov.first);

        es_it->second.erase(es_it->second.begin());
        sorted_by_value.erase(deleted_pair);

        auto nv = std::make_shared<V>(value);

        try {
            // Polegamy na silnej gwarancji kontenerów STL (map, set)
            insert(key, value);
        } catch (...) {
            // Dodajemy usunięte wcześniej elementy
            es_it->second.insert(ov);
            sorted_by_value.insert(deleted_pair);
            throw;
        }
    }

    // Metoda scalająca zawartość kolejki z podaną kolejką queue; ta operacja
    // usuwa
    // wszystkie elementy z kolejki queue i wstawia je do kolejki *this
    // [O(size() + queue.size() * log (queue.size() + size()))]
    void merge(PriorityQueue<K, V>& queue) {
        if (this == &queue) return;

        PriorityQueue<K, V> merged_queue = *this;
        for (element e : queue.sorted_by_value) {
            key_ptr k = e.first;
            value_ptr v = e.second;

            merged_queue.sorted_by_value.insert(e);
            merged_queue.sorted_by_key[k][v].insert(e);
        }

        queue.sorted_by_value.clear();
        queue.sorted_by_key.clear();

        this->swap(merged_queue);
    }

    // Metoda zamieniającą zawartość kolejki z podaną kolejką queue (tak jak
    // większość kontenerów w bibliotece standardowej) [O(1)]
    // Gwarancja no-throw
    void swap(PriorityQueue<K, V>& queue) noexcept {
        if (this == &queue) return;
        this->sorted_by_value.swap(queue.sorted_by_value);
        this->sorted_by_key.swap(queue.sorted_by_key);
        this->all_values.swap(queue.all_values);
    }

    friend void swap(PriorityQueue<K, V>& lhs,
                     PriorityQueue<K, V>& rhs) noexcept {
        lhs.swap(rhs);
    }

    friend bool operator==(const PriorityQueue<K, V>& lhs,
                           const PriorityQueue<K, V>& rhs) {
        return lhs.sorted_by_value == rhs.sorted_by_value;
    }
    friend bool operator!=(const PriorityQueue<K, V>& lhs,
                           const PriorityQueue<K, V>& rhs) {
        return lhs.sorted_by_value != rhs.sorted_by_value;
    }
    friend bool operator<(const PriorityQueue<K, V>& lhs,
                          const PriorityQueue<K, V>& rhs) {
        return std::lexicographical_compare(
            lhs.sorted_by_value.begin(), lhs.sorted_by_value.end(),
            rhs.sorted_by_value.begin(), rhs.sorted_by_value.end(),
            ValueKeyComparer());
    }
    friend bool operator>(const PriorityQueue<K, V>& lhs,
                          const PriorityQueue<K, V>& rhs) {
        return rhs < lhs;
    }
    friend bool operator<=(const PriorityQueue<K, V>& lhs,
                           const PriorityQueue<K, V>& rhs) {
        return !(lhs > rhs);
    }
    friend bool operator>=(const PriorityQueue<K, V>& lhs,
                           const PriorityQueue<K, V>& rhs) {
        return !(lhs < rhs);
    }
};

#endif /* end of include guard: _JNP1_PRIORITYQUEUE_HH_ */
