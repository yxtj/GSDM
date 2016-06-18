#include "stdafx.h"
#include "LoaderADHD200.h"

using namespace std;

const std::vector<std::string> LoaderADHD200::header={
	"ScanDir ID","Site","Gender","Age","Handedness",
	"DX","Secondary Dx ","ADHD Measure","ADHD Index","Inattentive",
	"Hyper/Impulsive","IQ Measure","Verbal IQ","Performance IQ","Full2 IQ",
	"Full4 IQ","Med Status","QC_Rest_1","QC_Rest_2","QC_Rest_3",
	"QC_Rest_4","QC_Anatomical_1","QC_Anatomical_2" };

bool LoaderADHD200::checkHeader(const string& line) {
	int count = 0;
	for(size_t plast = 0, p = line.find(','); p != string::npos;) {
		if(line.substr(plast, p) != header[count])
			return false;
		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
	return true;
}

std::vector<std::pair<std::string, int>> LoaderADHD200::loadValidList(const std::string & fn)
{
	string filename(fn);
	// if fn is a folder name, translate it into filename with ADHD200's manner
	if(filename.find("phenotypic.csv") == string::npos) {
		size_t pos_l, pos_f;
		// case 1: .../folder/
		// case 2: .../folder
		pos_f = pos_l = filename.find_last_of("/\\");
		if(pos_f == filename.length()) //case 1
			pos_f = filename.find_last_of("/\\", pos_f - 1);
		else // case 2
			pos_l = string::npos;
		string leaf = filename.substr(pos_f, pos_l);
		filename += "/" + leaf + "_phenotypic.csv";
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

	std::vector<std::pair<std::string, int>> res;
	while(getline(fin, line)) {
		bool valid;
		string sid; 
		int type;
		tie(valid, sid, type) = parsePhenotypeLine(line);
		if(valid) {
			res.push_back(make_pair(move(sid), type));
		}
	}
	fin.close();
	return res;
}

std::vector<std::vector<double>> LoaderADHD200::loadTimeCourse(const std::string & fn)
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
				dx = stoi(line.substr(plast, p));
			} else if(find(POS_QC.begin(), POS_QC.end(), count) != POS_QC.end()) {
				string qc_str = line.substr(plast, p);
				if("N/A" != qc_str)
					qc = qc && stoi(qc_str) == 1;
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
