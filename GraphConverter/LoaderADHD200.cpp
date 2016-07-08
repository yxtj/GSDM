#include "stdafx.h"
#include "LoaderADHD200.h"

using namespace std;

std::string LoaderADHD200::filePrefix = "sfnwmrda";
//std::string LoaderADHD200::filePrefix = "snwmrda";

const std::vector<std::string> LoaderADHD200::header={
	"ScanDir ID","Site","Gender","Age","Handedness",
	"DX","Secondary Dx ","ADHD Measure","ADHD Index","Inattentive",
	"Hyper/Impulsive","IQ Measure","Verbal IQ","Performance IQ","Full2 IQ",
	"Full4 IQ","Med Status","QC_Rest_1","QC_Rest_2","QC_Rest_3",
	"QC_Rest_4","QC_Anatomical_1","QC_Anatomical_2" };

bool LoaderADHD200::checkHeader(const string& line) {
	int count = 0;
	for(size_t plast = 0, p = line.find(','); p != string::npos;) {
		if(header[count] != (line[plast] != '"' ? line.substr(plast, p - plast) 
			: line.substr(plast + 1, p - plast - 2)))
			return false;
		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
	return true;
}

std::string LoaderADHD200::fixSubjectID(std::string id) const
{
	if(id.size() >= 7)
		return id;
	while(id.size() < 7)
		id = "0" + id;
	return id;
}

std::vector<Subject> LoaderADHD200::loadValidList(const std::string & fn, const int nSubject)
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
	if(!checkHeader(line)) {
		cerr << "Header line of file '" << fn << "' is not correct!" << endl;
		throw invalid_argument("file header does not match that of the specific dataset");
	}
	size_t limit = nSubject > 0 ? nSubject : numeric_limits<size_t>::max();
	std::vector<Subject> res;
	while(getline(fin, line)) {
		bool valid;
		string sid; 
		int type;
		tie(valid, sid, type) = parsePhenotypeLine(line);
		if(valid) {
			sid = fixSubjectID(sid);
			res.push_back(Subject{ move(sid), type });
		}
		if(res.size() >= limit)
			break;
	}
	fin.close();
	return res;
}

std::vector<Subject> LoaderADHD200::getAllSubjects(
	std::vector<Subject>& vldList, const std::string& root)
{
	using namespace boost::filesystem;
	vector<Subject> res;
	res.reserve(vldList.size() * 6 / 5);
	regex reg("^" + filePrefix + "(.+?)_session_\\d+?_rest_(\\d+)_.+?_TCs\\.1D$");
	for(Subject& s : vldList) {
		path base(root + "/" + s.id);
		for(auto it = directory_iterator(base); it != directory_iterator(); ++it) {
			smatch m;
			string fn = it->path().filename().string();
			if(regex_search(fn, m, reg) && m[1].str()==s.id) {
				int scanNum = stoi(m[2].str()) - 1;
				res.push_back(s);
				res.back().sgId = scanNum;
			}
		}

	}
	return res;
}

std::string LoaderADHD200::getFilePath(const Subject & sub)
{
	return sub.id + "/" + filePrefix + sub.id + "_session_1"
		"_rest_" + to_string(sub.sgId + 1) + "_aal_TCs.1D";
}

// tc_t = std::vector<std::vector<double>>
tc_t LoaderADHD200::loadTimeCourse(const std::string & fn)
{
	std::vector<std::vector<double>> res;
	ifstream fin(fn);
	if(!fin)
		return res;
	string line;
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
		p = line.find('\t');
		p = line.find('\t', p + 1);
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
std::tuple<bool, std::string, int> LoaderADHD200::parsePhenotypeLine(const std::string & line)
{
	bool qc=true;
	string id;
	int dx;
	static const int POS_ID = 0, POS_DX = 5;
	static const vector<int> POS_QC = { 17, 18, 19, 20, 21, 22 };
	int count = 0;
	size_t plast = 0, p = line.find(',');
	try {
		while(p != string::npos) {
			if(POS_ID == count) {
				id = line.substr(plast, p);
			} else if(POS_DX == count) {
				dx = stoi(line.substr(plast, p - plast));
			} else if(find(POS_QC.begin(), POS_QC.end(), count) != POS_QC.end()) {
				string qc_str = line.substr(plast, p - plast);
				if(qc && "\"N/A\"" != qc_str && "N/A" != qc_str)
					qc = qc && !qc_str.empty() && stoi(qc_str) == 1;
			}
			plast = p + 1;
			p = line.find(',', plast);
			++count;
		}
	} catch(...) {
		cerr << "Error when parsing line:\n" << line << endl;
		qc = false;
	}
	return make_tuple(qc, move(id), dx);
}
