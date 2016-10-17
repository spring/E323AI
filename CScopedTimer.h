#ifndef SCOPEDTIMER_H
#define SCOPEDTIMER_H

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cinttypes>
#include <chrono>

#include "headers/Defines.h"
#include "headers/HAIInterface.h"
#include "headers/HEngine.h"


#define PROFILE(x) CScopedTimer t(std::string(#x), ai->cb);

// Time interval in logic frames (1 min)
#define TIME_INTERVAL 1800

static const float3 colors[] = {
	float3(1.0f, 0.0f, 0.0f),
	float3(0.0f, 1.0f, 0.0f),
	float3(0.0f, 0.0f, 1.0f),
	float3(1.0f, 1.0f, 0.0f),
	float3(0.0f, 1.0f, 1.0f),
	float3(1.0f, 0.0f, 1.0f),
	float3(0.0f, 0.0f, 0.0f),
	float3(1.0f, 1.0f, 1.0f)
};

class CScopedTimer {

public:
	CScopedTimer(const std::string& s, IAICallback *_cb);
	~CScopedTimer();

private:
	static std::vector<std::string> tasks;
	static std::map<std::string, int> taskIDs;
	static std::map<std::string, std::int64_t> curTime, prevTime;

	IAICallback *cb;
	bool initialized;
	std::int64_t t1, t2;
	const std::string task;
};

#endif
