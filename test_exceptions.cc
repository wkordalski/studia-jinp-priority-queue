#include "priorityqueue.hh"

#include <exception>
#include <iostream>

using namespace std;

#define mad(n) do { if(delay_exceptions > 0) { delay_exceptions--; } else { if(exceptions_on & (1<<n)) throw my_exception(n); } } while(0)
int exceptions_on = 0;
int delay_exceptions = 0;

const int exceptions_all = 0xfffffff;


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
};

using mad_queue = PriorityQueue<mad_class, mad_class>;
using mc = mad_class;

int main() {
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

  cout << "Done" << endl;
  return 0;
}
