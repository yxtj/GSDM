#include "stdafx.h"
#include "LoaderADHD200.h"

using namespace std;

const std::string LoaderADHD200::filePrefix = "sfnwmrda";
//const std::string LoaderADHD200::filePrefix = "snwmrda";

const int LoaderADHD200::POS_ID = 0;
const int LoaderADHD200::POS_DX = 5;

const std::vector<std::string> LoaderADHD200::header1={
	"ScanDir ID","Site","Gender","Age","Handedness",
	"DX","Secondary Dx ","ADHD Measure","ADHD Index","Inattentive",
	"Hyper/Impulsive","IQ Measure","Verbal IQ","Performance IQ","Full2 IQ",
	"Full4 IQ","Med Status","QC_Rest_1","QC_Rest_2","QC_Rest_3",
	"QC_Rest_4","QC_Anatomical_1","QC_Anatomical_2" };
const vector<int> LoaderADHD200::POS_QC_1 = { 17, 18, 19, 20, 21, 22 };

const std::vector<std::string> LoaderADHD200::header2 = {
	"ScanDirID","Site","Gender","Age","Handedness",
	"DX","Secondary Dx ","ADHD Measure","ADHD Index","Inattentive",
	"Hyper/Impulsive","IQ Measure","Verbal IQ","Performance IQ","Full2 IQ",
	"Full4 IQ","Med Status","Study #","QC_S1_Rest_1","QC_S1_Rest_2",
	"QC_S1_Rest_3","QC_S1_Rest_4","QC_S1_Rest_5","QC_S1_Rest_6","QC_S1_Anat",
	"QC_S2_Rest_1","QC_S2_Rest_2","QC_S2_Anat"};
const vector<int> LoaderADHD200::POS_QC_2 = { 18, 19, 20, 21, 22, 23, 24, 25, 26, 27 };

int LoaderADHD200::checkHeader(const string& line) {
	int count = 0;
	int res = 1;
	for(size_t plast = 0, p = line.find(','); p != string::npos;) {
		if(header1[count] != line.substr(plast + 1, p - plast - 2)) {
			res = 0;
			break;
		}
		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
	if(res == 1)
		return res;
	res = 2;
	count = 0;
	for(size_t plast = 0, p = line.find(','); p != string::npos;) {
		if(header2[count] != line.substr(plast + 1, p - plast - 2)) {
			res = 0;
			break;
		}
		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
	return res;
}

std::string LoaderADHD200::fixSubjectID(std::string id) const
{
	if(id.size() >= 7)
		return id;
	while(id.size() < 7)
		id = "0" + id;
	return id;
}

std::vector<SubjectInfo> LoaderADHD200::loadValidList(const std::string & fn, const int nSubject)
{
	string filename(fn);
	// if fn is a folder name, translate it into filename with ADHD200's manner
	if(filename.find("phenotypic.csv") == string::npos) {
		size_t pos_l, pos_f;
		// case 1: .../folder/
		// case 2: .../folder
		pos_f = pos_l = filename.find_last_of("/\\");
		if(pos_f == filename.length() - 1) { //case 1
			pos_f = filename.find_last_of("/\\", pos_f - 1) + 1;
		} else // case 2
			pos_l = string::npos;
		string leaf = filename.substr(pos_f, pos_l - pos_f);
		filename += leaf + "_phenotypic.csv";
	}
	ifstream fin(filename);
	if(!fin) {
		cerr << "Cannot open phenotype file with given parameter: " << fn
			<< (fn == filename ? "" : ", file: " + filename) << endl;
		throw invalid_argument("cannot open valid list with given parameter");
	}

	string line;
	getline(fin, line);
	int headerType = checkHeader(line);
	const vector<int>* pPOS_QC = nullptr;
	if(headerType == 0) {
		cerr << "Header line of file '" << fn << "' is not correct!" << endl;
		throw invalid_argument("file header does not match that of the specific dataset");
	} else if(headerType == 1) {
		pPOS_QC = &POS_QC_1;
	} else if(headerType == 2) {
		pPOS_QC = &POS_QC_2;
	}
	size_t limit = nSubject > 0 ? nSubject : numeric_limits<size_t>::max();
	std::vector<SubjectInfo> res;
	while(getline(fin, line)) {
		bool valid;
		string sid; 
		int type;
		tie(valid, sid, type) = parsePhenotypeLine(line, pPOS_QC);
		if(valid) {
			sid = fixSubjectID(sid);
			res.push_back(SubjectInfo{ move(sid), type });
		}
		if(res.size() >= limit)
			break;
	}
	fin.close();
	return res;
}

std::vector<SubjectInfo> LoaderADHD200::getAllSubjects(
	std::vector<SubjectInfo>& vldList, const std::string& root)
{
	using namespace boost::filesystem;
	vector<SubjectInfo> res;
	res.reserve(vldList.size() * 6 / 5);
	regex reg("^" + filePrefix + "(.+?)_session_\\d+?_rest_(\\d+)_.+?_TCs\\.1D$");
	for(SubjectInfo& s : vldList) {
		path base(root + "/" + s.id);
		for(auto it = directory_iterator(base); it != directory_iterator(); ++it) {
			smatch m;
			string fn = it->path().filename().string();
			if(regex_search(fn, m, reg) && m[1].str()==s.id) {
				int scanNum = stoi(m[2].str()) - 1;
				res.push_back(s);
				res.back().seqNum = scanNum;
			}
		}

	}
	return res;
}

std::string LoaderADHD200::getFilePath(const SubjectInfo & sub)
{
	return sub.id + "/" + filePrefix + sub.id + "_session_1"
		"_rest_" + to_string(sub.seqNum + 1) + "_aal_TCs.1D";
}

// tc_t = std::vector<std::vector<double>>
tc_t LoaderADHD200::loadTimeCourse(const std::string & fn)
{
	std::vector<std::vector<double>> res;
	ifstream fin(fn);
	if(!fin)
		return res;
	string line;
	// there is a header line
	getline(fin, line);
	int nNodes=0;
	for(size_t p = line.find('\t'); p != string::npos; p = line.find('\t', p + 1)) {
		++nNodes;
	}
	nNodes -= 2;
	while(getline(fin, line)) {
		if(line.empty())
			break;
		vector<double> row;
		row.reserve(nNodes);
		size_t plast, p;
		// first column: File
		p = line.find('\t');
		// second column: Sub-brick
		p = line.find('\t', p + 1);
		// valid data: start from the third column
		plast = p + 1;
		p = line.find('\t', plast);
		if(p == string::npos)
			continue;
		while(p != string::npos) {
			double t = stod(line.substr(plast, p));
			row.push_back(t);
			plast = p + 1;
			p = line.find('\t', plast);
		}
		res.push_back(move(row));
	}
	fin.close();
	return res;
}

// return <QC passed, scan id, diagnosis result>
std::tuple<bool, std::string, int> LoaderADHD200::parsePhenotypeLine(
	const std::string & line, const vector<int>* pPOS_QC)
{

	bool qc=true;
	string id;
	int dx;
	int count = 0;
	size_t plast = 0, p = line.find(',');
	try {
		while(p != string::npos) {
			if(POS_ID == count) {
				id = line.substr(plast, p);
			} else if(POS_DX == count) {
				dx = stoi(line.substr(plast, p - plast));
			} else if(find(pPOS_QC->begin(), pPOS_QC->end(), count) != pPOS_QC->end()) {
				//string qc_str = line.substr(plast, p - plast);
				// acceptable case: 1, "N/A", N/A, <empty>
				qc = qc && line.substr(plast, p - plast) != "0";
			}
			plast = p + 1;
			p = line.find(',', plast);
			++count;
		}
	} catch(...) {
		cerr << "Error when parsing line:\n" << line << endl;
		qc = false;
	}
	id = padID2Head(id, ID_LENGTH_FILE, PADDING);
	return make_tuple(qc, move(id), dx);
}
