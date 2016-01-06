#include "priorityqueue.hh"

#include <exception>
#include <iostream>

using namespace std;

#define mad(n) do { if(delay_exceptions > 0) { delay_exceptions--; } else { if(exceptions_on & (1<<n)) throw my_exception(n); } } while(0)
int exceptions_on = 0;
int delay_exceptions = 0;

const int exceptions_all = 0xfffffff;
const int exceptions_none = 0;


class my_exception : public exception {
  int source;
public:
  my_exception(int n) : source(n) {}
  virtual const char * what() const noexcept(true) {
    cout << "Code: " << source << endl;
    return "As U see.";
  }
};

class mad_class {
  int data;

public:
  mad_class() {
    mad(0);
    data = 0;
  }
  mad_class(int x) {
    mad(1);
    data = x;
  }
  mad_class(const mad_class& orig) {
    mad(2);
    data = orig.data;
  }
  mad_class(mad_class&& orig) {
    mad(3);
    data = move(orig.data);
    orig.data = 0;
  }

  mad_class & operator= (const mad_class& orig) {
    mad(4);
    data = orig.data;
    return *this;
  }
  mad_class & operator= (mad_class&& orig) {
    mad(5);
    data = move(orig.data);
    orig.data = 0;
    return *this;
  }

  bool operator== (const mad_class& rhs) {
    mad(6);
    return data == rhs.data;
  }
  bool operator<  (const mad_class& rhs) {
    mad(7);
    return data < rhs.data;
  }

  operator int () const noexcept(true) {
    return data;
  }
};

using mad_queue = PriorityQueue<mad_class, mad_class>;
using mc = mad_class;

int main() {
  //
  // Testing: default ctor
  //
  {
    try {
      exceptions_on = exceptions_all;
      mad_queue q;
      exceptions_on = exceptions_none;
    } catch(const bad_alloc &e) {
      exceptions_on = exceptions_none;
      cout << "Allocation failed - exception possible." << endl;
    } catch(...) {
      exceptions_on = exceptions_none;
      cout << "There should be no exception here - but there is." << endl;
    }
    mad_queue *ptr = NULL;
    try {
      exceptions_on = exceptions_all;
      ptr = new mad_queue();
      delete ptr;
      exceptions_on = exceptions_none;
    } catch(const bad_alloc &e) {
      exceptions_on = exceptions_none;
      cout << "Allocation failed - exception possible." << endl;
      if(ptr != NULL) {
        cout << "But pointer assigned to not-null value." << endl;
      }
    } catch(...) {
      exceptions_on = exceptions_none;
      cout << "There should be no exception here - but there is." << endl;
    }
  }
  //
  // Testing: copying ctor
  //
  {
    mad_queue q;
    q.insert(mc(1), mc(100));
    try {
      exceptions_on = exceptions_all;
      mad_queue r = q;
      exceptions_on = exceptions_none;
      if(r.size() != 1) {
        cout << "Copying not performed correctly." << endl;
      } else {
        if(r.minKey() == 1 && r.minValue() == 100) {
          // OK
        } else {
          cout << "Copying not performed correctly." << endl;
        }
      }
    } catch(const bad_alloc &e) {
      exceptions_on = exceptions_none;
      cout << "Allocation failed - exception possible." << endl;
    } catch(...) {
      exceptions_on = exceptions_none;
      cout << "There should be no exception." << endl;
    }
  }
  //
  // Testing: move ctor
  //
  {
    mad_queue q;
    q.insert(mc(1), mc(100));
    try {
      exceptions_on = exceptions_all;
      mad_queue r = move(q);
      exceptions_on = exceptions_none;
      if(r.size() != 1) {
        cout << "Moving not performed correctly." << endl;
      } else {
        if(r.minKey() == 1 && r.minValue() == 100) {
          // OK
        } else {
          cout << "Moving not performed correctly." << endl;
        }
      }
    } catch(const bad_alloc &e) {
      exceptions_on = exceptions_none;
      cout << "Allocation failed - exception possible." << endl;
    } catch(...) {
      exceptions_on = exceptions_none;
      cout << "There should be no exception." << endl;
    }
  }
  //
  // Testing: copying operator
  //
  {
    mad_queue q;
    q.insert(mc(1), mc(100));
    mad_queue r;
    r.insert(mc(2), mc(200));
    r.insert(mc(3), mc(300));
    try {
      exceptions_on = exceptions_all;
      r = q;
      exceptions_on = exceptions_none;
      if(r.size() != 1) {
        cout << "Copying not performed correctly." << endl;
      } else {
        if(r.minKey() == 1 && r.minValue() == 100) {
          // OK
        } else {
          cout << "Copying not performed correctly." << endl;
        }
      }
    } catch(...) {
      exceptions_on = exceptions_none;
      cout << "There should be no exception." << endl;
      if(r.size() != 2) {
        cout << "No rollback after exception." << endl;
      } else {
        if(r.minKey() == 2 && r.minValue() == 200 && r.maxKey() == 3 && r.maxValue() == 300) {
          // OK
        } else {
          cout << "Rollback not performed correctly." << endl;
        }
      }
    }
  }
  //
  // Testing: move operator
  //
  {
    mad_queue q;
    q.insert(mc(1), mc(100));
    mad_queue r;
    r.insert(mc(2), mc(200));
    r.insert(mc(3), mc(300));
    try {
      exceptions_on = exceptions_all;
      r = move(q);
      exceptions_on = exceptions_none;
      if(r.size() != 1) {
        cout << "Moving not performed correctly." << endl;
      } else {
        if(r.minKey() == 1 && r.minValue() == 100) {
          // OK
        } else {
          cout << "Moving not performed correctly." << endl;
        }
      }
    } catch(...) {
      exceptions_on = exceptions_none;
      cout << "There should be no exception." << endl;
      if(r.size() != 2) {
        cout << "No rollback after exception." << endl;
      } else {
        if(r.minKey() == 2 && r.minValue() == 200 && r.maxKey() == 3 && r.maxValue() == 300) {
          // OK
        } else {
          cout << "Rollback not performed correctly." << endl;
        }
      }
    }
  }
  //
  // Testing: deleteMin()
  //
  {
    mad_queue q;
    q.insert(mc(1), mc(100));
    try {
      exceptions_on = 1<<7;
      q.deleteMin();
      exceptions_on = 0;
      cout << "Exception lost." << endl;
    } catch(my_exception &e) {
      // caught
      exceptions_on = 0;
      if(q.size() == 1) {
        //cout << "Exception safety is OK." << endl;
      } else {
        cout << "Missing rollback after exception." << endl;
      }
    }
  }

  //
  // Testing: deleteMax()
  //
  {
    mad_queue q;
    q.insert(mc(1), mc(100));
    try {
      exceptions_on = 1<<7;
      q.deleteMax();
      exceptions_on = 0;
      cout << "Exception lost." << endl;
    } catch(my_exception &e) {
      // caught
      exceptions_on = 0;
      if(q.size() == 1) {
        //cout << "Exception safety is OK." << endl;
      } else {
        cout << "Missing rollback after exception." << endl;
      }
    }
  }
  //
  // Testing: operator <
  //
  {
    mad_queue q;
    q.insert(mc(1), mc(100));
    q.insert(mc(3), mc(200));
    mad_queue r;
    r.insert(mc(1), mc(100));
    r.insert(mc(3), mc(300));
    if(q < r) {
      // OK
    } else {
      cout << "Comparing is not done correctly!" << endl;
    }
    exceptions_on = exceptions_all;
    try {
      if(q < r) {
        cout << "Lost some exception!" << endl;
      } else {
        cout << "Lost some exception and result is incorrect!" << endl;
      }
      exceptions_on = exceptions_none;
    } catch(...) {
      exceptions_on = exceptions_none;
      // OK - operator < get const objects so no modification possible
    }
  }

  cout << "Done" << endl;
  return 0;
}
