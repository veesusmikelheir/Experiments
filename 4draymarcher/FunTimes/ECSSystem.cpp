#include "ECSSystem.h"

int ECSSystem::InstantiateEntity() {
	auto ID = nextEntID++;
	Entities[ID] |= EXISTS_FLAG;
	return ID;
}

ECSSystem::ECSSystem() : Entities() {

}