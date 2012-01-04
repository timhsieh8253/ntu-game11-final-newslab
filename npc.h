
enum Action   {idle, run, right, left, attack, damage, defance, dead};
enum NPCState {wait, follow, attackPlayer, escape, hitted, die};

class NPC : public FnActor{
public:

	NPC();
	~NPC(); 
	bool init(OBJECTid id, int actorType);

	void setNPCActionID();
	void setNPCurAction(int action, char *ActionName);
	void changeState(int newState, int hitLevel);
	void playAction(int skip);
	void fsm(int skip);
	void decreaseLife(int value);
	int  getlife();
	int  getcurAction();
	int  getState();
	OBJECTid getid();
	bool Isdead();

private:

	OBJECTid npc_id;
	ACTIONid curActID, nextActID;
	ACTIONid idleID,   runID,    rightID,   leftID;
	ACTIONid attackID, damageID, defanceID, deadID;

	// blood bar
	int blood_billboardID;
	OBJECTid bloodID;

	Action    npcAction;
	NPCState  state;
	int	      actor;
	int	      life;
	BOOL	  flag;
	int		  hitLevel;

};