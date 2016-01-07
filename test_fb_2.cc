#include <bits/stdc++.h>
#include "priorityqueue.hh"


class SafetyCheckException : public std::exception {};

int numRuns = 0;
int whenThrow = 0;

int check_throw() {
  ++numRuns;

  if (numRuns == whenThrow) {
    throw SafetyCheckException();
  }

  return 0;
}


struct ThrowingInt {
  int *value;

  ThrowingInt() {
    //std::cerr << "Default constructor" << std::endl;
    check_throw();
    value = new int(0);
  }

  ThrowingInt(int val) {
    //std::cerr << "Value constructor, value is " << val << std::endl;
    check_throw();
    value = new int(val);
  }

  ThrowingInt(const ThrowingInt& other) {
    //std::cerr << "Copy constructor" << std::endl;
    check_throw();
    value = new int(*other.value);
  }

  const ThrowingInt& operator=(const ThrowingInt& other) {
    //std::cerr << "Copy assignment" << std::endl;
    check_throw();
    *value = *other.value;
    return *this;
  }

  ~ThrowingInt() {
    delete value;
  }

  bool operator<(const ThrowingInt& other) const {
    check_throw();
    //std::cerr << "Less comparison; " << *value << ", " << *other.value << std::endl;
    return *value < *other.value;
  }

  bool operator==(const ThrowingInt& other) const {
    check_throw();
    return *value == *other.value;
  }

  bool operator==(int x) const {
    //std::cerr << "Int comparison" << std::endl;
    return *value == x;
  }
};


using VP = std::vector<std::pair<int,int>>;


bool check_contents(PriorityQueue<ThrowingInt, ThrowingInt> Q,
                    VP expected) {
  try {
    if (Q.size() != expected.size()) {
      return false;
    }
  
    size_t N = expected.size();

    std::pair<int,int> lastPair{-1e9, -1e9};
    VP userData;
  
    for (size_t i = 0; i < N; ++i) {
      std::pair<int,int> nextPair{*Q.minKey().value, *Q.minValue().value};
      if (nextPair.second < lastPair.second) { return false; }

      userData.push_back(nextPair);
      lastPair = nextPair;

      Q.deleteMin();
    }

    sort(userData.begin(), userData.end());
    sort(expected.begin(), expected.end());
    return userData == expected;

  } catch (...) {
    assert(!"Exception thrown during check");
  }
}

template<class... Args>
bool check_contents(PriorityQueue<ThrowingInt, ThrowingInt> Q,
                    const VP& expected, Args... args) {
  if (check_contents(Q, expected)) {
    return true;
  }
  return check_contents(Q, args...);
}


template<typename T> T id(T value) {
  return value;
}

bool nd_st_cmp(const std::pair<int,int>& lhs, const std::pair<int,int>& rhs) {
  return std::make_pair(lhs.second, lhs.first) < std::make_pair(rhs.second, rhs.first);
}


#define DO_OP(op, chk, ...)                                 \
  do {                                                      \
    try {                                                   \
      op;                                                   \
    } catch (SafetyCheckException e) {                      \
      std::cout << "[" #op "] @ " << __LINE__ << " ";       \
      assert(check_contents(chk, __VA_ARGS__));             \
      return true;                                          \
    }                                                       \
  } while(0)


bool run_test() {
  PriorityQueue<ThrowingInt, ThrowingInt> P;
  VP expected, expectedAlt;
  for (int i = 0; i < 13; ++i) {
    DO_OP(P.insert(ThrowingInt((i * i) % 7), ThrowingInt((i + 3) * (11 - i))),
          P, expected);

    expected.emplace_back((i * i) % 7, (i + 3) * (11 - i));
    sort(expected.begin(), expected.end(), nd_st_cmp);
  }

  PriorityQueue<ThrowingInt, ThrowingInt> Q = id(id(id(P))),
                                          R(Q),
                                          S = std::move(R),
                                          T = S;

  DO_OP(assert(!T.empty()), T, expected);
  DO_OP(assert(T.size() == 13), T, expected);

  DO_OP(assert(T.minValue() == -15), T, expected);
  DO_OP(assert(T.minKey() == 4), T, expected);
  DO_OP(assert(T.maxValue() == 49), T, expected);
  DO_OP(assert(T.maxKey() == 2), T, expected);

  DO_OP(T.deleteMin(), T, expected);

  expected = VP{{2,0}, {2,13}, {4,24}, {0,33}, {1,33}, {0,40}, {1,40},
                           {1,45}, {4,45}, {2,48}, {4,48}, {2,49}};
  DO_OP(T.deleteMin(), T, expected);

  expected = VP{{2,13}, {4,24}, {0,33}, {1,33}, {0,40}, {1,40},
                           {1,45}, {4,45}, {2,48}, {4,48}, {2,49}};
  DO_OP(T.deleteMax(), T, expected);

  expected = VP{{2,13}, {4,24}, {0,33}, {1,33}, {0,40}, {1,40},
                           {1,45}, {4,45}, {2,48}, {4,48}};
  DO_OP(T.deleteMax(), T, expected);

  expected = VP{{2,13}, {4,24}, {0,33}, {1,33}, {0,40}, {1,40},
                           {1,45}, {4,45}, {2,48}};
  expectedAlt = VP{{2,13}, {4,24}, {0,33}, {1,33}, {0,40}, {1,40},
                           {1,45}, {4,45}, {4,48}};
  DO_OP(T.deleteMin(), T, expected, expectedAlt);

  expected = VP{{4,24}, {0,33}, {1,33}, {0,40}, {1,40}, {1,45}, {4,45}, {2,48}};
  expectedAlt = VP{{4,24}, {0,33}, {1,33}, {0,40}, {1,40}, {1,45}, {4,45}, {4,48}};
  DO_OP(T.deleteMax(), T, expected, expectedAlt);

  
  expected = VP{{4,24}, {0,33}, {1,33}, {0,40}, {1,40}, {1,45}, {4,45}};
  DO_OP(assert(T.minValue() == 24), T, expected);
  DO_OP(assert(T.minKey() == 4), T, expected);
  DO_OP(assert(T.maxValue() == 45), T, expected);
  DO_OP(assert(T.maxKey() == 1 || T.maxKey() == 4), T, expected);

  
  DO_OP(T.insert(ThrowingInt(3), ThrowingInt(33)), T, expected);

  expected = VP{{4,24}, {0,33}, {1,33}, {3,33}, {0,40}, {1,40}, {1,45}, {4,45}};
  DO_OP(T.changeValue(ThrowingInt(3), ThrowingInt(8)), T, expected);

  expected = VP{{3,8}, {4,24}, {0,33}, {1,33}, {0,40}, {1,40}, {1,45}, {4,45}};
  DO_OP(assert(T.minValue() == 8), T, expected);
  DO_OP(assert(T.minKey() == 3), T, expected);


  DO_OP(T.changeValue(ThrowingInt(3), ThrowingInt(500)), T, expected);

  expected = VP{{4,24}, {0,33}, {1,33}, {0,40}, {1,40}, {1,45}, {4,45}, {3,500}};
  DO_OP(assert(T.minValue() == 24), T, expected);
  DO_OP(assert(T.minKey() == 4), T, expected);
  DO_OP(assert(T.maxValue() == 500), T, expected);
  DO_OP(assert(T.maxKey() == 3), T, expected);

  bool thrown = false;
  try {
    T.changeValue(ThrowingInt(5), ThrowingInt(400));
  } catch (PriorityQueueNotFoundException e) {
    thrown = true;
  } catch (SafetyCheckException e) {
    assert(check_contents(T, expected));
    return true;
  } catch (...) {
    throw;
  }

  assert(thrown);

  DO_OP(T.changeValue(ThrowingInt(3), ThrowingInt(42)), T, expected);

  ///
  using PQ = PriorityQueue<ThrowingInt, ThrowingInt>;
  std::unique_ptr<PQ> A(new PQ());
  DO_OP(A->insert(ThrowingInt(5), ThrowingInt(40)), *A, VP());
  expected = VP{{5,40}};
  DO_OP(A->insert(ThrowingInt(6), ThrowingInt(42)), *A, expected);
  expected = VP{{5,40}, {6,42}};

  std::unique_ptr<PQ> B(new PQ(*A));
  DO_OP(B->changeValue(ThrowingInt(5), ThrowingInt(30)), *B, expected);

  expectedAlt = VP{{5,30}, {6,42}};

  DO_OP(assert(A->minValue() == 40), *A, expected);
  DO_OP(assert(A->minValue() == 40), *B, expectedAlt);
  DO_OP(assert(!(*A == *B)), *A, expected);
  DO_OP(assert(*A != *B), *B, expectedAlt);
  DO_OP(assert(!(*A < *B)), *A, expected);
  DO_OP(assert(!(*A <= *B)), *B, expectedAlt);
  DO_OP(assert(*A > *B), *A, expected);
  DO_OP(assert(*A >= *B), *B, expectedAlt);

  delete A.release();
  DO_OP(assert(B->maxValue() == 42), *B, expectedAlt);
  delete B.release();

  
  ///
  PQ C, D;
  expected.clear(); expectedAlt.clear();

  for (int i = 0; i < 13; ++i) {
    int key = (i * i) % 7,
        val = (i + 3) * (11 - i);
    PQ& who = (i % 2 == 0) ? C : D;
    VP& whichExp = (i % 2 == 0) ? expected : expectedAlt;

    DO_OP(who.insert(ThrowingInt(key), ThrowingInt(val)), who, whichExp);

    whichExp.emplace_back(key, val);
    sort(whichExp.begin(), whichExp.end(), nd_st_cmp);
  }

  DO_OP(C.merge(D), C, expected);

  expected = VP{{4,-15}, {2,0}, {2,13}, {4,24}, {0,33}, {1,33}, {0,40}, {1,40},
                           {1,45}, {4,45}, {2,48}, {4,48}, {2,49}};
  // assert(C==expected);
  // DO_OP(assert(C == P), C, expected);
  // DO_OP(assert(D.empty()), D, VP());

  // DO_OP(C.swap(D), C, expected);
  // DO_OP(assert(C.empty()), C, VP());
  // DO_OP(assert(P == D), D, expected);
  
  // DO_OP(D.merge(D), D, expected);

  // DO_OP(swap(C, D), D, expected);
  // DO_OP(assert(P == C), C, expected);
  // DO_OP(assert(D.size() == 0), D, VP());

  return false;
}


int main() {
  assert(!run_test());
  int totalRuns = numRuns;
  std::cout << "Number of times check_throw was run: " << totalRuns << std::endl;
  std::cout << "Now trying different throw moments:" << std::endl;


  for (whenThrow = 1; whenThrow <= totalRuns; ++whenThrow) {
    try {
      std::cout << whenThrow << ": ";
      numRuns = 0;
      assert(run_test());
      std::cout << "OK" << std::endl;
    } catch (SafetyCheckException e) {
      std::cout << "OK (thrown @ constructor)" << std::endl;
    }
  }

  return 0;
}
