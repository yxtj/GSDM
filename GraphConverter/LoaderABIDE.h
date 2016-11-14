
#pragma once
#include "TCLoader.h"
#include <tuple>

class LoaderABIDE
	:public TCLoader

{

	static const int POS_ID, POS_DX;
	static const std::string filePrefix;
	static const std::vector<std::string> header;
	static const std::vector<int> POS_QC; // ADI_R_RSRCH_RELIABLE, ADOS_RSRCH_RELIABLE
//	static const int ID_LENGTH_FILE = 7;
//	static const char PADDING = '0';
public:
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(const std::string& fn, const int nSubject = -1);

//	virtual std::vector<SubjectInfo> pruneSubjectsViaScanFile(std::vector<SubjectInfo>& vldlist, const std::string& root);


	virtual std::string getFilePath(const SubjectInfo& sub);

	virtual tc_t loadTimeCourse(const std::string& fn);
private:
	bool checkHeader(const std::string& line);
	std::tuple<bool, std::string, int> parsePhenotypeLine(const std::string& line);
};


