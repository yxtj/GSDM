#pragma once
#include "LoaderBaseCPAC.h"
class LoaderABIDE2 :
	public LoaderBaseCPAC
{
	static const int POS_ID, POS_DX;
	static const std::vector<std::string> header;
	static const std::vector<int> POS_QC; //ADI_R_RSRCH_RELIABLE, ADOS_RSRCH_RELIABLE
public:
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(
		const std::string& fn, const std::string& qcMethod, const int nSubject = -1);

private:
	bool checkHeader(const std::string &line);
	std::tuple<bool, std::string, int> parsePhenotypeLine(
		const std::string& line, const std::string& qcMethod);
};

