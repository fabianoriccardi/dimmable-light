#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

/*
 * Circular queue it can only be filled.
 * Older values are automatically overwritten.
 * By default all the value are set to zeros.
 */
template<typename T, int N>
class CircularQueue {
public:
    CircularQueue(): arr{0}, index(0), n(0) {};
    
    /**
     * Insert an element. If full, it returns the overwritten element. 
     */
    T insert(T value) {
      T ret = arr[index];
      arr[index] = value;
      index++;
      if(index == N){
        index = 0;
      }
      if(n<N){
        n++;
      }

      return ret;
    };

    void reset() {
      for(int i = 0; i < N; i++) {
        arr[i] = 0;
      }
      n = 0;
      index = 0;
    }

    /**
     * Return the number of stored elements.
     */
    int getCount() {
      return n;
    };

private:
    T arr[N];

    // Current position of lement to be inserted
    int index;

    // Number of elements currently stored
    int n;
};

#endif // END CIRCULAR_QUEUE_H
