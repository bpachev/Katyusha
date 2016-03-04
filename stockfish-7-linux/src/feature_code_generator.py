

pieces = ["QUEEN", "ROOK", "BISHOP", "KNIGHT", "PAWN"]

mobilePieces = pieces[:3]
pieceNums = [1, 2, 2, 2, 8]

colors = ["WHITE", "BLACK"]

nfeatures_list = []
for color in colors:
    for piece, num in zip(pieces, pieceNums):
        for i in xrange(1, num+1):
            feature =  color[0] + piece[0] + str(i) + "_EXISTS"
            #existence
            print "features[" + feature + "] = (<" + piece + ">pos.count("+color+") >= "+str(i)+") ? 1 : 0;"
            #square to rank is /8, square to file is % 8  

            nfeatures_list.append(feature)
        print
    print
