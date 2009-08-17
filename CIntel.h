#ifndef INTEL_H
#define INTEL_H

#include <vector>

#include "CE323AI.h"

class CIntel {
	public:
		CIntel(AIClasses *ai);
		~CIntel();

		bool hasAir;

		void update(int frame);
		bool enemyInbound();

		std::vector<int> factories;
		std::vector<int> attackers;
		std::vector<int> mobileBuilders;
		std::vector<int> metalMakers;
		std::vector<int> energyMakers;

		std::multimap<float,unitCategory> roulette;


	private:
		AIClasses *ai;

		int *units;
		std::map<unitCategory,int> counts;
		std::vector<unitCategory> selector;
		int totalCount;

		/* Reset enemy unit counters */
		void resetCounters();

		/* Count enemy units */
		void updateCounts(unsigned c);

};

#endif
