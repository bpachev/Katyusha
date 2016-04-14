import numpy as np
import chess
from chess import uci
from sys import argv, exit

usage = "usage katyushabinaryfile"
if len(argv) < 2:
    print usage
    exit(1)

katyusha = None
try:
    katyusha = uci.popen_engine(argv[1])
    katyusha.uci()
except e:
    print "error initializing engine"
    print e
    exit(1)

#katyusha.info_handlers.append(uci.InfoHandler())

def extract_evals(katyusha, startFen, num_moves):
    """
    Have Katyusha play num_moves moves against itself.
    Get katyusha's evaluation of the board between moves.
    (Will probably need to fix signs on evals, to fix relative to absolute)
    """
    res = np.zeros(num_moves)
#    board = chess.Board(fen = startFen)
    board = chess.Board()
    if board.is_game_over():
        print "Got a starting position that is already terminal."
        return res


    #katyusha_info  = katyusha.info_handlers[-1]
    katyusha_info = uci.InfoHandler()
    katyusha.info_handlers.append(katyusha_info)
    for i in xrange(num_moves):
        katyusha.position(board)
        kinfo = katyusha.go(movetime = 1000)
        with katyusha_info:
            if 1 in katyusha_info.info["score"]:
                cp = katyusha_info.info["score"][1].cp
                mate = katyusha_info.info["score"][1].mate
                if cp is None:
                    print "Checkmate (probably) found"
                    res[i:] = res[max(i-1,0)]
                    break
                else:
                    if board.turn == chess.BLACK:
                        cp *= -1
                    res[i] = cp
            else:
                print "Bad Info "
                res[i:] = res[max(i-1,0)]
                break

        board.push(kinfo.bestmove)
        if board.is_game_over():
            #If the game is cut short, then all the last evaluations should be the same as the last real one.
            #This ensures minimal noisy temporal differences
            res[i+1:] = res[i]
            break

    return res

print extract_evals(katyusha, None, 100)
