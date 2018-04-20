mkdir -p ../logm/$DATASET/
mkdir -p result/$GRAPH/$th
echo "no sd";
./Search --prefix=../data_$DATASET --all-data --prefix-graph=$DATA/$GRAPH --shared-input --typePos $TYPEPOS --typeNeg $TYPENEG --n=113 --npi=$NUMPOS --nni=$NUMNEG --ns=0 --method freq 2 30 $th --strategy ofg-para 15 $th diff $al sd-no net dces log:../logm/$DATASET/score-$th-diff-$al-nosd.txt stat:../logm/$DATASET/stat-$th-diff-$al-nosd.txt --out=result/$GRAPH/$th/diff-$al-nosd > ../logm/$DATASET/log-$th-diff-$al-nosd.txt
echo "no dces";
./Search --prefix=../data_$DATASET --all-data --prefix-graph=$DATA/$GRAPH --shared-input --typePos $TYPEPOS --typeNeg $TYPENEG --n=113 --npi=$NUMPOS --nni=$NUMNEG --ns=0 --method freq 2 30 $th --strategy ofg-para 15 $th diff $al sd net dces-no log:../logm/$DATASET/score-$th-diff-$al-nodce.txt stat:../logm/$DATASET/stat-$th-diff-$al-nodce.txt --out=result/$GRAPH/$th/diff-$al-nodce > ../logm/$DATASET/log-$th-diff-$al-nodce.txt
echo "no sd dces"
./Search --prefix=../data_$DATASET --all-data --prefix-graph=$DATA/$GRAPH --shared-input --typePos $TYPEPOS --typeNeg $TYPENEG --n=113 --npi=$NUMPOS --nni=$NUMNEG --ns=0 --method freq 2 30 $th --strategy ofg-para 15 $th diff $al sd-no net dces-no log:../logm/$DATASET/score-$th-diff-$al-nosd-nodce.txt stat:../logm/$DATASET/stat-$th-diff-$al-nosd-nodce.txt --out=result/$GRAPH/$th/diff-$al-nosd-nodce > ../logm/$DATASET/log-$th-diff-$al-nosd-nodce.txt

