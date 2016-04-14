import chess
from chess import uci

from sys import argv, exit

usage = "usage engine1 engine2"
if len(argv) < 3:
  print usage
  exit(1)

engine1, engine2 = None, None
try:
    engine1, engine2 = uci.popen_engine(argv[1]), uci.popen_engine(argv[2])
    engine1.uci()
    engine2.uci()
    engine1.info_handlers.append(uci.InfoHandler())
    engine2.info_handlers.append(uci.InfoHandler())
except e:
    print e
    print "error opening engine binaries"
    print usage
    exit(1)

def play_game(engine1, engine2):
    board = chess.Board()
    info1 = uci.InfoHandler()
    info2 = uci.InfoHandler()
    engine1.info_handlers.append(info1)
    engine2.info_handlers.append(info2)
    while not board.is_game_over():
        engine1.position(board)
        bestInfo = engine1.go(movetime = 1000)
        print bestInfo
        board.push(bestInfo.bestmove)
        if board.is_game_over(): break

        engine2.position(board)
        bestInfo = engine2.go(movetime = 1000)
        print bestInfo
        board.push(bestInfo.bestmove)
        print board
        with info1:
            cp = info1.info["score"][1].cp
            mate = info1.info["score"][1].mate
            print cp, mate
        with info2:
            cp, mate = info2.info["score"][1].cp, info2.info["score"][1].mate
            print cp, mate


play_game(engine1, engine2)
#bestInfo = engine1.go(wtime=1000, btime = 1000)
#bestInfo.
