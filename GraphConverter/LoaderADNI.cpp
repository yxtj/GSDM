#include "stdafx.h"
#include "LoaderADNI.h"
#include "CheckerFactory.h"

using namespace std;

const int LoaderADNI::POS_ID = 0;
const int LoaderADNI::POS_DX = 7;

const int LoaderADNI::ID_LENGTH_FILE  = 4;
const char LoaderADNI::PADDING = '0';

const std::vector<std::string> LoaderADNI::header = {
	"RID","PTID","VISCODE","SITE","COLPROT","ORIGPROT","EXAMDATE","DX_bl","AGE","PTGENDER",
	"PTEDUCAT","PTETHCAT","PTRACCAT","PTMARRY","APOE4","FDG","PIB","AV45","CDRSB","ADAS11",
	"ADAS13","MMSE","RAVLT_immediate","RAVLT_learning","RAVLT_forgetting","RAVLT_perc_forgetting","FAQ","MOCA","EcogPtMem","EcogPtLang",
	"EcogPtVisspat","EcogPtPlan","EcogPtOrgan","EcogPtDivatt","EcogPtTotal","EcogSPMem","EcogSPLang","EcogSPVisspat","EcogSPPlan","EcogSPOrgan",
	"EcogSPDivatt","EcogSPTotal","FLDSTRENG","FSVERSION","Ventricles","Hippocampus","WholeBrain","Entorhinal","Fusiform","MidTemp",
	"ICV","DX","EXAMDATE_bl","CDRSB_bl","ADAS11_bl","ADAS13_bl","MMSE_bl","RAVLT_immediate_bl","RAVLT_learning_bl","RAVLT_forgetting_bl",
	"RAVLT_perc_forgetting_bl","FAQ_bl","FLDSTRENG_bl","FSVERSION_bl","Ventricles_bl","Hippocampus_bl","WholeBrain_bl","Entorhinal_bl","Fusiform_bl","MidTemp_bl",
	"ICV_bl","MOCA_bl","EcogPtMem_bl","EcogPtLang_bl","EcogPtVisspat_bl","EcogPtPlan_bl","EcogPtOrgan_bl","EcogPtDivatt_bl","EcogPtTotal_bl","EcogSPMem_bl",
	"EcogSPLang_bl","EcogSPVisspat_bl","EcogSPPlan_bl","EcogSPOrgan_bl","EcogSPDivatt_bl","EcogSPTotal_bl","FDG_bl","PIB_bl","AV45_bl","Years_bl",
	"Month_bl","Month","M","update_stamp"
};
const std::vector<int> LoaderADNI::POS_QC = { }; // no qc field



std::vector<SubjectInfo> LoaderADNI::loadSubjectsFromDescFile(
	const std::string& fn, const std::string& qcMethod, const int nSubject, const int nSkip)
{
	string filename(fn);
	// if fn is a folder name, translate it into filename with ADHD200's manner
	if(filename.find("ADNIMERGE") == string::npos) {
		size_t pos_slash = filename.find_last_of("/\\");
		if(pos_slash == filename.length() - 1) {
			filename += "ADNIMERGE.csv";
		} else {
			filename += "/ADNIMERGE.csv";
		}
	}

	ifstream fin(filename);
	if(!fin) {
		cerr << "Cannot open phenotype file with given parameter: " << fn
			<< (fn == filename ? "" : ", file: " + filename) << endl;
		throw invalid_argument("cannot create valid list with given phenotype file location");
	}

	string line;
	getline(fin, line);
	if(!checkHeader(line)) {
		cerr << "Header line of file '" << fn << "' is not correct!" << endl;
		throw invalid_argument("file header does not match that of the specific dataset");
	}

	int limit = nSubject >= 0 ? nSubject + max(0, nSkip) : numeric_limits<int>::max();

	QCChecker* pchecker = CheckerFactory::generate(qcMethod, POS_QC.size());
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
	delete pchecker;
	fin.close();

	//cout << res.size() << endl;
	auto first = res.begin();
	for(auto it = ++res.begin(); it != res.end(); ++it) {
		if(first->id == it->id && first->type == it->type) {
			continue;
		} else if(first->id == it->id && first->type != it->type) {
			cout << "  " << first->id << "\t" << first->type << "\t" << it->type << endl;
		} else {
			*++first = move(*it);
		}
	}
	res.erase(++first, res.end());
	//cout << res.size() << endl;

	//ofstream fout("unique_list.txt");
	//for(auto it = res.begin(); it != res.end(); ++it) {
	//	fout << it->id << '\t' << it->type << '\n';
	//}
	//fout.close();

	return res;
}

std::vector<SubjectInfo> LoaderADNI::pruneAndAddScanViaScanFile(
	std::vector<SubjectInfo>& vldList, const std::string & root)
{
	using namespace boost::filesystem;
	vector<SubjectInfo> res;
	res.reserve(vldList.size());
	regex reg("^(\\d+)_scan_(\\d+)\\.1D$");
	for(SubjectInfo& s : vldList) {
		path base(root + "/" + s.id);
		if(!exists(base))
			continue;
		for(auto it = directory_iterator(base); it != directory_iterator(); ++it) {
			smatch m;
			string fn = it->path().filename().string();
			if(!regex_search(fn, m, reg) || m[1] != s.id)
				continue;
			int scanNum = stoi(m[2].str());
			res.push_back(s);
			res.back().seqNum = scanNum;
		}
	}
	return res;
}

std::string LoaderADNI::getFilePath(const SubjectInfo & sub)
{
	return sub.id + "/" + sub.id + "_scan_" + to_string(sub.seqNum) + ".1D";
}

bool LoaderADNI::checkHeader(const std::string &line) {
	int count = 0;
	for(size_t plast = 0, p = line.find(','); p != string::npos;) {
		// remove the "" in the phenotype header
		if(line.substr(plast + 1, p - plast - 2) != header[count]) {
			return false;
		}
		plast = p + 1;
		p = line.find(',', plast);
		++count;
	}
	return true;
}

// Control -> CN -> 0, Alzheimer's Disease -> AD -> 1, Significant Memory Concern -> SMC -> 2,
// early Mild Cognitive Impairment -> EMCI -> 3, late Mild Cognitive Impairment -> LMCI -> 4
int LoaderADNI::giveDXId(const std::string& dx){
	static const map<string, int> TMAP{ {"CN", 0}, {"AD", 1}, {"SMC", 2}, {"EMCI", 3}, {"LMCI", 4} } ;
	return TMAP.at(dx);
}

std::tuple<bool, std::string, int> LoaderADNI::parsePhenotypeLine(
	const std::string & line, QCChecker* pchecker)
{
	std::string id;
	int dx;

	static const int minPos = max(POS_ID, POS_DX);
	static const int maxPos = max(minPos,
		POS_QC.empty() ? 0 : *max_element(POS_QC.begin(), POS_QC.end()));

	size_t plast = 0;
	size_t p = line.find(',');
	
	int count = 0;
	while(p != string::npos && count <= maxPos && (pchecker->needMore() || count <= minPos))
	{
		if(count == POS_ID) {
			// remove the \" 
			id = line.substr(plast + 1, p - plast - 2);
		} else if(count == POS_DX) {
			// remove the \" 
			dx = giveDXId(line.substr(plast + 1, p - plast - 2));
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
	id = padID2Head(id, ID_LENGTH_FILE, PADDING);
	//bool reliable = pchecker->result();
	//return make_tuple(reliable, move(id), dx);
	return make_tuple(true, move(id), dx);
}
