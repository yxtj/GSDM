#pragma once
#include "serializer_basic.hpp"
#include "c_motif.h"

template <>
struct _Serializer<Motif> {
	int estimateSize(const Motif& item) {
		return estimateSizeMotif(item);
	}
	char* serial(char* res, int bufSize, const Motif& item) {
		return serializeMotif(res, bufSize, item);
	}
	char* serial(char* res, const Motif& item) {
		return serializeMotif(res, item);
	}
	std::pair<Motif, const char*> deserial(const char* p) {
		return deserializeMotif(p);
	}
};

template <>
struct _Serializer<MotifBuilder> {
	int estimateSize(const MotifBuilder& item) {
		return estimateSizeMotif(item);
	}
	char* serial(char* res, int bufSize, const MotifBuilder& item) {
		return serializeMotif(res, bufSize, item);
	}
	char* serial(char* res, const MotifBuilder& item) {
		return serializeMotif(res, item);
	}
	std::pair<MotifBuilder, const char*> deserial(const char* p) {
		return deserializeMotifBuilder(p);
	}
};
