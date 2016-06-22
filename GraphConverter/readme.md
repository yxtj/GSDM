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
  
    Write related constructor and init function for class TCCutter.

  
    - Correlation method:
  
    Write new related calculating function in class TC2Corr.
    Add the initialization for symmetric and pFunCorr in the constructor of TC2Corr.

