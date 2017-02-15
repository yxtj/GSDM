TYPE="1 2" # for abide, abide2
TYPE="0 1 2 3 4" # for adni
GRAPH="p-s20-10/graph-0.5"

# establish shuffled list for each type (first use)
n=0
arr=()
for t in $TYPE; do
    echo $t;
    temp=${GRAPH//\//\\\/};
    ls data-all/$GRAPH/$t-*-0.txt | sed "s/data-all\/$temp\///" | sed 's/-0.txt/-*.txt/' | shuf > $t;
    temp=$(cat $t|wc -l);
    arr[$t]=$temp;
    n=$((n+temp));
done

n=$((n*3/10/2));


# for multiple-types group:
TGROUP="1 2 3 4"
name="";
list="";
gn=0;
for t in $TGROUP; do
    name+=$t;
    list+=" $t";
    gn=$((gn+${arr[$t]}));
done
cat $list | shuf > $name;
arr[$name]=$gn;


# count each type (second use)

n=0
arr=()
for t in $TYPE; do
    echo $t;
    temp=$(cat $t|wc -l);
    arr[$t]=$temp;
    n=$((n+temp));
done
echo ${arr[*]}

n=$((n*3/10/2));


# output

mkdir -p data/$GRAPH ;
mkdir -p data-test/$GRAPH ;
for t in $TYPE; do
    num=$(( ${arr[$t]} - n ));
    echo $t: $n-$num;
    for line in $(cat $t|head -$n); do
        cp data-all/$GRAPH/$line data-test/$GRAPH;
    done
    for line in $(cat $t|tail -$num); do
        cp data-all/$GRAPH/$line data/$GRAPH;
    done
done

