#ifndef _JNP1_PRIORITYQUEUE_HH_
#define _JNP1_PRIORITYQUEUE_HH_

class PriorityQueueEmptyException {
  // TODO: inherit from some std::exception?
};

class PriorityQueueNotFoundException {
  // TODO: j.w.
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
  template<typename sorter> using elements = std::multiset<element, sorter>;
  using elements_by_key_t = std::map<key_ptr, std::multiset<element>>;

protected:
  // TODO: add custom sorters that will sort pairs
  // sortowanie po wartości, a potem po kluczu
  std::multiset<element> sorted_by_value;
  // sortowanie po kluczu, wszystkie wartości są równe
  std::map<key_ptr, std::multiset<value_ptr>> sorted_by_key;

public:
  // Konstruktor bezparametrowy tworzący pustą kolejkę [O(1)]
  PriorityQueue() = default;

  // Konstruktor kopiujący [O(queue.size())]
  PriorityQueue(const PriorityQueue<K, V>& queue) = default;

  // Konstruktor przenoszący [O(1)]
  PriorityQueue(PriorityQueue<K, V>&& queue)
    : sorted_by_key(std::move(queue.sorted_by_key)),
      sorted_by_value(std::move(queue.sorted_by_value)) {
  }

  // Operator przypisania [O(queue.size()) dla użycia P = Q, a O(1) dla użycia
  // P = move(Q)]
  // TODO: w treści była inna definicja operatora przypisania,
  // ale chyba trzeba rozbić ją na dwa konstruktory
  PriorityQueue<K, V>& operator=(const PriorityQueue<K, V>& queue) {
    if(this == &queue) return *this;
    PriorityQueue<K, V> tmp(queue);
    this->swap(tmp);
    return *this;
  }

  PriorityQueue<K, V>& operator=(PriorityQueue<K, V>&& queue) {
    this->sorted_by_value = queue.sorted_by_value;
    this->sorted_by_key = queue.sorted_by_key;
    return *this;
  }

  // Metoda zwracająca true wtedy i tylko wtedy, gdy kolejka jest pusta [O(1)]
  bool empty() const {
    return sorted_by_value.empty();
  }

  // Metoda zwracająca liczbę par (klucz, wartość) przechowywanych w kolejce
  // [O(1)]
  size_type size() const {
    return sorted_by_value.size();
  }

  // Metoda wstawiająca do kolejki parę o kluczu key i wartości value
  // [O(log size())] (dopuszczamy możliwość występowania w kolejce wielu
  // par o tym samym kluczu)
  void insert(const K& key, const V& value) {
    auto k = key_ptr(key);
    auto v = key_ptr(value);
    sorted_by_value.insert(make_pair(k, v));
    sorted_by_key[k].insert(v);
  }

  // Metody zwracające odpowiednio najmniejszą i największą wartość przechowywaną
  // w kolejce [O(1)]; w przypadku wywołania którejś z tych metod na pustej
  // strukturze powinien zostać zgłoszony wyjątek PriorityQueueEmptyException
  const V& minValue() const {
    if(empty()) throw PriorityQueueEmptyException();
    return sorted_by_value.begin()->second->get();
  }
  const V& maxValue() const {
    if(empty()) throw PriorityQueueEmptyException();
    return sorted_by_value.rbegin()->second->get();
  }

  // Metody zwracające klucz o przypisanej odpowiednio najmniejszej lub
  // największej wartości [O(1)]; w przypadku wywołania którejś z tych metod
  // na pustej strukturze powinien zostać zgłoszony wyjątek
  // PriorityQueueEmptyException
  const K& minKey() const {
    if(empty()) throw PriorityQueueEmptyException();
    return sorted_by_value.begin()->first->get();
  }
  const K& maxKey() const {
    if(empty()) throw PriorityQueueEmptyException();
    return sorted_by_value.rbegin()->first->get();
  }

  // Metody usuwające z kolejki jedną parę o odpowiednio najmniejszej lub
  // największej wartości [O(log size())]
  void deleteMin() {
    // TODO: znajduje najmniejszy element,
    // po czym znajduje, gdzie się on znajduje w drugim secie
    if(empty()) {
      // TODO
      return;
    }
    element e = *(sorted_by_value.begin());
    sorted_by_value.erase(sorted_by_value.begin());
    sorted_by_key[e.first].erase(e.second);
    if(sorted_by_key[e.first].empty()) sorted_by_key.erase(e.first);
  }
  void deleteMax() {
    // TODO: j.w.
    if(empty()) {
      // TODO
      return;
    }
    element e = *(sorted_by_value.rbegin());
    sorted_by_value.erase(sorted_by_value.rbegin());
    sorted_by_key[e.first].erase(e.second);
    if(sorted_by_key[e.first].empty()) sorted_by_key.erase(e.first);
  }

  // Metoda zmieniająca dotychczasową wartość przypisaną kluczowi key na nową
  // wartość value [O(log size())]; w przypadku gdy w kolejce nie ma pary
  // o kluczu key, powinien zostać zgłoszony wyjątek
  // PriorityQueueNotFoundException(); w przypadku kiedy w kolejce jest kilka par
  // o kluczu key, zmienia wartość w dowolnie wybranej parze o podanym kluczu

  // sprawdza, czy do danej pary odwołuje się tylko jeden zestaw wskaźników
  // inaczej musi zaalokować nową parę (by modyfikacja nie dosięgła innych par)
  void changeValue(const K& key, const V& value) {
    auto k = key_ptr(key);

    auto es_it = sorted_by_key.find(k);
    if(es_it == sorted_by_key.end()) throw PriorityQueueNotFoundException();

    assert(!es_it->empty());
    auto ov = *(es_it->begin());
    es_it->erase(es_it->begin());
    sorted_by_value->erase(make_pair(k, ov));

    auto nv = value_ptr(value);
    es_it->insert(nv);
    sorted_by_value->insert(make_pair(k, nv));
  }

  // Metoda scalająca zawartość kolejki z podaną kolejką queue; ta operacja usuwa
  // wszystkie elementy z kolejki queue i wstawia je do kolejki *this
  // [O(size() + queue.size() * log (queue.size() + size()))]
  void merge(PriorityQueue<K, V>& queue) {
    if(this == &queue) return;
    for(element e : sorted_by_value) {
      key_ptr k = e.first;
      value_ptr v = e.second;
      sorted_by_value.insert(make_pair(k, v));
      sorted_by_key[k].insert(v);
    }
  }

  // Metoda zamieniającą zawartość kolejki z podaną kolejką queue (tak jak
  // większość kontenerów w bibliotece standardowej) [O(1)]
  void swap(PriorityQueue<K, V>& queue) {
    if(this == &queue) return;
    this->sorted_by_value.swap(queue.sorted_by_value);
    this->sorted_by_key.swap(queue.sorted_by_key);
  }
};

// TODO: dlaczego chcą dwa swapy? - jakaś pułapka?
template<typename K, typename V>
void swap(PriorityQueue<K,V> &lhs, PriorityQueue<K,V> &rhs) {
  lhs.swap(rhs);
}

template<typename K, typename V>
bool operator==(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
  return lhs.sorted_by_value == rhs.sorted_by_value;
}
template<typename K, typename V>
bool operator!=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
  return lhs.sorted_by_value != rhs.sorted_by_value;
}

template<typename K, typename V>
bool operator< (const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
  // TODO: use std::lexicographical_compare on sorted_by_value
}
template<typename K, typename V>
bool operator> (const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
  // TODO: use std::lexicographical_compare on sorted_by_value
}
template<typename K, typename V>
bool operator<=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
  // TODO: use std::lexicographical_compare on sorted_by_value
}
template<typename K, typename V>
bool operator>=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs) {
  // TODO: use std::lexicographical_compare on sorted_by_value
}

#endif /* end of include guard: _JNP1_PRIORITYQUEUE_HH_ */
