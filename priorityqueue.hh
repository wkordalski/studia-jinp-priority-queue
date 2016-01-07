#ifndef _JNP1_PRIORITYQUEUE_HH_
#define _JNP1_PRIORITYQUEUE_HH_

#include <cassert>
#include <exception>
#include <map>
#include <memory>
#include <set>

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
    // Komparatory
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
    using value_set = std::multiset<value_ptr, ValueComparer>;

    // sortowanie po wartości, a potem po kluczu
    elements sorted_by_value;
    // sortowanie po kluczu, a potem po wartości, a na koniec po adresach
    key_map sorted_by_key;
    // set z wartoścami trzymanymi w kolejce
    value_set all_values;

   protected:
    element find_element(const key_ptr& k, const value_ptr& v) {
        auto kit = sorted_by_key.find(k);
        auto vit = all_values.find(v);

        auto kk = (kit == sorted_by_key.end()) ? k : (kit->first);
        auto vv = (vit == all_values.end()) ? v : (*vit);

        return make_pair(kk, vv);
    }
    element find_element(const K& key, const V& value) {
        auto k = std::make_shared<K>(key);
        auto v = std::make_shared<V>(value);

        return find_element(k, v);
    }

   public:
    // Konstruktor bezparametrowy tworzący pustą kolejkę [O(1)]
    PriorityQueue() = default;

    // Konstruktor kopiujący [O(queue.size())]
    // konstruktory kontenerów zapewniają silną gwarancję
    // jak coś się rzuci, to i tak PriorityQueue się nie utworzy
    PriorityQueue(const PriorityQueue<K, V>& queue) = default;

    // Konstruktor przenoszący [O(1)]
    PriorityQueue(PriorityQueue<K, V>&& queue) noexcept
        : sorted_by_value(std::move(queue.sorted_by_value)),
          sorted_by_key(std::move(queue.sorted_by_key)),
          all_values(std::move(queue.all_values)) {}

    // Operator przypisania [O(queue.size()) dla użycia P = Q, a O(1) dla użycia
    // P = move(Q)]
    PriorityQueue<K, V>& operator=(const PriorityQueue<K, V>& queue) {
        if (this == &queue) return *this;
        PriorityQueue<K, V> tmp(queue);
        this->swap(tmp);
        return *this;
    }

    PriorityQueue<K, V>& operator=(PriorityQueue<K, V>&& queue) noexcept(true) {
        this->sorted_by_value = std::move(queue.sorted_by_value);
        this->sorted_by_key = std::move(queue.sorted_by_key);
        this->all_values = std::move(queue.all_values);
        return *this;
    }

    // Metoda zwracająca true wtedy i tylko wtedy, gdy kolejka jest pusta [O(1)]
    bool empty() const noexcept { return sorted_by_value.empty(); }

    // Metoda zwracająca liczbę par (klucz, wartość) przechowywanych w kolejce
    // [O(1)]
    size_type size() const noexcept { return sorted_by_value.size(); }

    // Metoda wstawiająca do kolejki parę o kluczu key i wartości value
    // [O(log size())] (dopuszczamy możliwość występowania w kolejce wielu
    // par o tym samym kluczu)
    void insert(const K& key, const V& value) {
        using std::make_pair;
        using std::tie;

        key_ptr k;
        value_ptr v;
        tie(k, v) = find_element(key, value);

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

            tie(it2, al2) = sorted_by_key.insert(make_pair(k, value_map()));

            tie(it3, al3) = it2->second.insert(make_pair(v, element_set<>()));

            it4 = it3->second.insert(pair_by_value);
            al4 = true;

            it5 = all_values.insert(v);
            al5 = true;
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
        return *(sorted_by_value.begin()->second);
    }
    const V& maxValue() const {
        if (empty()) throw PriorityQueueEmptyException();
        return *(sorted_by_value.rbegin()->second);
    }

    // Metody zwracające klucz o przypisanej odpowiednio najmniejszej lub
    // największej wartości [O(1)]; w przypadku wywołania którejś z tych metod
    // na pustej strukturze powinien zostać zgłoszony wyjątek
    // PriorityQueueEmptyException
    const K& minKey() const {
        if (empty()) throw PriorityQueueEmptyException();
        return *(sorted_by_value.begin()->first);
    }
    const K& maxKey() const {
        if (empty()) throw PriorityQueueEmptyException();
        return *(sorted_by_value.rbegin()->first);
    }

    // Metody usuwające z kolejki jedną parę o odpowiednio najmniejszej lub
    // największej wartości [O(log size())]
    void deleteMin() {
        if (empty()) return;
        const element& e = *(sorted_by_value.begin());
        value_ptr v = e.second;

        auto kit = sorted_by_key.find(e.first);
        assert(kit != sorted_by_key.end());
        auto vit = kit->second.find(e.second);
        assert(vit != kit->second.end());
        auto ait = vit->second.begin();
        assert(ait != vit->second.end());
        auto bit = all_values.find(e.second);
        assert(bit != all_values.end());

        // Modyfikacje
        vit->second.erase(ait);
        if (vit->second.empty()) kit->second.erase(vit);
        if (kit->second.empty()) sorted_by_key.erase(kit);
        sorted_by_value.erase(sorted_by_value.begin());
        all_values.erase(bit);
    }

    void deleteMax() {
        if (empty()) return;
        const element& e = *prev(sorted_by_value.end());
        value_ptr v = e.second;

        auto kit = sorted_by_key.find(e.first);
        assert(kit != sorted_by_key.end());
        auto vit = kit->second.find(e.second);
        assert(vit != kit->second.end());
        auto ait = vit->second.begin();
        assert(ait != vit->second.end());
        auto bit = all_values.find(e.second);
        assert(bit != all_values.end());

        // Modyfikacje
        vit->second.erase(ait);
        if (vit->second.empty()) kit->second.erase(vit);
        if (kit->second.empty()) sorted_by_key.erase(kit);
        sorted_by_value.erase(prev(sorted_by_value.end()));
        all_values.erase(bit);
    }

    // Metoda zmieniająca dotychczasową wartość przypisaną kluczowi key na nową
    // wartość value [O(log size())]; w przypadku gdy w kolejce nie ma pary
    // o kluczu key, powinien zostać zgłoszony wyjątek
    // PriorityQueueNotFoundException(); w przypadku kiedy w kolejce jest kilka
    // par
    // o kluczu key, zmienia wartość w dowolnie wybranej parze o podanym kluczu
    void changeValue(const K& key, const V& value) {
        using std::make_pair;
        using std::tie;

        key_ptr k;
        value_ptr v;
        tie(k, v) = find_element(key, value);

        auto kit = sorted_by_key.find(k);
        if (kit == sorted_by_key.end()) throw PriorityQueueNotFoundException();

        value_ptr old = kit->second.begin()->first;

        auto itr_e1 = sorted_by_value.find(make_pair(k, old));
        auto itr_e2 = all_values.find(old);
        auto vit = kit->second.find(old);
        assert(vit != kit->second.end());

        // Wstawmy najpierw nową parę...

        auto pair_by_value = make_pair(k, v);

        // Iterators
        typename elements::iterator it1;
        typename key_map::iterator it2;
        typename value_map::iterator it3;
        typename element_set<>::iterator it4;
        typename value_set::iterator it5;
        // If we have to remove them on fail.
        bool al1 = false, al2 = false, al3 = false, al4 = false, al5 = false;
        // Polegamy na silnej gwarancji kontenerów STL (map, set)
        try {
            it1 = sorted_by_value.insert(pair_by_value);
            al1 = true;

            tie(it2, al2) = sorted_by_key.insert(make_pair(k, value_map()));

            tie(it3, al3) = it2->second.insert(make_pair(v, element_set<>()));

            it4 = it3->second.insert(pair_by_value);
            al4 = true;

            it5 = all_values.insert(v);
            al5 = true;
        } catch (...) {
            if (al5) all_values.erase(it5);
            if (al4) it3->second.erase(it4);
            if (al3) it2->second.erase(it3);
            if (al2) sorted_by_key.erase(it2);
            if (al1) sorted_by_value.erase(it1);
            throw;
        }
        // A teraz usuńmy starą
        sorted_by_value.erase(itr_e1);
        all_values.erase(itr_e2);
        vit->second.erase(vit->second.begin());
        if (vit->second.size() == 0) kit->second.erase(vit);
    }

    // Metoda scalająca zawartość kolejki z podaną kolejką queue; ta operacja
    // usuwa
    // wszystkie elementy z kolejki queue i wstawia je do kolejki *this
    // [O(size() + queue.size() * log (queue.size() + size()))]
    void merge(PriorityQueue<K, V>& queue) {
        using std::tie;

        if (this == &queue) return;

        PriorityQueue<K, V> merged_queue = *this;

        for (element e : queue.sorted_by_value) {
            key_ptr k;
            value_ptr v;
            tie(k, v) = merged_queue.find_element(e.first, e.second);

            merged_queue.sorted_by_value.insert(e);
            merged_queue.sorted_by_key[k][v].insert(e);
            merged_queue.all_values.insert(v);
        }
        queue.sorted_by_value.clear();
        queue.sorted_by_key.clear();
        queue.all_values.clear();

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
