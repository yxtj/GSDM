#pragma once
#include "Motif.h"
#include <vector>
#include <unordered_map>

size_t estimateBufferSize(const Motif& m);

char* serialize(char* res, const Motif& m);
std::pair<Motif, char*> deserialize(char* p);

std::pair<char*, std::unordered_map<Motif, std::pair<int, double>>::const_iterator> serializeMP(
	char* res, int bufSize, std::unordered_map<Motif, std::pair<int, double>>::const_iterator it,
	std::unordered_map<Motif, std::pair<int, double>>::const_iterator itend);
std::unordered_map<Motif, std::pair<int, double>> deserializeMP(char* p);

std::pair<char*, std::vector<Motif>::const_iterator> serializeVM(char* res, int bufSize,
	std::vector<Motif>::const_iterator it, std::vector<Motif>::const_iterator itend);
std::vector<Motif> deserializeVM(char *p);
