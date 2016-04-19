#ifndef KatyushaEngine_H
#define KatyushaEngine_H

#include "analyze.h"
#include "KatyushaNet.h"
#include "types.h"

namespace KatyushaEngine {
   void init();
   Value evaluate(const Position& pos);
   Value to_stockfish_value(float raw_eval);
   bool engine_active();
   void activate();
   void deactivate();
   void setWeightsfile(string newname);
   string getWeightsfile();
}

#endif
