#include "MELayer.h"
#include <vector>

using namespace std;

struct _MELayerData {
	vector<Motif> current, next, nextAbandon;
};

MELayer::MELayer()
	:data(nullptr)
{
}

MELayer::~MELayer()
{
	delete data;
}

bool MELayer::parse(const std::vector<std::string>& param)
{
	data = new _MELayerData();

	return true;
}

bool MELayer::isStable() const
{
	return true;
}
