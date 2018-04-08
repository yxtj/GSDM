if [ $# -lt 5 ] ; then
	echo "Usage: <motif root> <graph folder> <type pos> <type neg> <output folder> [n motifs=-1]"
	echo "	<motif root>: next layer is threshold, then explore all in next layer. like <motif root>/0.3/25-0.5-0.5"
	echo "    The input structure should be <motif root>/<threshold>/<folder for parameters>/res-0.txt"
	echo "	<graph type>: USE \" and space to express multiple types. like \"0 1\""
	exit
fi

MOTIF_ROOT=$1
GRAPH_FOLDER=$2
GRAPH_TYPE_POS=$3
GRAPH_TYPE_NEG=$4
OUT_FOLDER=$5
NUM_MOTIF=0
if [ $# -ge 6 ]; then
	NUM_MOTIF=$6
fi

for th in $(ls $MOTIF_ROOT); do
	for fn in $(ls $MOTIF_ROOT/$th); do
		echo $th-$fn
		if [ 0 -eq $(cat $MOTIF_ROOT/$th/$fn/res-*.txt| wc -l) ]; then
			echo "escape the empty motif set"
			continue
		fi
#		MLIST="$MLIST $MOTIF_ROOT/$th/$fn"
#		OLIST="$OLIST $OUT_FOLDER/tst-$th-$fn.txt"
		 ./MotifAnalyzer --graphPath $GRAPH_FOLDER --typePos $GRAPH_TYPE_POS --typeNeg $GRAPH_TYPE_NEG \
--nMotif $NUM_MOTIF --motifPath $MOTIF_ROOT/$th/$fn --outputPath $OUT_FOLDER/$th/$fn
	done
done
