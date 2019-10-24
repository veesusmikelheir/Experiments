#ifndef ECSSYSTEM
#define ECSSYSTEM

#define ENTITY_COUNT 200
#define EXISTS_FLAG 1

class ECSSystem {
	int nextEntID = 1;

public:
	ECSSystem();

	unsigned char Entities[ENTITY_COUNT];

	int InstantiateEntity();
};
#endif // !ECSSYSTEM
