#ifndef _JNP1_PRIORITYQUEUE_HH_
#define _JNP1_PRIORITYQUEUE_HH_

#include <exception>
#include <map>
#include <memory>
#include <set>

// TODO: Add some messages in ctors to exceptions objects

class PriorityQueueEmptyException : public std::exception {
public:
  PriorityQueueEmptyException() = default;
  virtual const char * what() const noexcept(true) {
    return "Priority queue is empty.";
  }
};

class PriorityQueueNotFoundException : public std::exception {
public:
  PriorityQueueNotFoundException() = default;
  virtual const char * what() const noexcept(true) {
    return "Could not find element in priority queue with specified key.";
  }
};

template<typename K, typename V>
class PriorityQueue {
public:
  typedef size_t size_type;
  typedef K key_type;
  typedef V value_type;

protected:
  // UWAGA: przy changeValue i merge musimy sprawdzić ilość referencji,
  // aby ewentualnie zrobić głębokie kopie obiektów
  // nie możemy tego wykonać wcześniej ze względu na wymagane O(n)
  // w konstruktorze kopiującym.
  using key_ptr = std::shared_ptr<K>;
  using value_ptr = std::shared_ptr<V>;
  using element = std::pair<key_ptr, value_ptr>;

protected:
  // Sorter classes
  // Konstruktory i operatory przypisania muszą być nothrow
  class KeyComparer {
  public:
    bool operator () (const key_ptr& lhs, const key_ptr& rhs) {
      return *lhs < *rhs;
    }
  };

  class ValueComparer {
  public:
    bool operator () (const value_ptr& lhs, const value_ptr& rhs) {
      return *lhs < *rhs;
    }
  };

  class ValueKeyComparer {
  public:
    bool operator () (const element& lhs, const element& rhs) {
      if(*(lhs.second) < *(rhs.second)) return true;
      if(*(rhs.second) < *(lhs.second)) return false;
      return *(lhs.first) < *(rhs.first);
    }
  };

protected:
  // sortowanie po wartości, a potem po kluczu
  std::multiset<element, ValueKeyComparer> sorted_by_value;
  // sortowanie po kluczu, a potem po wartości
  std::map<key_ptr, std::multiset<value_ptr, ValueComparer>, KeyComparer> sorted_by_key;

public:
  // TODO: czy konstruktory na prawdę potrzebują jakiegoś exception-safety?

  // Konstruktor bezparametrowy tworzący pustą kolejkę [O(1)]
  PriorityQueue() = default;

  // Konstruktor kopiujący [O(queue.size
  // konstruktory kontenerów zapewniają silną gwarancję
  // jak coś się rzuci, to i tak PriorityQueue się nie utworzy
  // TODO: zweryfikować
  PriorityQueue(const PriorityQueue<K, V>& queue) = default;

  // Konstruktor przenoszący [O(1)]
  PriorityQueue(PriorityQueue<K, V>&& queue) noexcept(true)
    : sorted_by_value(std::move(queue.sorted_by_value)),
      sorted_by_key(std::move(queue.sorted_by_key)) {
  }

  // Operator przypisania [O(queue.size()) dla użycia P = Q, a O(1) dla użycia
  // P = move(Q)]
  // TODO: w treści była inna definicja operatora przypisania,
  // ale chyba trzeba rozbić ją na dwa konstruktory
  PriorityQueue<K, V>& operator=(const PriorityQueue<K, V>& queue) {
    if(this == &queue) return *this;
    PriorityQueue<K, V> tmp(queue);                 // może rzucać tylko bad_alloc
    this->swap(tmp);                                // powinno być noexcept
    return *this;
  }

  PriorityQueue<K, V>& operator=(PriorityQueue<K, V>&& queue) noexcept(true) {
    this->sorted_by_value = std::move(queue.sorted_by_value);    // powinno być noexcept (move)
    this->sorted_by_key = std::move(queue.sorted_by_key);        // powinno być noexcept (move)
    // jest noexcept bo kopiowanie shared_ptr jest noexcept
    return *this;
  }

  // Metoda zwracająca true wtedy i tylko wtedy, gdy kolejka jest pusta [O(1)]
  bool empty() const noexcept(true) {
    return sorted_by_value.empty();
  }

  // Metoda zwracająca liczbę par (klucz, wartość) przechowywanych w kolejce
  // [O(1)]
  size_type size() const noexcept(true) {
    return sorted_by_value.size();
  }

  // Metoda wstawiająca do kolejki parę o kluczu key i wartości value
  // [O(log size())] (dopuszczamy możliwość występowania w kolejce wielu
  // par o tym samym kluczu)
  void insert(const K& key, const V& value) {
    auto k = std::make_shared<K>(key);
    auto v = std::make_shared<V>(value);
    sorted_by_value.insert(make_pair(k, v));
    sorted_by_key[k].insert(v);
  }

  // Metody zwracające odpowiednio najmniejszą i największą wartość przechowywaną
  // w kolejce [O(1)]; w przypadku wywołania którejś z tych metod na pustej
  // strukturze powinien zostać zgłoszony wyjątek PriorityQueueEmptyException
  const V& minValue() const {
    if(empty()) throw PriorityQueueEmptyException();
    // begin() i * - noexcept(true)
    return *(sorted_by_value.begin()->second);
  }
  const V& maxValue() const {
    if(empty()) throw PriorityQueueEmptyException();
    // rbegin() i * - noexcept(true)
    return *(sorted_by_value.rbegin()->second);
  }

  // Metody zwracające klucz o przypisanej odpowiednio najmniejszej lub
  // największej wartości [O(1)]; w przypadku wywołania którejś z tych metod
  // na pustej strukturze powinien zostać zgłoszony wyjątek
  // PriorityQueueEmptyException
  const K& minKey() const {
    if(empty()) throw PriorityQueueEmptyException();
    // begin() i * - noexcept(true)
    return *(sorted_by_value.begin()->first);
  }
  const K& maxKey() const {
    if(empty()) throw PriorityQueueEmptyException();
    // rbegin() i * - noexcept(true)
    return *(sorted_by_value.rbegin()->first);
  }

  // Metody usuwające z kolejki jedną parę o odpowiednio najmniejszej lub
  // największej wartości [O(log size())]
  void deleteMin() {
    if(empty()) return;                             // noexcept
    const element& e = *(sorted_by_value.begin());  // noexcept

    auto kit = sorted_by_key.find(e.first);         // może rzucać operator porównania
    assert(kit != sorted_by_key.end());
    auto vit = kit->second.find(e.second);          // może rzucać operator porównania
    assert(vit != kit->second.end());

    // Modyfikacje
    kit->second.erase(vit);                          // noexcept
    if(kit->second.empty()) sorted_by_key.erase(kit); // noexcept
    sorted_by_value.erase(sorted_by_value.begin());   // noexcept

  }
  void deleteMax() {
    if(empty()) return;                             // noexcept
    const element& e = *prev(sorted_by_value.end()); // noexcept

    auto kit = sorted_by_key.find(e.first);         // może rzucać operator porównania
    assert(kit != sorted_by_key.end());
    auto vit = kit->second.find(e.second);          // może rzucać operator porównania
    assert(vit != kit->second.end());

    // Modyfikacje
    kit->second.erase(vit);                          // noexcept
    if(kit->second.empty()) sorted_by_key.erase(kit); // noexcept
    sorted_by_value.erase(prev(sorted_by_value.end()));  // noexcept
  }

  // Metoda zmieniająca dotychczasową wartość przypisaną kluczowi key na nową
  // wartość value [O(log size())]; w przypadku gdy w kolejce nie ma pary
  // o kluczu key, powinien zostać zgłoszony wyjątek
  // PriorityQueueNotFoundException(); w przypadku kiedy w kolejce jest kilka par
  // o kluczu key, zmienia wartość w dowolnie wybranej parze o podanym kluczu

  // sprawdza, czy do danej pary odwołuje się tylko jeden zestaw wskaźników
  // inaczej musi zaalokować nową parę (by modyfikacja nie dosięgła innych par)
  void changeValue(const K& key, const V& value) {
    auto k = std::make_shared<K>(key);

    auto es_it = sorted_by_key.find(k);
    if(es_it == sorted_by_key.end()) throw PriorityQueueNotFoundException();

    assert(!es_it->second.empty());
    auto ov = *(es_it->second.begin());
    es_it->second.erase(es_it->second.begin());
    sorted_by_value.erase(make_pair(k, ov));

    auto nv = std::make_shared<V>(value);
    es_it->second.insert(nv);
    sorted_by_value.insert(make_pair(k, nv));
  }

  // Metoda scalająca zawartość kolejki z podaną kolejką queue; ta operacja usuwa
  // wszystkie elementy z kolejki queue i wstawia je do kolejki *this
  // [O(size() + queue.size() * log (queue.size() + size()))]
  void merge(PriorityQueue<K, V>& queue) {
    if(this == &queue) return;
    for(element e : queue.sorted_by_value) {
      key_ptr k = e.first;
      value_ptr v = e.second;
      sorted_by_value.insert(make_pair(k, v));
      sorted_by_key[k].insert(v);
    }
    queue.sorted_by_value.clear();
    queue.sorted_by_key.clear();
  }

  // Metoda zamieniającą zawartość kolejki z podaną kolejką queue (tak jak
  // większość kontenerów w bibliotece standardowej) [O(1)]
  void swap(PriorityQueue<K, V>& queue) noexcept(true) {
    if(this == &queue) return;
    this->sorted_by_value.swap(queue.sorted_by_value);
    this->sorted_by_key.swap(queue.sorted_by_key);
  }

  friend void swap(PriorityQueue<K,V> &lhs, PriorityQueue<K,V> &rhs) noexcept(true) {
    lhs.swap(rhs);
  }

  friend bool operator==(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
    return lhs.sorted_by_value == rhs.sorted_by_value;
  }
  friend bool operator!=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
    return lhs.sorted_by_value != rhs.sorted_by_value;
  }
  friend bool operator< (const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
    return std::lexicographical_compare(
      lhs.sorted_by_value.begin(), lhs.sorted_by_value.end(),
      rhs.sorted_by_value.begin(), rhs.sorted_by_value.end(),
      ValueKeyComparer());
  }
  friend bool operator<=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
    return ! lhs > rhs;
  }
  friend bool operator> (const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
    return rhs < lhs;
  }
  friend bool operator>=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
    return ! lhs < rhs;
  }
};

#endif /* end of include guard: _JNP1_PRIORITYQUEUE_HH_ */
