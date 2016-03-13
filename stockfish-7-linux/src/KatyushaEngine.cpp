#include "KatyushaEngine.h"

KatyushaNet network;
string weightsfile = "adagrad_weights2.npz";
int pos_rep_arr[Analyze::NB_FEATURES];


void KatyushaEngine::init()
{
  network.load(weightsfile);
}

Value KatyushaEngine::evaluate(const Position& pos)
{
  Analyze::Katyusha_pos_rep(pos, pos_rep_arr);
  return to_stockfish_value(network.evaluate(pos_rep_arr));
}

Value KatyushaEngine::to_stockfish_value(float raw_eval)
{
  //TODO: come up with a better transformation
  Value v = VALUE_DRAW;
  return (v + (int)(raw_eval*50));
}
