#include "stdafx.h"
#include "LoaderABIDE.h"
#include "CheckerFactory.h"

using namespace std;


const int LoaderABIDE::POS_ID = 6;
const int LoaderABIDE::POS_DX = 7;

//const std::string LoaderABIDE::filePrefix = "/Outputs/cpac/nofilt_global/";

const std::vector<std::string> LoaderABIDE::header = {
	"","Unnamed: 0","SUB_ID","X","subject","SITE_ID","FILE_ID","DX_GROUP","DSM_IV_TR","AGE_AT_SCAN",
	"SEX","HANDEDNESS_CATEGORY","HANDEDNESS_SCORES","FIQ","VIQ","PIQ","FIQ_TEST_TYPE","VIQ_TEST_TYPE","PIQ_TEST_TYPE","ADI_R_SOCIAL_TOTAL_A",
	"ADI_R_VERBAL_TOTAL_BV","ADI_RRB_TOTAL_C","ADI_R_ONSET_TOTAL_D","ADI_R_RSRCH_RELIABLE","ADOS_MODULE","ADOS_TOTAL","ADOS_COMM","ADOS_SOCIAL","ADOS_STEREO_BEHAV","ADOS_RSRCH_RELIABLE",
	"ADOS_GOTHAM_SOCAFFECT","ADOS_GOTHAM_RRB","ADOS_GOTHAM_TOTAL","ADOS_GOTHAM_SEVERITY","SRS_VERSION","SRS_RAW_TOTAL","SRS_AWARENESS","SRS_COGNITION","SRS_COMMUNICATION","SRS_MOTIVATION",
	"SRS_MANNERISMS","SCQ_TOTAL","AQ_TOTAL","COMORBIDITY","CURRENT_MED_STATUS","MEDICATION_NAME","OFF_STIMULANTS_AT_SCAN","VINELAND_RECEPTIVE_V_SCALED","VINELAND_EXPRESSIVE_V_SCALED","VINELAND_WRITTEN_V_SCALED",
	"VINELAND_COMMUNICATION_STANDARD","VINELAND_PERSONAL_V_SCALED","VINELAND_DOMESTIC_V_SCALED","VINELAND_COMMUNITY_V_SCALED","VINELAND_DAILYLVNG_STANDARD","VINELAND_INTERPERSONAL_V_SCALED","VINELAND_PLAY_V_SCALED","VINELAND_COPING_V_SCALED","VINELAND_SOCIAL_STANDARD","VINELAND_SUM_SCORES",
	"VINELAND_ABC_STANDARD","VINELAND_INFORMANT","WISC_IV_VCI","WISC_IV_PRI","WISC_IV_WMI","WISC_IV_PSI","WISC_IV_SIM_SCALED","WISC_IV_VOCAB_SCALED","WISC_IV_INFO_SCALED","WISC_IV_BLK_DSN_SCALED",
	"WISC_IV_PIC_CON_SCALED","WISC_IV_MATRIX_SCALED","WISC_IV_DIGIT_SPAN_SCALED","WISC_IV_LET_NUM_SCALED","WISC_IV_CODING_SCALED","WISC_IV_SYM_SCALED","EYE_STATUS_AT_SCAN","AGE_AT_MPRAGE","BMI","anat_cnr",
	"anat_efc","anat_fber","anat_fwhm","anat_qi1","anat_snr","func_efc","func_fber","func_fwhm","func_dvars","func_outlier",
	"func_quality","func_mean_fd","func_num_fd","func_perc_fd","func_gsr","qc_rater_1","qc_notes_rater_1","qc_anat_rater_2","qc_anat_notes_rater_2","qc_func_rater_2",
	"qc_func_notes_rater_2","qc_anat_rater_3","qc_anat_notes_rater_3","qc_func_rater_3","qc_func_notes_rater_3","SUB_IN_SMP"
};

bool LoaderABIDE::FLG_QC_SET = false;
std::vector<int> LoaderABIDE::POS_QC; // qc_rater_1, qc_anat_rater_2, qc_func_rater_2, qc_anat_rater_3


void LoaderABIDE::InitPOS_QC()
{
	if(!FLG_QC_SET) {
		POS_QC = FindOffsets(header,
		{ "qc_rater_1", "qc_anat_rater_2", "qc_func_rater_2", "qc_anat_rater_3" });
		FLG_QC_SET = true;
	}
}

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

std::vector<SubjectInfo> LoaderABIDE::loadSubjectsFromDescFile(
	const std::string& fn, const std::string& qcMethod, const int nSubject, const int nSkip)
{
	InitPOS_QC();

	string filename(fn);
	if(filename.find("Phenotypic_V1_0b_preprocessed1") == string::npos) {
		size_t pos_slash = filename.find_last_of("/\\");
		if(pos_slash == filename.length() - 1) {
			filename += "Phenotypic_V1_0b_preprocessed1.csv";
		} else {
			filename += "/Phenotypic_V1_0b_preprocessed1.csv";
		}
	}

	ifstream fin(filename);
	if(!fin) {
		cerr << "Cannot open phenotype file with given parameter: " << fn
			<< (fn == filename ? "" : ", file: " + filename) << endl;
		throw invalid_argument("cannot create valid list with given phenotype file location");
	}

	std::string line;
	getline(fin, line);
	if(!checkHeader(line)) {
		cerr << "Header line of file '" << fn << "' is not correct!" << endl;
		throw invalid_argument("file header does not match that of the specific dataset");
	}
	int limit = nSubject >= 0 ? nSubject + max(0, nSkip) : numeric_limits<int>::max();

	QCChecker * pchecker = CheckerFactory::generate(qcMethod, POS_QC.size());
	vector<SubjectInfo> res;
	int cnt = 0;
	while(getline(fin, line))
	{
		if(++cnt <= nSkip)
			continue;
		bool valid;
		string sid;
		int type;
		tie(valid, sid, type) = parsePhenotypeLine(line, pchecker);
		pchecker->reset();

		if(valid) {
			res.push_back(SubjectInfo{ sid,type });
		}

		if(cnt > limit)
			break;
	}
	fin.close();
	return res;
}

std::vector<SubjectInfo> LoaderABIDE::pruneAndAddScanViaScanFile(
	std::vector<SubjectInfo>& vldList, const std::string & root)
{
	using namespace boost::filesystem;
	vector<SubjectInfo> res;
	res.reserve(vldList.size());
	path base(root);
	if(!exists(base)) {
		cerr << "Warning: cannot access the given directory for the subject files." << endl;
		return res;
	}

	regex reg(R"(^(\w+_\d{7})_rois_.+?\.1D$)");
	set<string> files;
	for(auto it = directory_iterator(base); it != directory_iterator(); ++it) {
		smatch m;
		string fn = it->path().filename().string();
		if(regex_match(fn, m, reg)) {
			files.insert(m[1].str());
		}
	}
	regex regID(R"(^[\w_]+_(\d{7})$)");
	for(SubjectInfo& s : vldList) {
		auto it = files.find(s.id);
		if(it==files.end())
			continue;
		smatch m;
		if(!regex_match(*it, m, regID))
			continue;
		string id = m[1].str();
		nameMapping[id] = *it;
		SubjectInfo si(move(id), s.type, 0);
		res.push_back(move(si));
	}
	return res;
}

string LoaderABIDE::getFilePath(const SubjectInfo &sub) {
	static regex reg(R"(^\d{7}$)");
	string fn = sub.id;
	smatch m;
	if(regex_match(fn,m,reg)) {
		fn = nameMapping.at(m[0].str());
	}
	// TODO: handle differnt ROI
	return fn + "_rois_aal.1D";
}

tc_t LoaderABIDE::loadTimeCourse(const std::string &fn)
{
	return loadTimeCourse1D(fn);
}


std::tuple<bool, std::string, int> LoaderABIDE::parsePhenotypeLine(
	const std::string &line, QCChecker* pchecker)
{
	std::string id;
	int dx;// Autism==1, Control==2
	static const int minPos = max(POS_ID, POS_DX);
	static const int maxPos= max(max(POS_ID, POS_DX),
		*max_element(POS_QC.begin(), POS_QC.end()));

	size_t plast = 0;
	size_t p = line.find(',');

	int count = 0;
	while(p != string::npos && count <= maxPos && (pchecker->needMore() || count <= minPos))
	{
		if(count == POS_ID) {
			id = line.substr(plast, p - plast);
			if(id == "no_filename") {
				return make_tuple(false, move(id), dx);
			}
		} else if(count == POS_DX) {
			dx = stoi(line.substr(plast, p - plast));
		} else if(find(POS_QC.begin(), POS_QC.end(), count) != POS_QC.end()) {
			if(p != plast) {
				string x = line.substr(plast, p - plast);
				pchecker->input(stoi(x) > 0);
			} else {
				pchecker->input();
			}
		}
		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
//	id = padID2Head(id, ID_LENGTH_FILE, PADDING);
	return make_tuple(pchecker->result(), move(id), dx);
}
