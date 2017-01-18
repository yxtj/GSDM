#pragma once
/*
For customized type: Motif
*/
#include "../common/Motif.h"
#include <vector>
#include <unordered_map>


/*
single
*/
size_t estimateSizeMotif(const Motif& m);
char* serializeMotif(char* res, int bufSize, const Motif& item);
char* serializeMotif(char* res, const Motif& item);
std::pair<Motif, const char*> deserializeMotif(const char* p);

/*
some existing container
*/
std::pair<char*, std::unordered_map<Motif, std::pair<int, double>>::const_iterator> serializeMP(
	char* res, int bufSize, std::unordered_map<Motif, std::pair<int, double>>::const_iterator first,
	std::unordered_map<Motif, std::pair<int, double>>::const_iterator last);
std::pair<std::unordered_map<Motif, std::pair<int, double>>, const char*> deserializeMP(const char* p);

std::pair<char*, std::vector<Motif>::const_iterator> serializeVM(char* res, int bufSize,
	std::vector<Motif>::const_iterator first, std::vector<Motif>::const_iterator last);
std::pair<std::vector<Motif>, const char*> deserializeVM(const char *p);
