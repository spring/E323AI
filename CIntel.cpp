#include "CIntel.h"

CIntel::CIntel(AIClasses *ai) {
	this->ai = ai;
	units = new int[MAX_UNITS];
	selector.push_back(ANTIAIR);
	selector.push_back(ASSAULT);
	selector.push_back(SCOUTER);
	selector.push_back(SNIPER);
	selector.push_back(ARTILLERY);
	for (size_t i = 0; i < selector.size(); i++)
		counts[selector[i]] = 1;
}

CIntel::~CIntel() {
	delete[] units;
}

void CIntel::update(int frame) {
	mobileBuilders.clear();
	factories.clear();
	attackers.clear();
	metalMakers.clear();
	energyMakers.clear();
	resetCounters();
	int numUnits = ai->cheat->GetEnemyUnits(units, MAX_UNITS);
	for (int i = 0; i < numUnits; i++) {
		const UnitDef *ud = ai->cheat->GetUnitDef(units[i]);
		UnitType      *ut = UT(ud->id);
		unsigned int    c = ut->cats;
		
		if (c&ATTACKER && c&MOBILE) {
			attackers.push_back(units[i]);
		}
		else if (c&FACTORY) {
			factories.push_back(units[i]);
			if (c&AIR)
				hasAir = true;
			else
				hasAir = false;
		}
		else if (c&BUILDER && c&MOBILE) {
			mobileBuilders.push_back(units[i]);
		}
		else if (c&MEXTRACTOR || c&MMAKER) {
			metalMakers.push_back(units[i]);
		}
		else if (c&EMAKER) {
			energyMakers.push_back(units[i]);
		}

		updateCounts(c);
	}
}

void CIntel::updateCounts(unsigned c) {
	std::map<unitCategory, int>::iterator i;
	for (i = counts.begin(); i != counts.end(); i++) {
		if (i->first & c) {
			i->second++;
			totalCount++;
		}
	}
}

void CIntel::resetCounters() {
	roulette.clear();
	/* Put the counts in a normalized reversed map first and reset counters*/
	for (size_t i = 0; i < selector.size(); i++) {
		roulette.insert(std::pair<float,unitCategory>(counts[selector[i]]/float(totalCount), selector[i]));
		counts[selector[i]] = 1;
	}

	totalCount = selector.size();
}

bool CIntel::enemyInbound() {
	return false;
}
