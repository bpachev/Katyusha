import numpy as np
import chess
from chess import uci
import argparse
import json

def parse_engine_json(jsonfile):
    obj = json.load(jsonfile)
    engine = uci.popen_engine("/home/benjamin/Katyusha/"+obj["binary"])
    engine.setoption(obj["options"])
    engine.description = obj["description"]
    return engine

def play_game(engine1, engine2):
    board = chess.Board()
    info1 = uci.InfoHandler()
    info2 = uci.InfoHandler()
    engine1.info_handlers.append(info1)
    engine2.info_handlers.append(info2)
    while not board.is_game_over():
        engine1.position(board)
        bestInfo = engine1.go(movetime=100)
#        print bestInfo
        board.push(bestInfo.bestmove)
        if board.is_game_over(): break

        engine2.position(board)
        bestInfo = engine2.go(movetime=100)
#        print bestInfo
        board.push(bestInfo.bestmove)
#        print board
#        with info1:
#            cp = info1.info["score"][1].cp
#            mate = info1.info["score"][1].mate
#            print cp, mate
#        with info2:
#            cp, mate = info2.info["score"][1].cp, info2.info["score"][1].mate
#            print cp, mate
    if board.is_checkmate():
        #White's turn
        if board.turn:
            return 0.
        else: return 1.
    else: return .5


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'Play two chess engines against each other.')
    json_help = "A json file containing the path to the engine binary and engine options."
    parser.add_argument('engine1_json',type=argparse.FileType("r"), help= json_help)
    parser.add_argument('engine2_json', type = argparse.FileType("r"), help = json_help)
    parser.add_argument('--num_games', type=int, default=1)

    args = parser.parse_args()


    engine1 = parse_engine_json(args.engine1_json)
    engine2 = parse_engine_json(args.engine2_json)
    print "Engine 1 Description: "+engine1.description
    print "Engine 2 Description: "+engine2.description

    for i in xrange(args.num_games):
        #randomly pick a side
        if np.random.randint(2) % 2:
            colors = ["White","Black"]
            result = play_game(engine1, engine2)
        else:
            colors = ["Black", "White"]
            result = play_game(engine2, engine1)
        for i in xrange(2):
            print colors[i]+" Engine "+str(i+1)
        print "Result: " + str(result)
