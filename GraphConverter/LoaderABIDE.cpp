#include "stdafx.h"
#include "LoaderABIDE.h"

using namespace std;

const string LoaderABIDE::filePrefix = "ROISignals_FunimgARCWF/ROISignals_00";

const std::vector<std::string> LoaderABIDE::header = {
"SITE_ID","SUB_ID","DX_GROUP","DSM_IV_TR","AGE_AT_SCAN","SEX","HANDEDNESS_CATEGORY","HANDEDNESS_SCORES","FIQ","VIQ","PIQ","FIQ_TEST_TYPE",
"VIQ_TEST_TYPE","PIQ_TEST_TYPE","ADI_R_SOCIAL_TOTAL_A","ADI_R_VERBAL_TOTAL_BV","ADI_RRB_TOTAL_C","ADI_R_ONSET_TOTAL_D","ADI_R_RSRCH_RELIABLE",
"ADOS_MODULE","ADOS_TOTAL","ADOS_COMM","ADOS_SOCIAL","ADOS_STEREO_BEHAV","ADOS_RSRCH_RELIABLE","ADOS_GOTHAM_SOCAFFECT","ADOS_GOTHAM_RRB",
"ADOS_GOTHAM_TOTAL","ADOS_GOTHAM_SEVERITY","SRS_VERSION","SRS_RAW_TOTAL","SRS_AWARENESS","SRS_COGNITION","SRS_COMMUNICATION","SRS_MOTIVATION",
"SRS_MANNERISMS","SCQ_TOTAL","AQ_TOTAL","COMORBIDITY","CURRENT_MED_STATUS","MEDICATION_NAME","OFF_STIMULANTS_AT_SCAN",
"VINELAND_RECEPTIVE_V_SCALED","VINELAND_EXPRESSIVE_V_SCALED","VINELAND_WRITTEN_V_SCALED","VINELAND_COMMUNICATION_STANDARD",
"VINELAND_PERSONAL_V_SCALED","VINELAND_DOMESTIC_V_SCALED","VINELAND_COMMUNITY_V_SCALED","VINELAND_DAILYLVNG_STANDARD",
"VINELAND_INTERPERSONAL_V_SCALED","VINELAND_PLAY_V_SCALED","VINELAND_COPING_V_SCALED","VINELAND_SOCIAL_STANDARD","VINELAND_SUM_SCORES",
"VINELAND_ABC_STANDARD","VINELAND_INFORMANT","WISC_IV_VCI","WISC_IV_PRI","WISC_IV_WMI","WISC_IV_PSI","WISC_IV_SIM_SCALED",
"WISC_IV_VOCAB_SCALED","WISC_IV_INFO_SCALED","WISC_IV_BLK_DSN_SCALED","WISC_IV_PIC_CON_SCALED","WISC_IV_MATRIX_SCALED",
"WISC_IV_DIGIT_SPAN_SCALED","WISC_IV_LET_NUM_SCALED","WISC_IV_CODING_SCALED","WISC_IV_SYM_SCALED","EYE_STATUS_AT_SCAN","AGE_AT_MPRAGE","BMI"
};


bool LoaderABIDE::checkHeader(const std::string &line) {
	int count = 0;
	for(size_t plast = 0, p = line.find(','); p != string::npos;) {
		if(line.substr(plast, p - plast) != header[count]) {
			return false;
		}
		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
	return true;
}

std::vector<SubjectInfo> LoaderABIDE::loadValidList(const std::string &fn, const int nSubject)
{
	string filename(fn);
	if(filename.find("RfMRIMaps_ABIDE_Phenotypic") == string::npos) {
		size_t pos_slash = filename.find_last_of("/\\");
		if(pos_slash == filename.length() - 1) {
			filename += "RfMRIMaps_ABIDE_Phenotypic.csv";
		} else {
			filename += "/RfMRIMaps_ABIDE_Phenotypic.csv";
		}
	}

	ifstream fin(filename);
	if(!fin) {
		cerr << "Cannot open phenotype file with given parameter: " << endl;
		throw invalid_argument("cannot open valid list with given parameter");
	}

	std::string line;
	getline(fin, line, '\r'); // This phenotype csv uses '\r' for line termination, not '\n'.
	if(!checkHeader(line)) {
		cerr << "Header line of file '" << fn << "' is not correct!" << endl;
		throw invalid_argument("file header does not match that of the specific dataset");
	}
	size_t limit = nSubject > 0 ? nSubject : numeric_limits<size_t>::max();

	vector<SubjectInfo> res;
	while(getline(fin, line, '\r'))
	{
		bool valid;
		string sid;
		int type;
		tie(valid, sid, type) = parsePhenotypeLine(line);

		if(valid) {
			res.push_back(SubjectInfo{ sid,type });
		}

		if(res.size() >= limit) {
			break;
		}
	}
	fin.close();
	return res;
}

string LoaderABIDE::getFilePath(const SubjectInfo &sub) {
	return filePrefix + sub.id + ".txt";
}

tc_t LoaderABIDE::loadTimeCourse(const std::string &fn)
{
	vector<vector<double>> res;

	string filename(fn);

	if(filename.find(".txt") == string::npos) {
		cerr << "Cannot read from this filetype, please direct to a .txt" << endl;
		throw invalid_argument("Cannot read from this filetype, please direct to a .txt");
	}
	ifstream fin(filename);
	if(!fin) {
		return res;
	}
	string line;
	// no header line
	getline(fin, line);
	int nNodes = 0;
	for(size_t plast = 0, p = line.find('\t'); p != string::npos; p = line.find('\t', plast))
	{
		++nNodes;
		plast = p + 1;
	}
	do
	{
		if(line.empty()) {
			break;
		}
		vector<double> row;
		row.reserve(nNodes);
		for(size_t plast = 0, p = line.find('\t'); p != string::npos; p = line.find('\t', plast))
		{
			double TCentry = stod(line.substr(plast, p - plast));
			row.push_back(TCentry);
			plast = p + 1;
		}
		res.push_back(move(row));
	} while(getline(fin, line));

	return res;
}


std::tuple<bool, std::string, int> LoaderABIDE::parsePhenotypeLine(const std::string &line)
{
	static const int POS_ID = 1;
	static const int POS_DX = 2;
	static const int POS_RSRCH_RELIABLE = 18;

	bool reliable = false;
	std::string id;
	int dx;// Autism==1, Control==2

	size_t plast = 0;
	size_t p = line.find(',');

	int count = 0;
	while(p != string::npos)
	{
		if(count == POS_ID) {
			id = line.substr(plast, p - plast);
		} else if(count == POS_DX) {
			dx = stoi(line.substr(plast, p - plast));
		} else if(count == POS_RSRCH_RELIABLE && line.substr(plast, p - plast).empty() == false) {
			int entry = stoi(line.substr(plast, p - plast));
			reliable = (entry == 1);
		}

		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
//	id = padID2Head(id, ID_LENGTH_FILE, PADDING);
	return make_tuple(reliable, move(id), dx);
}
