#include "KatyushaNet.h"

int main()
{
  string fname = "bootstrapped_weights.npz";
  KatyushaNet k;
  k.load(fname);
  int tvec[TOTAL_FEATURES];
  for (int i = 0; i < TOTAL_FEATURES; i++) tvec[i] = 0;
  cout << k.evaluate(tvec) << endl;
}
