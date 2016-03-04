/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2016 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <string>

#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "timeman.h"
#include "uci.h"
#include "analyze.h"

using namespace std;

namespace {

const char* StartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

Search::StateStackPtr SetupStates;

double to_cp(Value v) { return double(v) / PawnValueEg; }

double centipawn_evaluate(Position& pos)
{
  Value v = Eval::evaluate<true>(pos);
  v = pos.side_to_move() == WHITE ? v : -v; // White's point of view
  return to_cp(v); //convert to centipawns
}

double evaluate_fen(string fen)
{
  Position pos(fen, false, Threads.main());
  return centipawn_evaluate(pos);
}

string analyze_game(string& moves)
{
   SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);

    istringstream is(moves);

    std::stringstream ss;
    string token;
    Move m;
    Position pos(StartFEN, false, Threads.main());
    ss << centipawn_evaluate(pos);

    // Parse move list
    while (is >> token && (m = UCI::to_move(pos, token)) != MOVE_NONE)
    {
        SetupStates->push(StateInfo());
        pos.do_move(m, SetupStates->top(), pos.gives_check(m, CheckInfo(pos)));
        ss << "," << centipawn_evaluate(pos);
    }
    return ss.str();
}

}//namespace


void output_game(ostream& out, string& mov_str)
{
 out << analyze_game(mov_str) << "\n";
}

/*
 Reads in a file of games, and writes the static evaluation move-by-move to the given outputfile.
*/
void Analyze::game_list(istringstream& is)
{
  string infile;
  string ofile;
  if ( !(is >> infile) || !(is >> ofile)) return;
  fstream f;
  f.open(infile);
  string line;
  ofstream out;
  out.open(ofile);

  string mov_str;
  while (getline(f, line))
  {
    if (line.length())
    {
      mov_str += line;
      mov_str += " "; //so there will be a space between moves
    }
    else
    {
      output_game(out, mov_str);
      mov_str = "";
    }
  }

  if (mov_str.length())
  {
    output_game(out, mov_str);
  }

  f.close();
  out.close();
}

/*

*/
void Analyze::pos_list(istringstream& is)
{
  string infile;
  string ofile;
  if ( !(is >> infile) || !(is >> ofile)) return;
  fstream f;
  f.open(infile);
  string line;
  ofstream out;
  out.open(ofile);
  while(getline(f, line))
  {
    if (line.length()) {
      double e = evaluate_fen(line);
      out << line << '\n' <<  e << '\n';
    }
  }
  f.close();
  out.close();
}


//extract a feature representation from the position pos
//The features are as follows
// Side to move:
// Castling WK, WQ, BK, BQ

#define NFEATURES 400

enum feature {
   TEMPO_FEATURE,
   WCASTLE_OO,
   WCASTLE_OOO,
   BCASTLE_OO,
   BCASTLE_OOO,
   NUM_WQ,
   NUM_WR
   NUM_WB,
   NUM_WN,
   NUM_WP,
   NUM_BQ,
   NUM_BR
   NUM_BB,
   NUM_BN,
   NUM_BP,

   WQ_EXISTS,
   BQ_EXISTS,
   WR_EXISTS,
   BR_EXISTS,
   WB_EXISTS,
   BB_EXISTS,

};

//utility function to convert Stockfish's internal feature representation into a feature vector I can use to train Katusha.
// NOTE: this feature representation is based on that used by Giraffe, with some tweaks.
void Analyze::Katyusha_pos_rep(Position& pos)
{
  int i;
  int * features = (int*)malloc(sizeof(int) * NFEATURES);
  std::memeset(features, 0, sizeof(int)*NFEATURES);

  //tempo
  if (pos.side_to_move() == WHITE)
  {
    features[TEMPO_FEATURE] = 1;
  }
  else features[TEMPO_FEATURE] = 0;

  //castling rights
  features[WCASTLE_OO] = pos.can_castle(WHITE_OO);
  features[WCASTLE_OOO] = pos.can_castle(WHITE_OOO);
  features[BCASTLE_OO] = pos.can_castle(BLACK_OO);
  features[BCASTLE_OOO] = pos.can_castle(BLACK_OOO);

  //material configuration
  features[NUM_WQ] = <QUEEN>pos.count(WHITE);
  features[NUM_WR] = <ROOK>pos.count(WHITE);
  features[NUM_WB] = <BISHOP>pos.count(WHITE);
  features[NUM_WN] = <KNIGHT>pos.count(WHITE);
  features[NUM_WP] = <PAWN>pos.count(WHITE);
  features[NUM_BQ] = <QUEEN>pos.count(BLACK);
  features[NUM_BR] = <ROOK>pos.count(BLACK);
  features[NUM_BB] = <BISHOP>pos.count(BLACK);
  features[NUM_BN] = <KNIGHT>pos.count(BLACK);
  features[NUM_BP] = <PAWN>pos.count(BLACK);

  //piece coordinates and existence information
  features[WQ_EXISTS] = (features[NUM_WQ] > 0 ) ? 1 : 0;
  features[BQ_EXISTS] = (features[NUM_BQ] > 0 ) ? 1 : 0;

// ex of how to get the square list for a piece <PAWN>pos.squares(WHITE);
//  features[]

  //sliding piece mobility

  //attack and defend maps


  //pawn information

  //something about king safety


}
