if [ $# -lt 5 ] ; then
	echo "Usage: <motif root> <graph folder> <type pos> <type neg> <output folder> [group generation=none] [m-group method=any] [n motifs=-1]"
	echo "	<motif root>: next layer is threshold, then explore all in next layer. like <motif root>/0.3/25-0.5-0.5"
	echo "    The input structure should be <motif root>/<threshold>/<folder for parameters>/res-0.txt"
	echo "	<graph type>: USE \" and space to express multiple types. like \"0 1\""
	echo "  [group generation]: none, topk <k>, comb <k>"
	exit
fi

MOTIF_ROOT=$1
GRAPH_FOLDER=$2
GRAPH_TYPE_POS=$3
GRAPH_TYPE_NEG=$4
OUT_FOLDER=$5
GROUP_SIZE=1
GROUP_GENERATE="none"
if [ $# -ge 6 ] ; then
	GROUP_GENERATE=$6
fi
GROUP_METHOD="any"
if [ $# -ge 7 ]; then
	GROUP_METHOD=$7
fi
NUM_MOTIF=-1
if [ $# -ge 8 ]; then
	NUM_MOTIF=$8
fi

for th in $(ls $MOTIF_ROOT); do
#for th in $THRESHOLD ;  do
	echo "Threshold: $th"
	MLIST=""
	OLIST=""
	for fn in $(ls $MOTIF_ROOT/$th); do
		echo $th-$fn
		if [ 0 -eq $(cat $MOTIF_ROOT/$th/$fn/res-*.txt| wc -l) ]; then
			echo "escape the empty motif set"
			continue
		fi
		MLIST="$MLIST $MOTIF_ROOT/$th/$fn"
		OLIST="$OLIST $OUT_FOLDER/tst-$th-$fn.txt"
	done
		 ./Evaluator --graphPath $GRAPH_FOLDER --graphTypePos $GRAPH_TYPE_POS --graphTypeNeg $GRAPH_TYPE_NEG \
--nMotif $NUM_MOTIF --testMethodSingle freq $th --testMethodGroup $GROUP_METHOD --testGenerateGroup $GROUP_GENERATE \
--motifPath $MLIST --outputFile $OLIST
#	done
done
