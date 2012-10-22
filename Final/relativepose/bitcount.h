// -*- c++ -*-
#include <climits>

template <typename T>
int bitcount (T v){
  v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
  v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
  v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
  int c = (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; // count
  return c;
}
