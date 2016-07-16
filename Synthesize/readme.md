- Synthesize

Synthesize some graph data.

- Usage of this program:

For detailed command line usage, please run this program with parameter "--help".


Synthesize garphes with given number of nodes following powerlaw distribution with given alpha.

The motif size is random generated within a given range.

Example command parameters:

```
% 50 nodes of each graph, alpha parameter of the powerlaw distribution is 4,
% 10 positive individuals and 10 negative individuals, 10 positive motifs and 10 negatives,
% for each generated motifs: minimum size is 3, maximum size is 5, the mean probabilitf of all the motifs is 0.5
% the seed for all the random numbers used in this program is 123456
--prefix=../data/ --n=50 --alpha=4 --npi=10 --nni=10 --ns=10 --npm=10 --nnm=10 --smmin=3 --smmax=5 --pmotif=0.5 --seed=123456
```

