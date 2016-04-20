for i in `seq 5 7`
do
  python test_katyusha.py $1.json stockfish$i.json --num_games $2 1>>$1_stockfish$i.txt
done
