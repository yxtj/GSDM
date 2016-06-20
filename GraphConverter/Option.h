#pragma once
#include <string>

class Option
{
public:
	enum class CutType { NGRAPH, NSCAN, NONE };
	enum class FileType { TC, CORR, GRAPH, NONE };

	std::string dataset; // specific which dataset is going to be use
	// path for input/output data folder:
	std::string tcPath, corrPath, graphPath; // time course data, correlation data path, graph data path

	CutType cutType;
	int nGraph; // evenly divides the data into given number of graphs
	int nScan; // number of scans for each graph, no fixed number of graphs

	std::string corrMethod; // method of calculating correlation
	double conThrshd; // the correlation threshold for determine connectivity

public:
	bool parseInput(int argc, char *argv[]);
	std::pair<CutType, int> getCutType() const;
	std::pair<FileType, std::string> getInputFolder() const;
	std::pair<bool, std::string> getOutputFolder(FileType ft) const;
private:
	std::string& sortUpPath(std::string& path);
	bool checkIOLogic();
	bool checkCutLogic();
};

