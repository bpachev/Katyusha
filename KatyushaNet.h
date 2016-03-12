#ifndef KatyushaNet_h
#define KatyushaNet_h
#include <iostream>
#include <fstream>
#include <string>
#include "reluLayer.h"

//SIDE TO MOVE, 4 CASTLE RIGHTS, 2*5 material counts
#define GLOBAL_FEATURES 15
//computed
#define PIECE_FEATURES 164  
//two board maps of 64 squares
#define SQUARE_FEATURES 128
//8 files, 2 colors, num of pawns of given color on given file
#define PAWN_FEATURES 16
#define TOTAL_FEATURES (GLOBAL_FEATURES + PIECE_FEATURES + SQUARE_FEATURES + PAWN_FEATURES)

class KatyushaNet {
public:
//  KatyushaNet();
  float evaluate(int * pos_features);  
  void load_params(string fname);
  
  reluLayer layer1;
  reluLayer global;
  reluLayer piece;
  reluLayer square;
  reluLayer pawn;
};

#endif

