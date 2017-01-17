#include "stdafx.h"
#include "StrategyOFGPara.h"

using namespace std;

void StrategyOFGPara::regDSPImmediate(const int type, callback_t fp) {
	driver.registerImmediateHandler(type, bind(fp, this, placeholders::_1, placeholders::_2));
}
void StrategyOFGPara::regDSPProcess(const int type, callback_t fp) {
	driver.registerProcessHandler(type, bind(fp, this, placeholders::_1, placeholders::_2));
}
void StrategyOFGPara::regDSPDefault(callback_t fp) {
	driver.registerDefaultOutHandler(bind(fp, this, placeholders::_1, placeholders::_2));
}

