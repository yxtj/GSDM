# GSDM
Graph Searcher for Discriminative Motifs

## Depends:
boost, mpi

On Ubuntu/Debian, they can be installed with command:
```
sudo apt-get install libboost-program-optioms-dev libboost-filesystem-dev libopenmpi-dev
```

If you want to use a MPI implemenations other than OpenMPI, like MPICH, that will be fine.

## Usage:

1. Use *GraphConverter* to convert the raw time series into graphs
2. Use *Search* to find the discriminative motifs
3. Use *Evaluator* to evaluate the found results.

More detailed constructions can be found in the related projects themselves, and/or directly run the program with --help.