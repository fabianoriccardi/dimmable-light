#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

/*
 * A minimal static circular queue.
 * It supports only insertion, and older values are automatically overwritten.
 */
template<typename T, int N>
class CircularQueue {
public:
    /**
     * Construct a new Circular Queue object filling it with zeros or 
     * using the default constructor of type T.
     */
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

    /**
     * Empty the queue.
     */
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
    int getCount() const {
      return n;
    };

private:
    T arr[N];

    // The position for the next element to be inserted
    int index;

    // Number of elements currently stored
    int n;
};

#endif // END CIRCULAR_QUEUE_H
