#include "TheFlyWin32.h"
#include "utils.h"

#define LYUBU_IDLE		0
#define LYUBU_UP		1
#define LYUBU_LEFT		2
#define LYUBU_DOWN		4
#define LYUBU_RIGHT		8
#define LYUBU_ATT		16
#define LYUBU_HITTED	32

class Lyubu : public FnActor
{
public:
	Lyubu(OBJECTid id, OBJECTid cid, OBJECTid tid, OBJECTid aid, int bid);
	void changeState(BYTE code, BOOL value);
	void hit(int damage);
	void init();
	bool Isdead();
	int getState();
	float getBlood();
	void addBlood(float amount);
	
	void (Lyubu::*nextFrame)(int skip);
private:
	State state;
	OBJECTid selfID, cameraID, terrainID, arrowID, bloodID;
	int arrow_billboardID, blood_billboardID;
	ACTIONid idleID, runID, nAtt1ID, nAtt2ID, nAtt3ID, nAtt4ID, hittedID, dieID;
	ACTIONid curActID, curAttID;
	float minHeight, Height;
	float maxDistance, Distance;
	float face_target;
	float blood;

	void (Lyubu::*next_state_Function)(int skip);

	void ResetCamera();	
	void IdleFunction(int);
	void UpFunction(int);
	void DownFunction(int);
	void LeftRightFunction(int);
	void AttackFunction(int);
	void HittedFunction(int);
	void DieFunction(int);
	void MakeAction();
	void turn();
	void redrawBloodBar();
};