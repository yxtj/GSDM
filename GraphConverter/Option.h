#pragma once
#include "TCCutterParam.h"
#include <string>
#include <vector>
#include <functional>
#include <boost/program_options.hpp>

class Option
{
	boost::program_options::options_description desc;
	std::vector<std::function<bool()>> paramParser;
public:
	enum class CutType { NGRAPH, NSCAN, NONE };
	enum class FileType { TC, CORR, GRAPH, NONE };

	std::string dataset; // specific which dataset is going to be use
	int nSkip; // skip some of the first items(subject/corr), to efficiently restart after failure
	int nSubject; // number of subject need to be loaded from the dataset

	// path for input/output data folder:
	std::string phenoPath, tcPath, corrPath, graphPath; // time course data, correlation data path, graph data 

	std::string tcQualityControl; // how to filter the subjects with the Quality Control information (none, any, all)
	TCCutterParam cutp;

	std::string corrMethod; // method of calculating correlation
	std::vector<std::string> graphParam; // the correlation predicate for determine connectivity
	int comGraphLevel; // the compression level for graph outputing

public:
	Option();
	boost::program_options::options_description& getDesc();
	void addParser(std::function<bool()>& fun);

	bool parseInput(int argc, char *argv[]);
	std::string getCutMethod() const {
		return cutp.method;
	}
	std::pair<FileType, std::string> getInputFolder() const;
	bool isOutputFolder(FileType ft) const;
private:
	std::string& sortUpPath(std::string& path);
	bool checkIOLogic();
	bool initCutLogic();
	bool sortUpTCQC();
};

