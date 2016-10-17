#include "CScopedTimer.h"


std::map<std::string, int> CScopedTimer::taskIDs;
std::map<std::string, std::int64_t> CScopedTimer::curTime;
std::map<std::string, std::int64_t> CScopedTimer::prevTime;
std::vector<std::string> CScopedTimer::tasks;

static inline std::int64_t GetTimeNSec() {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

CScopedTimer::CScopedTimer(const std::string& s, IAICallback *_cb): cb(_cb), task(s) {
	initialized = true;

	if (std::find(tasks.begin(), tasks.end(), task) == tasks.end()) {
		taskIDs[task] = tasks.size();
#if !defined(BUILDING_AI_FOR_SPRING_0_81_2)
		cb->DebugDrawerSetGraphLineColor(taskIDs[task], colors[taskIDs[task] % 8]);
		cb->DebugDrawerSetGraphLineLabel(taskIDs[task], task.c_str());
#endif
		tasks.push_back(task);

		curTime[task] = cb->GetCurrentFrame();
		prevTime[task] = 0;
	}

	t1 = GetTimeNSec();
}

CScopedTimer::~CScopedTimer() {
	t2 = GetTimeNSec();

	if (!initialized)
		return;
#if !defined(BUILDING_AI_FOR_SPRING_0_81_2)
		unsigned int curFrame = cb->GetCurrentFrame();

	for (size_t i = 0; i < tasks.size(); i++) {
		const int taskID = taskIDs[tasks[i]];

		if (tasks[i] == task)
			prevTime[task] = t2 - t1;

		cb->DebugDrawerAddGraphPoint(taskID, curFrame, prevTime[tasks[i]]);

		if ((curFrame - curTime[tasks[i]]) >= TIME_INTERVAL)
			cb->DebugDrawerDelGraphPoints(taskID, 1);
	}
#endif
}
