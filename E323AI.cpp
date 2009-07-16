#include "E323AI.h"

CE323AI::CE323AI() {
}

CE323AI::~CE323AI() {
	/* close the logfile */
	ai->logger->close();

	delete ai->metalMap;
	delete ai->unitTable;
	delete ai->metaCmds;
	delete ai->eco;
	delete ai->logger;
	delete ai->tasks;
	delete ai->threatMap;
	delete ai->pf;
	delete ai->intel;
	delete ai->military;
	delete ai;
}

void CE323AI::InitAI(IGlobalAICallback* callback, int team) {
	ai          = new AIClasses();
	ai->call    = callback->GetAICallback();
	ai->cheat   = callback->GetCheatInterface();
	unitCreated = 0;

	/* Retrieve mapname, time and team info for the log file */
	std::string mapname = std::string(ai->call->GetMapName());
	mapname.resize(mapname.size() - 4);

	time_t now1;
	time(&now1);
	struct tm* now2 = localtime(&now1);

	sprintf(buf, "%s", LOG_FOLDER);
	ai->call->GetValue(AIVAL_LOCATE_FILE_W, buf);
	std::sprintf(
		buf, 
		"%s%2.2d%2.2d%2.2d%2.2d%2.2d-%s-team(%d).log", 
		std::string(LOG_PATH).c_str(), 
		now2->tm_year + 1900, 
		now2->tm_mon + 1, 
		now2->tm_mday, 
		now2->tm_hour, 
		now2->tm_min, 
		mapname.c_str(), 
		team
	);
	ai->logger		= new std::ofstream(buf, std::ios::app);

	std::string version("*** " + AI_VERSION + " ***");
	LOGS(version.c_str());
	LOGS("*** " AI_CREDITS " ***");
	LOGS("*** " AI_NOTES " ***");
	LOGS(buf);
	LOGN(AI_VERSION);

	ai->metalMap	= new CMetalMap(ai);
	ai->unitTable	= new CUnitTable(ai);
	ai->metaCmds	= new CMetaCommands(ai);
	ai->eco     	= new CEconomy(ai);
	ai->wl          = new CWishList(ai);
	ai->tasks     	= new CTaskPlan(ai);
	ai->threatMap   = new CThreatMap(ai);
	ai->pf          = new CPathfinder(ai);
	ai->intel       = new CIntel(ai);
	ai->military    = new CMilitary(ai);

	LOG("\n\n\nBEGIN\n\n\n");
}


/************************
 * Unit related callins *
 ************************/

/* Called when units are spawned in a factory or when game starts */
void CE323AI::UnitCreated(int unit, int builder) {
	const UnitDef *ud = ai->call->GetUnitDef(unit);
	sprintf(buf, "[CE323AI::UnitCreated]\t %s(%d) created", ud->humanName.c_str(), unit);
	LOGN(buf);
	UnitType *ut = UT(ud->id);
	unsigned int c = ut->cats;

	if (unitCreated == 1 && ud->isCommander) {
		ai->eco->init(unit);
		ai->military->init(unit);
	}

	if (c&MEXTRACTOR) {
		ai->metalMap->taken[unit] = ai->call->GetUnitPos(unit);
	}
	else if (c&MMAKER) {
		ai->eco->gameMetalMakers[unit] = true;
	}

	ai->eco->gameBuilding[unit] = builder;

	unitCreated++;
}

/* Called when units are finished in a factory and able to move */
void CE323AI::UnitFinished(int unit) {
	const UnitDef *ud = ai->call->GetUnitDef(unit);
	sprintf(buf, "[CE323AI::UnitFinished]\t %s(%d) finished", ud->humanName.c_str(), unit);
	LOGN(buf);
	UnitType* ut = UT(ud->id);
	unsigned int c = ut->cats;

	/* Eco unit */
	if (!(c&ATTACKER) || c&COMMANDER) {
		if (c&FACTORY) {
			ai->eco->gameFactories[unit] = true;
			ai->eco->gameIdle[unit]      = ut;
			ai->military->initSubGroups(unit);
		}

		if (c&BUILDER && c&MOBILE) {
			ai->eco->gameBuilders[unit]  = ut;
			if (!(c&COMMANDER))
				ai->metaCmds->moveForward(unit, -70.0f);
		}

		if (c&MEXTRACTOR || c&MMAKER || c&MSTORAGE) {
			ai->eco->gameMetal[unit]     = ut;
			ai->tasks->updateBuildPlans(unit);
			if (c&MEXTRACTOR) {
				ai->metalMap->taken[unit] = ai->call->GetUnitPos(unit);
			}
		}

		if (c&EMAKER || c&ESTORAGE) {
			ai->eco->gameEnergy[unit]    = ut;
			ai->tasks->updateBuildPlans(unit);
		}
	}
	/* Military unit */
	else {
		if (c&MOBILE) {
			ai->metaCmds->moveForward(unit, 100.0f);
			if (c&SCOUT)
				ai->military->addToGroup(ai->eco->gameBuilding[unit], unit, G_SCOUT);
			else
				ai->military->addToGroup(ai->eco->gameBuilding[unit], unit, G_ATTACKER);
		}
	}
	ai->unitTable->gameAllUnits[unit] = ut;
}

/* Called on a destroyed unit */
void CE323AI::UnitDestroyed(int unit, int attacker) {
	const UnitDef *ud = ai->call->GetUnitDef(unit);
	sprintf(buf, "[CE323AI::UnitDestroyed]\t %s(%d) destroyed", ud->humanName.c_str(), unit);
	LOGN(buf);
	UnitType* ut = UT(ud->id);
	unsigned int c = ut->cats;

	if (!(c&ATTACKER) || c&COMMANDER) {
		if (c&FACTORY) {
			ai->eco->gameFactories.erase(unit);
			ai->eco->gameFactoriesBuilding.erase(unit);
		}

		if (c&BUILDER && c&MOBILE) {
			ai->eco->gameBuilders.erase(unit);
			ai->eco->removeMyGuards(unit);
			ai->metalMap->taken.erase(unit);
			ai->tasks->buildplans.erase(unit);
		}

		if (c&MEXTRACTOR || c&MMAKER || c&MSTORAGE) {
			ai->eco->gameMetal.erase(unit);
			if (c&MEXTRACTOR) {
				ai->metalMap->removeFromTaken(unit);
			}
			else if (c&MMAKER) {
				ai->eco->gameMetalMakers.erase(unit);
			}
		}

		if (c&EMAKER || c& ESTORAGE) {
			ai->eco->gameEnergy.erase(unit);
		}
		ai->eco->gameGuarding.erase(unit);
		ai->eco->gameIdle.erase(unit);
		ai->eco->gameBuilding.erase(unit);
	}
	else {
		ai->military->removeFromGroup(ai->eco->gameBuilding[unit], unit);
	}

	ai->unitTable->gameAllUnits.erase(unit);
}

/* Called when unit is idle */
void CE323AI::UnitIdle(int unit) {
	const UnitDef *ud = ai->call->GetUnitDef(unit);
	sprintf(buf, "[CE323AI::UnitIdle]\t %s(%d) idling", ud->humanName.c_str(), unit);
	LOGN(buf);
	UnitType* ut = UT(ud->id);
	unsigned int c = ut->cats;

	if (!(c&ATTACKER) || c&COMMANDER) {
		ai->eco->gameIdle[unit] = ut;
	}
}

/* Called when unit is damaged */
void CE323AI::UnitDamaged(int damaged, int attacker, float damage, float3 dir) {
}

/* Called on move fail e.g. can't reach point */
void CE323AI::UnitMoveFailed(int unit) {
	const UnitDef *ud = ai->call->GetUnitDef(unit);
	sprintf(buf, "[CE323AI::UnitMoveFailed]\t %s(%d) failed moving", ud->humanName.c_str(), unit);
	LOGN(buf);
	UnitType* ut = UT(ud->id);
	unsigned int c = ut->cats;

	if (!(c&ATTACKER) || c&COMMANDER) {
		ai->metaCmds->moveRandom(unit, 100.0f);
	}
	
}



/***********************
 * Enemy related callins *
 ***********************/

void CE323AI::EnemyEnterLOS(int enemy) {
}

void CE323AI::EnemyLeaveLOS(int enemy) {
}

void CE323AI::EnemyEnterRadar(int enemy) {
}

void CE323AI::EnemyLeaveRadar(int enemy) {
}

void CE323AI::EnemyDestroyed(int enemy, int attacker) {
}

void CE323AI::EnemyDamaged(int damaged, int attacker, float damage, float3 dir) {
}


/****************
 * Misc callins *
 ****************/

void CE323AI::GotChatMsg(const char* msg, int player) {
}

int CE323AI::HandleEvent(int msg, const void* data) {
	const ChangeTeamEvent* cte = (const ChangeTeamEvent*) data;
	switch(msg) {
		case AI_EVENT_UNITGIVEN:
			/* Unit gained */
			if ((cte->newteam) == (ai->call->GetMyTeam()))
				UnitFinished(cte->unit);
		break;

		case AI_EVENT_UNITCAPTURED:
			/* Unit lost */
			if ((cte->oldteam) == (ai->call->GetMyTeam()))
				UnitDestroyed(cte->unit, 0);
		break;
	}
	return 0;
}


/* Update AI per logical frame = 1/30 sec on gamespeed 1.0 */
void CE323AI::Update() {
	int frame = ai->call->GetCurrentFrame();

	/* Rotate through the different update events to distribute computations */
	switch(frame % 18) {
		case 1:  /* update threatmap */
			ai->threatMap->update(frame);
		break;

		case 3:  /* update pathfinder with threatmap */
			ai->pf->updateMap(ai->threatMap->map);
		break;

		case 5:  /* update the groups following a path */
			ai->pf->updateFollowers();
		break;

		case 7:  /* update the path itself of a group */
			ai->pf->updatePaths();
		break;

		case 9:  /* update enemy intel */
			ai->intel->update(frame);
		break;

		case 11: /* update military */
			ai->military->update(frame);
		break;

		case 13: /* update incomes */
			ai->eco->updateIncomes(frame);
		break;

		case 15: /* update economy */
			ai->eco->update(frame);
		break;

		case 17: /* update military plans */
			ai->tasks->updateMilitaryPlans();
		break;

		default: return;
	}
}
