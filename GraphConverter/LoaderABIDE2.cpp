#include "stdafx.h"
#include "LoaderABIDE2.h"
#include "CheckerFactory.h"

using namespace std;

const int LoaderABIDE2::POS_ID = 1;
const int LoaderABIDE2::POS_DX = 3;

const std::vector<std::string> LoaderABIDE2::header = {
	"SITE_ID","SUB_ID","NDAR_GUID","DX_GROUP","PDD_DSM_IV_TR","ASD_DSM_5","AGE_AT_SCAN ","SEX","HANDEDNESS_CATEGORY","HANDEDNESS_SCORES",
	"FIQ","VIQ","PIQ","FIQ_TEST_TYPE","VIQ_TEST_TYPE","PIQ_TEST_TYPE","ADI_R_SOCIAL_TOTAL_A","ADI_R_VERBAL_TOTAL_BV","ADI_R_NONVERBAL_TOTAL_BV","ADI_R_RRB_TOTAL_C",
	"ADI_R_ONSET_TOTAL_D","ADI_R_RSRCH_RELIABLE","ADOS_MODULE","ADOS_RSRCH_RELIABLE","ADOS_G_TOTAL","ADOS_G_COMM","ADOS_G_SOCIAL","ADOS_G_STEREO_BEHAV","ADOS_G_CREATIVITY","ADOS_2_SOCAFFECT",
	"ADOS_2_RRB","ADOS_2_TOTAL","ADOS_2_SEVERITY_TOTAL","SRS_EDITION","SRS_VERSION","SRS_INFORMANT","SRS_TOTAL_RAW","SRS_AWARENESS_RAW","SRS_COGNITION_RAW","SRS_COMMUNICATION_RAW",
	"SRS_MOTIVATION_RAW","SRS_MANNERISMS_RAW","SRS_TOTAL_T","SRS_AWARENESS_T","SRS_COGNITION_T","SRS_COMMUNICATION_T","SRS_MOTIVATION_T","SRS_MANNERISMS_T","SCQ_VERSION","SCQ_TOTAL",
	"AQ_TOTAL","NONASD_PSYDX_ICD9CODE","NONASD_PSYDX_LABEL","CURRENT_MED_STATUS","CURRENT_MEDICATION_NAME","OFF_STIMULANTS_AT_SCAN","EYE_STATUS_AT_SCAN","AGE_AT_MPRAGE","BMI","VINELAND_RECEPTIVE_V_SCALED",
	"VINELAND_EXPRESSIVE_V_SCALED","VINELAND_WRITTEN_V_SCALED","VINELAND_COMMUNICATION_STANDARD","VINELAND_PERSONAL_V_SCALED","VINELAND_DOMESTIC_V_SCALED","VINELAND_COMMUNITY_V_SCALED","VINELAND_DAILYLIVING_STANDARD","VINELAND_INTERPERSONAL_V_SCALED","VINELAND_PLAY_V_SCALED","VINELAND_COPING_V_SCALED",
	"VINELAND_SOCIAL_STANDARD","VINELAND_GROSS_V_SCALED","VINELAND_FINE_V_SCALED","VINELAND_MOTOR_STANDARD","VINELAND_SUM_SCORES","VINELAND_ABC_Standard","VINELAND_INFORMANT","RBSR_6SUBSCALE_STEREOTYPED","RBSR_6SUBSCALE_SELF-INJURIOUS","RBSR_6SUBSCALE_COMPULSIVE",
	"RBSR_6SUBSCALE_RITUALISTIC","RBSR_6SUBSCALE_SAMENESS","RBSR_6SUBSCALE_RESTRICTED","RBSR_6SUBSCALE_TOTAL","RBSR_5SUBSCALE_STEREOTYPIC","RBSR_5SUBSCALE_SELF-INJURIOUS","RBSR_5SUBSCALE_COMPULSIVE","RBSR_5SUBSCALE_RITUALISTIC","RBSR_5SUBSCALE_RESTRICTED","RBSR_5SUBSCALE_TOTAL",
	"MASC_EDITION","MASC_TOTAL_T","MASC_T/R_T","MASC_S/A_T","MASC_PHYSICAL_TOTAL_T","MASC_PER_T","MASC_AC_T","MASC_HARM_TOTAL_T","MASC_H/R_T","MASC_PP_T",
	"MASC_SOCIAL_TOTAL_T","MASC_SEP_T","MASC_ADI_T","MASC_INCONSISTENCY_SCORE","BRIEF_VERSION","BRIEF_INFORMANT","BRIEF_INHIBIT_T","BRIEF_SHIFT_T","BRIEF_EMOTIONAL_T","BRIEF_BRI_T",
	"BRIEF_INITIATE_T","BRIEF_WORKING_T","BRIEF_PLAN_T","BRIEF_ORGANIZATION_T","BRIEF_MONITOR_T","BRIEF_MI_T","BRIEF_GEC_T","BRIEF_INCONSISTENCY_SCORE","BRIEF_NEGATIVITY_SCORE","CBCL_6-18_ACTIVITIES_T",
	"CBCL_6-18_SOCIAL_T","CBCL_6-18_SCHOOL_T","CBCL_6-18_TOTAL_COMPETENCE_T","CBCL_6-18_ANXIOUS_T","CBCL_6-18_WITHDRAWN_T","CBCL_6-18_SOMATIC_COMPAINT_T","CBCL_6-18_SOCIAL_PROBLEM_T","CBCL_6-18_THOUGHT_T","CBCL_6-18_ATTENTION_T","CBCL_6-18_RULE_T",
	"CBCL_6-18_AGGRESSIVE_T","CBCL_6-18_INTERNAL_T","CBCL_6-18_EXTERNAL_T","CBCL_6-18_TOTAL_PROBLEM_T","CBCL_6-18_AFFECTIVE_T","CBCL_6-18_ANXIETY_T","CBCL_6-18_SOMATIC_PROBLEM_T","CBCL_6-18_ATTENTION_DEFICIT_T","CBCL_6-18_OPPOSITIONAL_T","CBCL_6-18_CONDUCT_T",
	"CBCL_6-18_SLUGGISH_T","CBCL_6-18_OBSESSIVE_T","CBCL_6-18_POST_TRAUMATIC_T","CBCL_1.5-5_EMOTION_T","CBCL_1.5-5_ANXIOUS_T","CBCL_1.5-5_SOMANTIC_T","CBCL_1.5-5_WITHDRAWN_T","CBCL_1.5-5_SLEEP_T","CBCL_1.5-5_ATTENTION_PROBLEM_T","CBCL_1.5-5_AGGRESSIVE_T",
	"CBCL_1.5-5_INTERNAL_T","CBCL_1.5-5_EXTERNAL_T","CBCL_1.5-5_TOTAL_T","CBCL_1.5-5_STRESS_T","CBCL_1.5-5_AFFECTIVE_T","CBCL_1.5-5_ANXIETY_T","CBCL_1.5-5_PERVASIVE_T","CBCL_1.5-5_ATTENTION_DEFICIT_T","CBCL_1.5-5_OPPOSITIONAL_T","BDI_EDITION",
	"BDI_TOTAL","WIAT-II-A_WORD_T","WIAT-II-A_NUMERICAL_T","WIAT-II-A_SPELLING_T","WIAT-II-A_TOTAL_COMPOSITE_S","VMI_EDITION","VMI_VMI_S","CELF_EDITION","CELF_5-8_CORE_S","CELF_5-8_RECEPTIVE_S",
	"CELF_5-8_EXPRESSIVE_S","CELF_9-21_CORE_S","CELF_9-21_RECEPTIVE_S","CELF_9-21_EXPRESSIVE_S","BASC2_PRS_VERSION","BASC2_PRS_ANGER_T","BASC2_PRS_HYPERACTIVITY_T","BASC2_PRS_AGGRESSION_T","BASC2_PRS_CONDUCT_T","BASC2_PRS_EXTERNAL_T",
	"BASC2_PRS_EXTERNAL_MEAN_T","BASC2_PRS_ANXIETY_T","BASC2_PRS_DEPRESSION_T","BASC2_PRS_SOMATIZATION_T","BASC2_PRS_INTERNAL_T","BASC2_PRS_INTERNAL_MEAN_T","BASC2_PRS_ATYPICALITY_T","BASC2_PRS_WITHDRAWAL_T","BASC2_PRS_ATTENTION_T","BASC2_PRS_BSI_T",
	"BASC2_PRS_BSI_MEAN_T","BASC2_PRS_ADAPTABILITY_T","BASC2_PRS_SOCIAL_T","BASC2_PRS_LEADERSHIP_T","BASC2_PRS_ACTIVITIES_T","BASC2_PRS_FUNCTIONAL_T","BASC2_PRS_ADAPTIVE_T","BASC2_PRS_ADAPTIVE_MEAN_T","BASC2_PRS_BULLY_T","BASC2_PRS_DSD_T",
	"BASC2_PRS_EMOTIONAL_T","BASC2_PRS_EXECUTIVE_T","BASC2_PRS_NEGATIVE_T","BASC2_PRS_RESILIENCY_T","CPRS_OPP","CPRS_COG-INATT","CPRS_HYPERACT","CPRS_ANX_SHY","CPRS_PERFECT","CPRS_SOCIAL_PROB",
	"CPRS_PSYCHO_SOMATIC","CPRS_CONN_ADHD","CPRS_REST_IMPULS","CPRS_EMOT_LABILITY","CPRS_CONN_GI_TOTAL","CPRS_DSM_IV_INATT","CPRS_DSM_IV_HYPER_IMPUL","CPRS_DSM_IV_TOTAL","CPRS_INATT_SYMPTOMS","CPRS_HYPER_IMPUL_SYMPTOMS",
	"CASI_ADHD-I_CUTOFF","CASI_ADHD-H_CUTOFF","CASI_ADHD-C_CUTOFF","CASI_ODD_CUTOFF","CASI_CD_CUTOFF","CASI_GAD_CUTOFF","CASI_SPECIFIC_PHOBIA_CUTOFF","CASI_OBSESSIONS_CUTOFF","CASI_COMPULSIONS_CUTOFF","CASI_PTSD_CUTOFF",
	"CASI_MOTOR_TICS_CUTOFF","CASI_VOCAL_TICS_CUTOFF","CASI_SOCIAL_PHOBIA_CUTOFF","CASI_SEPARATION_CUTOFF","CASI_SCHIZOPHRENIA_CUTOFF","CASI_NOCTURNAL_ENURESIS_CUTOFF","CASI_ENURESIS_ENCOPRESIS_CUTOFF","CASI_MDE_CUTOFF","CASI_DYSTHYMIC_CUTOFF","CASI_AUTISTIC_CUTOFF",
	"CASI_ASPERGER_CUTOFF","CASI_ASPD_CUTOFF","CASI_PANIC_ATTACKS_CUTOFF","CASI_SOMATIZATION_CUTOFF","CASI_SCHIZOID_PERSONALITY_CUTOFF","CASI_MANIC_EPISODE_CUTOFF","CASI_ANOREXIA_NERVOSA_CUTOFF","CASI_BULIMIA_NERVOSA_CUTOFF","CASI_SUBSTANCE_USE_CUTOFF","CSI_ADHD-I_SEVERITY",
	"CSI_ADHD-I_CUTOFF","CSI_ADHD-H_SEVERITY","CSI_ADHD-H_CUTOFF","CSI_ADHD-C_SEVERITY","CSI_ADHD-C_CUTOFF","CSI_ODD_SEVERITY","CSI_ODD_ CUTOFF","CSI_CD_SEVERITY","CSI_CD_CUTOFF","CSI_GAD_SEVERITY",
	"CSI_GAD_CUTOFF","CSI_SPECIFIC_PHOBIA_SEVERITY","CSI_SPECIFIC_PHOBIA_CUTOFF","CSI_OBSESSIONS_SEVERITY","CSI_OBSESSIONS_CUTOFF","CSI_COMPULSIONS_SEVERITY","CSI_COMPULSIONS_CUTOFF","CSI_DISTURBING_EVENTS_SEVERITY","CSI_DISTURBING_EVENTS_CUTOFF","CSI_MOTOR_TICS_SEVERITY",
	"CSI_MOTOR_TICS_CUTOFF","CSI_VOCAL_TICS_SEVERITY","CSI_VOCAL_TICS_CUTOFF","CSI_SCHIZOPHRENIA_SEVERITY","CSI_SCHIZOPHRENIA_CUTOFF","CSI_MDD_SEVERITY","CSI_MDD_CUTOFF","CSI_DYSTHYMIC_SEVERITY","CSI_DYSTHYMIC_CUTOFF","CSI_AUTISTIC_SEVERITY",
	"CSI_AUTISTIC_CUTOFF","CSI_ASPERGER_SEVERITY","CSI_ASPERGER_CUTOFF","CSI_SOCIAL_PHOBIA_SEVERITY","CSI_SOCIAL_PHOBIA_CUTOFF","CSI_SEPARATION_SEVERITY","CSI_SEPARATION_CUTOFF","CSI_ENURESIS_SEVERITY","CSI_ENURESIS_CUTOFF","CSI_ENCOPRESIS_SEVERITY",
	"CSI_ENCOPRESIS_CUTOFF","ADI_R_A1_DIRECT_GAZE","ADI_R_A1_SOCIAL_SMILE","ADI_R_A1_FACIAL_EXPRESSIONS","ADI_R_A1_TOTAL","ADI_R_A2_IMAGINATIVE_PLAY","ADI_R_A2_INTEREST_IN_CHILDREN","ADI_R_A2_RESPONSE_TO_APPROACHES","ADI_R_A2_GROUP_PLAY","ADI_R_A2_HIGHER",
	"ADI_R_A2_FRIENDSHIPS","ADI_R_A2_TOTAL","ADI_R_A3_SHOWING_DIRECTING_ATTENTION","ADI_R_A3_OFFERING_TO_SHARE","ADI_R_A3_SEEKING_SHARE_ENJOYMENT","ADI_R_A3_Total","ADI_R_A4_USE_OTHERS_BODY","ADI_R_A4_OFFERING_COMFORT","ADI_R_A4_QUALITY_SOCIAL_OVERTURES","ADI_R_A4_INAPPROPRIATE_FACIAL_EXPRESSIONS",
	"ADI_R_A4_APPROPRIATE_SOCIAL_RESPONSES","ADI_R_A4_Total","ADI_R_B1_POINTING_EXPRESS_INTEREST","ADI_R_B1_NODDING","ADI_R_B1_HEAD_SHAKING","ADI_R_B1_CONVENTIONAL_GESTURES","ADI_R_B1_TOTAL","ADI_R_B4_SPONTANEOUS_IMITATION","ADI_R_B4_IMAGINATIVE_PLAY","ADI_R_B4_IMITATIVE_SOCIAL_PLAY",
	"ADI_R_B4_TOTAL","ADI_R_B2_SOCIAL_VERBALIZATION","ADI_R_B2_RECIPROCAL_CONVERSATION","ADI_R_B2_TOTAL","ADI_R_B3_STEREOTYPED_UTTERANCES","ADI_R_B3_INAPPROPRIATE_QUESTIONS","ADI_R_B3_PRONOMINAL_REVERSAL","ADI_R_B3_NEOLOGISMS","ADI_R_B3_Total","ADI_R_C1_UNUSUAL_PREOCCUPATIONS",
	"ADI_R_C1_CIRCUMSCRIBED_INTERESTS","ADI_R_C1_TOTAL","ADI_R_C2_VERBAL_RITUALS","ADI_R_C2_COMPULSIONS","ADI_R_C2_TOTAL","ADI_R_C3_HAND_FINGER_MANNERISMS","ADI_R_C3_HIGHER","ADI_R_C3_OTHER_COMPLEX_MANNERISMS","ADI_R_C3_TOTAL","ADI_R_C4_REPETITIVE_USE_OBJECTS",
	"ADI_R_C4_HIGHER","ADI_R_C4_UNUSUAL_SENSORY_INTERESTS","ADI_R_C4_TOTAL","ADI_R_D_AGE_PARENT_NOTICED","ADI_R_D_AGE_FIRST_SINGLE_WORDS","ADI_R_D_AGE_FIRST_PHRASES","ADI_R_D_AGE_WHEN_ABNORMALITY","ADI_R_D_INTERVIEWER_JUDGMENT"
};
const std::vector<int> LoaderABIDE2::POS_QC = { 21, 23 }; // ADI_R_RSRCH_RELIABLE, ADOS_RSRCH_RELIABLE



std::vector<SubjectInfo> LoaderABIDE2::loadSubjectsFromDescFile(
	const std::string& fn, const std::string& qcMethod, const int nSubject, const int nSkip)
{
	string filename(fn);
	// if fn is a folder name, translate it into filename with ADHD200's manner
	if(filename.find("ABIDEII_Composite_Phenotypic") == string::npos) {
		size_t pos_slash = filename.find_last_of("/\\");
		if(pos_slash == filename.length() - 1) {
			filename += "ABIDEII_Composite_Phenotypic.csv";
		} else {
			filename += "/ABIDEII_Composite_Phenotypic.csv";
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
	return res;
}

std::vector<SubjectInfo> LoaderABIDE2::pruneAndAddScanViaScanFile(
	std::vector<SubjectInfo>& vldList, const std::string & root)
{
	using namespace boost::filesystem;
	vector<SubjectInfo> res;
	res.reserve(vldList.size());
	regex reg("^session_\\d+?_rest_(\\d+)\\.1D$");
	for(SubjectInfo& s : vldList) {
		path base(root + "/" + s.id);
		if(!exists(base))
			continue;
		for(auto it = directory_iterator(base); it != directory_iterator(); ++it) {
			smatch m;
			string fn = it->path().filename().string();
			regex_search(fn, m, reg);
			int scanNum = stoi(m[1].str()) - 1; // scan num start from 1 in the raw data
			res.push_back(s);
			res.back().seqNum = scanNum;
		}
	}
	return res;
}

std::string LoaderABIDE2::getFilePath(const SubjectInfo & sub)
{
	return sub.id + "/" + "session_1_rest_" + to_string(sub.seqNum + 1) + ".1D";
}


bool LoaderABIDE2::checkHeader(const std::string &line) {
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

std::tuple<bool, std::string, int> LoaderABIDE2::parsePhenotypeLine(
	const std::string & line, QCChecker* pchecker)
{
	std::string id;
	int dx;// Autism==1, Control==2

	static const int minPos = max(POS_ID, POS_DX);
	static const int maxPos = max(max(POS_ID, POS_DX),
		*max_element(POS_QC.begin(), POS_QC.end()));

	size_t plast = 0;
	size_t p = line.find(',');
	
	int count = 0;
	while(p != string::npos && count <= maxPos && (pchecker->needMore() || count <= minPos))
	{
		if(count == POS_ID) {
			id = line.substr(plast, p - plast);
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
	bool reliable = pchecker->result();
	return make_tuple(reliable, move(id), dx);
}
