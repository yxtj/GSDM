- GraphConverter

Convert the time-course data of different dataset into graphs.

Support multiple datasets, multiple time-course cutting methods, multiple correlation methods.
These methods and parameters are provided via command arguments.
This progrem is able to load either time-course data or processed correlation data.
The output can be correlation and graph.


- Usage of this Graph Converter:

For detailed command line usage, please run this program without any parameter or with parameter "--help".

If --tcPath is given, the time-course is the input source and --corrPath is an output (if provided).
If --tcPath is not given, the correlation is the input source.

Example command parameters:

```
%start from time courses, generate correlation and graph, using sliding windows method and pearson correlation:
--dataset=ADHD -n 0 --tcPath=~/Data/ADHD200/ADHD200_AAL_TCs_filtfix/KKI --corrPath=../data_adhd/corr --graphPath=../data_adhd/graph-0.7 --cut-method slide 30 12 --corr-method pearson --graph-method ge 0.7

%start from correlation, generate graph, using pearson correlation, edges are told by absolute correlation values greater than 0.8:
--dataset=ADHD -n 0 --corrPath=../data_adhd/corr --graphPath=../data_adhd/graph-0.7 --cut-method slide 30 12 --corr-method pearson --graph-method outside -0.8 0.8

--dataset=ADHD -n 0 --tcPath=E:\Data\ADHD200\ADHD200_AAL_TCs_filtfix\NYU --corrPath=../data_adhd/corr2 --cut-method slide 30 12 --corr-method pearson --graph-method outside -0.8 0.8

%ABIDE
--dataset=ABIDE -n 5 --tcPath=E:\Data\ABIDE --corrPath=../data_abide/corr --cut-method slide 30 12 --corr-method pearson --graph-method outside -0.8 0.8
```

- Implementation:

  - Structure:

  load input data -> (process into correlation) -> output the correlation -> process into graph -> output generated graph

  While loading the input correlation data (called in IOfunctions.cpp, defined in class TCLoader):
  
    1. read a description file for subject id, subject type
    
    2. check data folders for all the scan files for each subject
    
    3. construct the path of each scan file
    
    4. parse the file and load the data

  - Extension:

    - Data Source:

    Derive a new class from class TCLoader.
    And implement its functions: loadValidList, getFilePath, loadTimeCourse and getAllSubjects if necessary.
    
    - Cutter method:
  
	  1. Modify class TCCutterParm to add necessary method parser.

	  2. Modify class TCCutter to support this new cutter method.

  
    - Correlation method:
  
    Write new related calculating function in class TC2Corr.
    Add the initialization for symmetric and pFunCorr in the constructor of TC2Corr.

