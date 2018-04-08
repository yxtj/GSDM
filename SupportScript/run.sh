NUM_POSITIVE=10
NUM_NEGATIVE=10
MOTIF_SIZE_MIN=2
MOTIF_SIZE_MAX=6
MOTIF_PROB=0.4
BLACKLIST=$(cat blacklist)
./Search --prefix=../data_adhd --prefix-graph=graph-0.8 --n=115 --npi=$NUM_POSITIVE --nni=$NUM_NEGATIVE --ns=0 --method freq $MOTIF_SIZE_MIN $MOTIF_SIZE_MAX $MOTIF_PROB --strategy paraPmN 0.6 0.3 0.5 --blacklist $BLACKLIST

