// Synthetize.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Option.h"
#include "Motif.h"
#include "Graph.h"
#include "MotifSelector.h"

using namespace std;

vector<Motif> genMotif(default_random_engine& engin, const Option& opt) {
	vector<Motif> res;
	int nMotif = opt.nNegMtf + opt.nPosMtf;
	uniform_int_distribution<int> disMotSz(opt.sMotifMin, opt.sMotifMax);
	for(int i = 0; i < nMotif; ++i) {
		int nEdge = disMotSz(engin);
		Motif m;
		do {
			m.autoGen(engin(), i, nEdge, opt.nNode);
		} while(find(res.begin(), res.end(), m) != res.end());
//		cout << i << " motifs have been generated" << endl;
		res.push_back(move(m));
	}
	return res;
}

vector<int> selectMotifs(default_random_engine& engin, 
	const vector<Motif>& motifs, const vector<double> thresholds) 
{
	uniform_real_distribution<double> prob(0.0, 1.0);
	vector<int> res;
	for(size_t i = 0; i < motifs.size(); ++i) {
		if(prob(engin) < thresholds[i])
			res.push_back(i);
	}
	return res;
}

Graph genGraph(default_random_engine& engin, const Option& opt,
	const vector<Motif>& motifs, const vector<int>& usedMotifID)
{
	// motifs[usedMotifID] will happen with prob. of opt.pMotif
	Graph g;
	g.autoGen(engin, opt.nNode, opt.degAlpha, motifs, usedMotifID);
	return g;
}

void outputMotifAbstractHeader(ostream& os) {
	os << "ID\t#Node\t#Edge\tProb.OnPos\tProb.OnNeg\tStructure" << '\n';
}
void outputMotifAbstract(ostream& os, const Motif& m,  const double freqPos, const double freqNeg) {
	os << m.id << "\t" << m.nNode << "\t" << m.nEdge << "\t";// << std::fixed << prob << "\t";
	os << std::fixed << freqPos  << "\t" << std::fixed << freqNeg << "\t";
	for(const Edge& e : m.edges) {
		os << "(" << e.s << "," << e.d << ") ";
	}
	os << '\n';
}

void outputGraphAbstractHeader(ostream& os) {
	os << "Name\t#Node\t#Edge\tMotifs" << '\n';
}
void outputGraphAbstract(ostream& os, const string& name, const Graph& g) {
	os << name << "\t" << g.nNode << "\t" << g.nEdge << "\t";
	for(const int id : g.motifID) {
		os << id << " ";
	}
	os << '\n';
}

vector<vector<int>> outputData(const string& prePos, const string& preNeg,
	const Option& opt, default_random_engine& engin, const std::vector<Motif>& motifs,
	const vector<double>& probMotifPos, const vector<double>& probMotifNeg)
{
	// statistical info motif occurrence on each individual:
	vector<vector<int>> stat(opt.nPosInd + opt.nNegInd, vector<int>(opt.nPosMtf + opt.nNegMtf, 0));

	auto fun = [&](const int i, const vector<double>& probMotifReference, const string& pre, ostream& osSmry) {
		// generate a list of used motif ID for each snapshot
		// ensure that the generated data will result a no-less probability
		MotifSelector *selector = new MotifSelectorEnsure(probMotifReference.size(), opt.nSnapshot, probMotifReference);
		for(int j = 0; j < opt.nSnapshot; ++j) {
			vector<int> toUse = selector->select(engin);
			string gName = to_string(i) + "-" + to_string(j);
			Graph g = genGraph(engin, opt, motifs, toUse);
			ofstream fout(pre + gName + ".txt");
			fout << g << endl;
			fout.close();
			outputGraphAbstract(osSmry, gName, g);
			for(int mid : g.motifID) {
				++stat[i][mid];
			}
		}
		delete selector;
	};
	cout << "Generating graphs for positive individuals..." << endl;
	ofstream foutGP(opt.prefix + prePos + "graphs.txt");
	outputGraphAbstractHeader(foutGP);
	for(int i = 0; i < opt.nPosInd; ++i) {
		cout << "Generating graphs for individual " << i << endl;
		fun(i, probMotifPos, opt.prefix + opt.subFolderGraph + prePos, foutGP);
	}
	foutGP.close();

	cout << "Generating graphs for negative individuals..." << endl;
	ofstream foutGN(opt.prefix + preNeg+ "graphs.txt");
	outputGraphAbstractHeader(foutGN);
	for(int i = 0; i < opt.nNegInd; ++i) {
		cout << "Generating graphs for individual " << i << endl;
		fun(opt.nPosInd + i, probMotifNeg, opt.prefix + opt.subFolderGraph + preNeg, foutGN);
	}
	foutGN.close();

	return stat;
}

int main(int argc, char* argv[])
{
	Option opt;
	if(!opt.parseInput(argc, argv)) {
		return 1;
	}
	cout << "Nodes: " << opt.nNode << "\tAlpha for PowerLaw distribution: " << opt.degAlpha
		<< "\nOutput folder: " << opt.prefix << "\tGraph sub-folder: "<<opt.subFolderGraph
		<< "\n# Individual +/-: " << opt.nPosInd << " - " << opt.nNegInd
		<< "\t# snapshots: " << opt.nSnapshot
		<< "\n# Motif +/-: " << opt.nPosMtf << " - " << opt.nNegMtf
		<< "\tMean motif probability of single patient: " << opt.pMotif
		<<"\nMotif size min/max: "<<opt.sMotifMin<<" - "<<opt.sMotifMax<< endl;

	{
		boost::filesystem::path p(opt.prefix + opt.subFolderGraph);
		if(!boost::filesystem::is_directory(p)) {
			cout << "Creating output folder " << endl;
			boost::filesystem::create_directories(p);
			// CANNOT directly check the return value of create_directories in boost 1.59
			if(!boost::filesystem::is_directory(p)) {
				cerr << "Cannot create output folder!" << endl;
				return 2;
			}
		}
	}
	string prefix = opt.prefix;
	string prePos("1-"), preNeg("0-");

	default_random_engine engin(opt.seed);

	cout << "Generating motifs... (# pos: " << opt.nPosMtf << " # neg: " << opt.nNegMtf << ")" << endl;
	//the first opt.nPosMtf motifs are positive, rest are negative
	vector<Motif> motifs = genMotif(engin, opt);
	cout << motifs.size() << " motifs have been generated" << endl;

	// probability of each motif, regardless group difference
	vector<double> probMotif(opt.nPosMtf + opt.nNegMtf);
	{
		normal_distribution<double> disTest(opt.pMotif, min(opt.pMotif, 1.0 - opt.pMotif) / 3.0);
		for(size_t i = 0; i < probMotif.size(); ++i)
			probMotif[i] = min(1.0, max(0.0, disTest(engin)));
	}
	// prepare the probability of each motif on two groups
	vector<double> probMotifPos, probMotifNeg;
	probMotifPos = probMotifNeg = probMotif;
	for(int i = opt.nPosMtf; i < opt.nPosMtf + opt.nNegMtf; ++i)
		probMotifPos[i] = 0.0;
	for(int i = 0; i < opt.nPosMtf; ++i)
		probMotifNeg[i] = 0.0;

	// generate and output each graph
	// returned statistics: row->individual, column->motif, content: occurrence times of a motif on the snapshots of a individual
	vector<vector<int>> stat = outputData(prePos, preNeg, opt,
		engin, motifs, probMotifPos, probMotifNeg);

	// output motif summary abstract information
	cout << "Outputting motif summary information" << endl;
	{
		double devPos = opt.nPosInd*opt.nSnapshot;
		double devNeg = opt.nNegInd*opt.nSnapshot;
		ofstream fout(prefix + "motifs.txt");
		outputMotifAbstractHeader(fout);
		for(size_t i = 0; i < motifs.size(); ++i) {
			int cp = accumulate(stat.begin(), stat.begin() + opt.nPosInd, 0,
				[i](const int v1, const vector<int>& v2) {return v1 + v2[i]; });
			int cn = accumulate(stat.begin() + opt.nPosInd, stat.end(), 0,
				[i](const int v1, const vector<int>& v2) {return v1 + v2[i]; });
			outputMotifAbstract(fout, motifs[i], cp/devPos, cn/devNeg);
		}
		fout.close();
	}

	// output individual-motif statistic
	cout << "Outputting individual-motif statistic matrix" << endl;
	{
		ofstream fout(prefix + "stat-i-m.txt");
		fout << "Ind-ID";
		for(size_t i = 0; i < motifs.size(); ++i)
			fout << "\tM" << i;
		fout << '\n';
		fout.precision(3);
		double dev = opt.nSnapshot;
		for(size_t i = 0; i < stat.size(); ++i) {
			fout << "Ind" << i;
			for(size_t j = 0; j < stat[i].size(); ++j)
				fout << '\t' << std::fixed << stat[i][j] / dev;
			fout << '\n';
		}
	}


	return 0;
}

