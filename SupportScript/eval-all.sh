#THRESHOLD="0.2 0.3"

if [ $# -lt 5 ] ; then
	echo "Usage: <motif root> <graph folder> <graph type pos> <graph type neg> <output folder> [group size=1] [m-group method=any]"
	echo "	<motif root>: next layer is threshold, then explore all in next layer. like <motif root>/0.3/25-0.5-0.5"
	echo "	<graph type>: USE \" and space to express multiple types. like \"0 1\""
	echo "The input structure should be <motif root>/<threshold>/<parameter folder>/res-0.txt"
	exit
fi

MOTIF_ROOT=$1
GRAPH_FOLDER=$2
GRAPH_TYPE_POS=$3
GRAPH_TYPE_NEG=$4
OUT_FOLDER=$5
GROUP_SIZE=1
if [ $# -ge 6 ] ; then
	GROUP_SIZE=$6
fi
GROUP_METHOD=any
if [ $# -ge 7 ]; then
	GROUP_METHOD=$7
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
--motifPath $MLIST --testMethodSingle freq $th --testMethodGroup $GROUP_METHOD --testGroupSize $GROUP_SIZE \
--outputFile $OLIST
#	done
done
