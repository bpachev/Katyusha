#include "KatyushaEngine.h"

KatyushaNet network;
string weightsfile = "/home/benjamin/Katyusha/stockfish-7-linux/src/katyusha_weights.npz";
int pos_rep_arr[Analyze::NB_FEATURES];
bool is_active = true;

bool KatyushaEngine::engine_active()
{
  return is_active;
}

void KatyushaEngine::activate() {is_active = true;}
void KatyushaEngine::deactivate() {is_active = false;}

void KatyushaEngine::setWeightsfile(string newname)
{
  weightsfile = newname;
  network.load(weightsfile);
}

string KatyushaEngine::getWeightsfile()
{
  return weightsfile;
}

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
  int mul = (int)(PawnValueEg*50);
  int increment  = (int)(mul * raw_eval);
  return (v + increment);
}
