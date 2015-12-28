#ifndef _JNP1_PRIORITYQUEUE_HH_
#define _JNP1_PRIORITYQUEUE_HH_

template<typename K, typename V>
class PriorityQueue {
public:
  typedef size_t size_type;
  typedef K key_type;
  typedef V value_type;

public:
  // Konstruktor bezparametrowy tworzący pustą kolejkę [O(1)]
  PriorityQueue();

  // Konstruktor kopiujący [O(queue.size())]
  PriorityQueue(const PriorityQueue<K, V>& queue);

  // Konstruktor przenoszący [O(1)]
  PriorityQueue(PriorityQueue<K, V>&& queue);

  // Operator przypisania [O(queue.size()) dla użycia P = Q, a O(1) dla użycia
  // P = move(Q)]
  PriorityQueue<K, V>& operator=(PriorityQueue<K, V> queue);

  // Metoda zwracająca true wtedy i tylko wtedy, gdy kolejka jest pusta [O(1)]
  bool empty() const;

  // Metoda zwracająca liczbę par (klucz, wartość) przechowywanych w kolejce
  // [O(1)]
  size_type size() const;

  // Metoda wstawiająca do kolejki parę o kluczu key i wartości value
  // [O(log size())] (dopuszczamy możliwość występowania w kolejce wielu
  // par o tym samym kluczu)
  void insert(const K& key, const V& value);

  // Metody zwracające odpowiednio najmniejszą i największą wartość przechowywaną
  // w kolejce [O(1)]; w przypadku wywołania którejś z tych metod na pustej
  // strukturze powinien zostać zgłoszony wyjątek PriorityQueueEmptyException
  const V& minValue() const;
  const V& maxValue() const;

  // Metody zwracające klucz o przypisanej odpowiednio najmniejszej lub
  // największej wartości [O(1)]; w przypadku wywołania którejś z tych metod
  // na pustej strukturze powinien zostać zgłoszony wyjątek
  // PriorityQueueEmptyException
  const K& minKey() const;
  const K& maxKey() const;

  // Metody usuwające z kolejki jedną parę o odpowiednio najmniejszej lub
  // największej wartości [O(log size())]
  void deleteMin();
  void deleteMax();

  // Metoda zmieniająca dotychczasową wartość przypisaną kluczowi key na nową
  // wartość value [O(log size())]; w przypadku gdy w kolejce nie ma pary
  // o kluczu key, powinien zostać zgłoszony wyjątek
  // PriorityQueueNotFoundException(); w przypadku kiedy w kolejce jest kilka par
  // o kluczu key, zmienia wartość w dowolnie wybranej parze o podanym kluczu
  void changeValue(const K& key, const V& value);

  // Metoda scalająca zawartość kolejki z podaną kolejką queue; ta operacja usuwa
  // wszystkie elementy z kolejki queue i wstawia je do kolejki *this
  // [O(size() + queue.size() * log (queue.size() + size()))]

  // TODO: lepiej skopiować std::shared_ptr zamiast kopiować obiekty
  // zdaje się, że to może być pożądane zachowanie.
  void merge(PriorityQueue<K, V>& queue);

  // Metoda zamieniającą zawartość kolejki z podaną kolejką queue (tak jak
  // większość kontenerów w bibliotece standardowej) [O(1)]
  void swap(PriorityQueue<K, V>& queue);
};

// TODO: dlaczego chcą dwa swapy? - jakaś pułapka?
template<typename K, typename V>
void swap(PriorityQueue<K,V> &lhs, PriorityQueue<K,V> &rhs) {
  lhs.swap(rhs);
}

template<typename K, typename V>
bool operator==(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs);
template<typename K, typename V>
bool operator!=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs);

template<typename K, typename V>
bool operator< (const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs);
template<typename K, typename V>
bool operator> (const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs);
template<typename K, typename V>
bool operator<=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs);
template<typename K, typename V>
bool operator>=(const PriorityQueue<K,V> &lhs, const PriorityQueue<K,V> &rhs);

#endif /* end of include guard: _JNP1_PRIORITYQUEUE_HH_ */
