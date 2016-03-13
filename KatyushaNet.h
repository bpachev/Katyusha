#ifndef KatyushaNet_h
#define KatyushaNet_h
#include <iostream>
#include <fstream>
#include <string>
#include "cnpy.h"
#include <map>
#include <vector>
#include "reluLayer.h"
#include "tanhLayer.h"

//SIDE TO MOVE, 4 CASTLE RIGHTS, 2*5 material counts
#define GLOBAL_FEATURES 15
//computed
#define PIECE_FEATURES 164
//two board maps of 64 squares
#define SQUARE_FEATURES 128
//8 files, 2 colors, num of pawns of given color on given file
#define PAWN_FEATURES 16
#define TOTAL_FEATURES (GLOBAL_FEATURES + PIECE_FEATURES + SQUARE_FEATURES + PAWN_FEATURES)

using namespace std;


struct firstLayer {
  string name;
  reluLayer * layer;
  int inputs;
};


class KatyushaNet {
public:

  vector<firstLayer> initial_layers;
  reluLayer layer1;
  tanhLayer out;

  KatyushaNet()
  {
    firstLayer global;
    global.name = "global";
    global.inputs = GLOBAL_FEATURES;
    initial_layers.push_back(global);

    firstLayer piece;
    piece.name = "piece";
    piece.inputs = PIECE_FEATURES;
    initial_layers.push_back(piece);

    firstLayer square;
    square.name = "square";
    square.inputs = SQUARE_FEATURES;
    initial_layers.push_back(square);

    firstLayer pawn;
    pawn.name = "pawn";
    pawn.inputs = PAWN_FEATURES;
    initial_layers.push_back(pawn);

    for (size_t i = 0; i < initial_layers.size(); i++)
    {
      initial_layers[i].layer = new reluLayer();
    }
  }
  //load the weights of the model from an npz archive
  void load(string archive_name);
  float evaluate(int * pos_features);
  void load_layer(string layer_name, Layer* layer, cnpy::npz_t& archive);

  ~KatyushaNet();
};

#endif
