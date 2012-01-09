#include "utils.h"

enum Action   {idle, run, move_right, move_left, attack, damage, defance, dead};
enum NPCState {wait, follow, attackPlayer, escape, hitted, die};

#define NPC_IDLE		0
#define NPC_UP			1
#define NPC_move_left		2
#define NPC_DOWN		4
#define NPC_move_right		8
#define NPC_ATT			16

class NPC : public FnActor{
public:

	NPC();
	~NPC(); 
	bool init(OBJECTid id, int actorType, OBJECTid cameraid);

	void setNPCActionID();
	void setNPCurAction(int action, char *ActionName);
	void changeState(int newState, int Level);
	void playAction(int skip);
	void fsm(int skip);
	void decreaseLife(int value);
	int  getlife();
	int  getcurAction();
	int  getState();
	int  getHitNum();
	int  getTest();


	OBJECTid getid();
	bool Isdead();

	void changeRunState(BYTE code, BOOL value);
	void changeAttackState(BYTE code, BOOL value);
	void MakeAction();
	void turn();


private:
	AttackEvent ae;
	OBJECTid npc_id, cameraID;
	ACTIONid curActID, nextActID;
	ACTIONid idleID,   runID,    move_rightID,   move_leftID;
	ACTIONid attackID, damageID, defanceID, deadID;
	State    runState, nextRunState;
	float face_NPC;
	int cd;
	void (NPC::*action)();
	void waitAction();
	void followAction();
	void attackAction();
	void hittedAction();
	void dieAction();


	// blood bar
	int blood_billboardID;
	OBJECTid bloodID;

	Action    npcAction;
	NPCState  state;
	int	      actor;
	int	      life;
	BOOL	  flag;
	int		  hitLevel, attackLevel;
	int		  hitNum;

	int		  test;

};