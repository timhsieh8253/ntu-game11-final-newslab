#include "TheFlyWin32.h"
#include "FyMedia.h"
#include "utils.h"
#include "npc.h"
#include "Lyubu.h"
#include "FX.h"

#include <queue>
#include <vector>


using namespace std;

WORLDid gID;
VIEWPORTid vID, vID2;
SCENEid sID;
OBJECTid cID, tID, lID, lyubuID, minimap_cID, arrowID;
int billboardID;
ACTIONid curActID, idleID, runID, att1ID, att2ID, att3ID, att4ID;
FX *fx;

NPC Donzo;
Lyubu *lyubu;
OBJECTid DonzoID;

HWND hwnd,End;
MEDIAid mmID,endID;
FnMedia mP,endP;
AUDIOid audioID;
FnAudio audio1,audio2,audio3,audio4;
BOOL A1,A2,A3,A4;

State walk_state;
int turn_state;
int mf_state;

int hit_tag, hitRotate;
float hitLen = 0.0f, cameraLen = 0.0f;
float lastRad;

float turn_target = 0.0f, angle;

float minHeight = 400.0f, Height;
float maxDistance = 700.0f, Distance;

void NPCKeydown(WORLDid, BYTE, BOOL);
void NPCKeydownAttack(WORLDid gID, BYTE code, BOOL value);
void Keydown(WORLDid, BYTE, BOOL);
void RenderFunc(int);
void GameAI(int skip);



vector<NPC *> NPCs;

void attack_test(FnActor attacker, FnActor defender, int att_dis);

MyAttackQueue AttackList;

int main(int argc, char **argv)
{
	/*AllocConsole() ;
    HANDLE hd = GetStdHandle(STD_OUTPUT_HANDLE) ;
	wchar_t *pBuf;
	pBuf=(wchar_t *)GlobalLock(GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,200));
	wsprintf(pBuf,L"123\n");
    WriteConsole(hd ,pBuf,wcslen(pBuf), NULL , NULL );
    CloseHandle(hd) ;*/


	// Create a new World
	FnWorld gw;
	gID = FyWin32CreateWorld("Game Programming HW3", 0, 0, 800, 600, 32, FALSE);
	gw.Object(gID);

	// Create a viewport
	FnViewport vp;
	vID = gw.CreateViewport(0, 0, 800, 600);
	vp.Object(vID);
	vp.SetBackgroundColor(0.3f, 0.4f, 0.5f);

	// Create a 3D scene and Load Objects
	FnScene scene;
	FnTerrain terrain;

	// minimap
	FnViewport vp2;
	vID2 = gw.CreateViewport(650, 50, 120, 90);
	vp.Object(vID2);
	vp.SetBackgroundColor(0.3f, 0.4f, 0.5f);

	FnScene scene2;
	OBJECTid sID2 = gw.CreateScene(10);
	scene2.Object(sID2);
	OBJECTid tID2 = scene2.CreateObject(ROOT);
	FnObject model;
	model.Object(tID2);
	gw.SetObjectPath("NTU4\\Scenes");
	if(model.Load("terrain")==FALSE)
		exit(2);
	model.ChangeRenderGroup(0);


	float minimap_cpos[3] = {3569.0f, -3208.0f, 5000.0f};
	minimap_cID = scene2.CreateCamera(ROOT);
	FnCamera camera;
	camera.Object(minimap_cID);
	camera.SetPosition(minimap_cpos);
	float face[3] = {0.0f, 0.0f, -1.0f}, up[3] = {0.0f, -1.0f, 0.0f};
	camera.SetDirection(face, up);

	arrowID = scene2.CreateObject(ROOT);
	FnObject arrow;
	arrow.Object(arrowID);
	float arrow_size[2] = {64.0f, 64.0f};
	float arrow_pos[3] = {0.0f, 0.0f, -1000.0f};
	gw.SetTexturePath("NTU4\\Characters");
	billboardID = arrow.Billboard(arrow_pos, arrow_size, "arrowU", 0, NULL);
	FnBillBoard bb;

	bb.Object(arrowID, billboardID);
	arrow.SetParent(minimap_cID);
	arrow.ChangeRenderGroup(1);
	// minimap end


	// Set Resource Path
	gw.SetScenePath("NTU4\\Scenes");
	gw.SetObjectPath("NTU4\\Scenes");
	gw.SetTexturePath("NTU4\\Scenes\\Textures");
	gw.SetShaderPath("NTU4\\Shaders");

	sID = gw.CreateScene(10);
	scene.Object(sID);
	scene.Load("scene2");

	tID = scene.CreateTerrain(ROOT);
	terrain.Object(tID);
	terrain.Load("terrain");
	terrain.GenerateTerrainData();


	//terrain.Show(TRUE);

	// Reset path
	gw.SetObjectPath("NTU4\\Characters");
	gw.SetTexturePath("NTU4\\Characters");
	gw.SetCharacterPath("NTU4\\Characters");
	gw.SetShaderPath("NTU4\\Shaders");

	// Camera
	cID = scene.CreateCamera(ROOT);

	// Lyubu

	OBJECTid lyubuID = scene.LoadActor("Lyubu");
	lyubu = new Lyubu(lyubuID, cID, tID, arrowID, billboardID);
	lyubu->Object(lyubuID);
	lyubu->init();

	// Load Donzo
	DonzoID = scene.LoadActor("Donzo");
	NPCs.push_back(&Donzo);
	Donzo.init(DonzoID, 0, cID);

	float pos[3] = {3569.0f, -3208.0f, 0.f};
	pos[0] = 3800.0f;
	Donzo.SetPosition(pos);
	Donzo.TurnRight(37);

	if(!Donzo.PutOnTerrain(tID, FALSE, 0.0f))
	   exit(1);

	// Default Action is Idle
	Donzo.setNPCurAction(idle, "CombatIdle");
	Donzo.Play(0, START, 0.0f, FALSE, TRUE);

	// Load Robber
	OBJECTid RobberID = scene.LoadActor("Robber02");
	NPC *robber = new NPC();
	NPCs.push_back(robber);
	robber->init(RobberID, 1, cID);

	pos[0] = 3454.f;
	pos[1] = -3874.f;
	pos[2] = 100.0f;
	robber->SetPosition(pos);

	if(!robber->PutOnTerrain(tID, FALSE, 0.0f))
	   exit(3);

	robber->changeState(wait, 0);

	//FX
	fx = new FX(sID);

	// Room
	OBJECTid roomID = scene.CreateRoom(COLLISION_ROOM, 100);
	FnRoom room;
	room.Object(roomID);
	room.AddEntity(lyubuID);
	room.AddEntity(DonzoID);


	// Light
	FnLight light;
	lID = scene.CreateLight(ROOT);
	light.Object(lID);
	light.SetName("MainLight");
	light.Translate(-50.0f, -50.0f, 50.0f, GLOBAL);

	FnLight light2;
	OBJECTid lID2 = scene2.CreateLight(ROOT);
	light2.Object(lID);
	light2.SetName("MapLight");
	light2.Translate(-50.0f, -50.0f, 50.0f, GLOBAL);



	// Set Hotkeys
	FyDefineHotKey(FY_W, Keydown, FALSE);
	FyDefineHotKey(FY_A, Keydown, FALSE);
	FyDefineHotKey(FY_S, Keydown, FALSE);
	FyDefineHotKey(FY_D, Keydown, FALSE);
	FyDefineHotKey(FY_UP, Keydown, FALSE);
	FyDefineHotKey(FY_LEFT, Keydown, FALSE);
	FyDefineHotKey(FY_DOWN, Keydown, FALSE);
	FyDefineHotKey(FY_RIGHT, Keydown, FALSE);
	FyDefineHotKey(FY_J, Keydown, FALSE);
	FyDefineHotKey(FY_K, Keydown, FALSE);
	FyDefineHotKey(FY_L, Keydown, FALSE);
	FyDefineHotKey(FY_I, Keydown, FALSE);

	FyDefineHotKey(FY_T, NPCKeydown, FALSE);
	FyDefineHotKey(FY_F, NPCKeydown, FALSE);
	FyDefineHotKey(FY_G, NPCKeydown, FALSE);
	FyDefineHotKey(FY_H, NPCKeydown, FALSE);

	FyDefineHotKey(FY_1, NPCKeydownAttack, FALSE);
	FyDefineHotKey(FY_2, NPCKeydownAttack, FALSE);
	FyDefineHotKey(FY_3, NPCKeydownAttack, FALSE);
	FyDefineHotKey(FY_4, NPCKeydownAttack, FALSE);

	/* bind a timer, frame rate = 30 fps */
	FyBindTimer(0, 30.0f, RenderFunc, TRUE);
	FyBindTimer(1, 30.0f, GameAI, TRUE);

	// Play Background music
	FyBeginMedia("NTU4\\Media", 1);
	hwnd = FyGetWindowHandle(gw.Object());
	mmID = FyCreateMediaPlayer(0, "menu", 0, 0, 0, 0);
	mP.Object(mmID);
	mP.Play(LOOP);
	End = FyGetWindowHandle(gw.Object());
	endID = FyCreateMediaPlayer(0, "end", 0, 0, 0, 0);

	gw.SetAudioPath("NTU4\\Audio");
    audioID = gw.CreateAudio();
    audio1.Object(audioID);
    A1 = audio1.Load("03_pose18");
	audioID = gw.CreateAudio();
	audio2.Object(audioID);
	A2 = audio2.Load("03_pose25"); // Lyubu died
	audioID = gw.CreateAudio();
    audio3.Object(audioID);
    A3 = audio3.Load("02_pose25"); //Dozon die
	audioID = gw.CreateAudio();
    audio4.Object(audioID);
    A4 = audio4.Load("01_pose07");

	FyInvokeTheFly(TRUE);


	return 0;
}

void GameAI(int skip)
{
	(lyubu->*(lyubu->nextFrame))(skip); // 很複雜的 Function 指標 囧 不過可以省下 if else
	int i, num = NPCs.size();
	for(i=0;i<num;i++)
		NPCs[i]->fsm(skip);
	AttackList.reduceDelay(skip);

	// 有 Attack Event
	if(AttackList.size()>0)
	{
		AttackEvent ae = AttackList.front();
		// delay 小於零，代表可以處理了
		if(ae.delay < 0)
		{
			// pop 掉最上面的 event
			AttackList.pop();

			// 呂布砍人
			if(ae.actor == lyubu)
			{
				
				// 檢查每個 NPC 有沒有被砍
				for(i=0;i<num;i++)
				{
					float start[3], pos[3], attdir[3], tmp[3], dis[3];
					ae.actor->GetPosition(start);
					//lyubu->GetPosition(start);

					NPC *npc;
					npc = NPCs[i];
					// 沒死的才判斷
					if(!npc->Isdead())
					{
						npc->GetPosition(pos);
						//Donzo.GetPosition(pos);
						ae.actor->GetDirection(attdir, tmp);
						FVector::Minus(pos, start, dis);
						FVector::Project(dis, attdir, tmp);
						float angle = FVector::Angle(attdir, dis);
						if(angle<90 && FVector::Magnitude(tmp)<=ae.length && sin(angle*3.14159/180)*FVector::Magnitude(tmp) < ae.width)
						{
							npc->changeState(hitted,ae.damage);
							if(npc == &Donzo){
								if(npc->Isdead())
									audio3.Play(0);
								else{
									audio4.Play(0);
								}
							}
						}
					}
					// 死了之後就不要再判斷了
					else
					{
						NPCs.erase(NPCs.begin() + i);
						num--;
						i--;
					}
				}
			}
			// 呂布被砍
			else
			{
				lyubu->hit(ae.damage);
				if(lyubu->Isdead())
					audio2.Play(0);
				else
					audio1.Play(0);
			}

		}
	}
	FnCamera camera;
	camera.Object(minimap_cID);
	float pos[3];
	lyubu->GetPosition(pos);
	pos[2] = 5000;
	camera.SetPosition(pos);

	if(lyubu->getBlood()<=0){
		mP.Stop();
		endP.Object(endID);
		endP.Play(ONCE);
	}

	/*if(!fx00->Play((float) skip))
		fx00->Reset();*/
	fx->Play((float) skip);
	return ;
}

void NPCKeydown(WORLDid gID, BYTE code, BOOL value)
{
	Donzo.changeRunState(code,value);
	if(Donzo.getState() != 4)
		Donzo.changeState(follow,0);
}

void NPCKeydownAttack(WORLDid gID, BYTE code, BOOL value)
{
	Donzo.changeAttackState(code,value);
}


void Keydown(WORLDid gID, BYTE code, BOOL value)
{
	lyubu->changeState(code, value);
}

void RenderFunc(int skip)
{
	FnViewport vp;
	vp.Object(vID);
	vp.Render(cID, TRUE, TRUE);

	FnViewport vp2;
	vp2.Object(vID2);
	vp2.Render(minimap_cID, TRUE, TRUE);

	FnWorld gw;
	gw.Object(gID);

	// Show Message

	char msg[200];
	/*FnActor lyubu;
	FnCamera camera;
	lyubu.Object(lyubuID);
	camera.Object(cID);
	float lpos[3], cpos[3], lfdir[3], ludir[3];
	lyubu.GetPosition(lpos);
	lyubu.GetDirection(lfdir, ludir);
	camera.GetPosition(cpos);*/

	float lpos[3];
	lyubu->GetPosition(lpos);


	gw.StartMessageDisplay();
/*	sprintf(msg, "Lyubu: %.03f %.03f %.03f\nCamera: %.03f %.03f %.03f", lpos[0], lpos[1], lpos[2], cpos[0], cpos[1], cpos[2]);
	gw.MessageOnScreen(10, 10, msg, 255, 255, 255);
	sprintf(msg, "Lyubu face: %.03f %.03f %.03f\nLyubu up: %.03f %.03f %.03f", lfdir[0], lfdir[1], lfdir[2], ludir[0], ludir[1], ludir[2]);
	gw.MessageOnScreen(10, 50, msg, 255, 255, 255);
	camera.GetDirection(lfdir, ludir);
	sprintf(msg, "Camera face: %.03f %.03f %.03f\nCamera up: %.03f %.03f %.03f", lfdir[0], lfdir[1], lfdir[2], ludir[0], ludir[1], ludir[2]);
	gw.MessageOnScreen(10, 90, msg, 255, 255, 255);*/
/*	sprintf(msg, "State = %d, Turn = %d, MF = %d, Turn_target = %.02f, Angle = %.02f, Hit tag:%d, Hit rotate:%d, cameraLen= %f, hitLen = %f", walk_state, turn_state, mf_state, turn_target, angle,  hit_tag, hitRotate, cameraLen, hitLen);
	gw.MessageOnScreen(10, 50, msg, 255, 255, 255);

	sprintf(msg, "NPC: state = %d, life = %d, curaction = %d", Donzo.getState(), Donzo.getlife(), Donzo.getcurAction());
	gw.MessageOnScreen(10, 10, msg, 255, 255, 255);*/

	sprintf(msg, "npc: %d  State:%d  Life = %d", Donzo.getTest(), Donzo.getState(), Donzo.getlife());
	gw.MessageOnScreen(10, 10, msg, 255, 255, 255);
	sprintf(msg, "State:%d Blood = %.0f Pos: %f %f\n", lyubu->getState(), lyubu->getBlood(), lpos[0], lpos[1]);
	gw.MessageOnScreen(10, 30, msg, 255, 255, 255);

	float output[2], input[3];
	Donzo.GetPosition(input);
	/*if(vp.ComputeScreenPosition(cID, output, input, PHYSICAL_SCREEN, FALSE))
	{
		sprintf(msg, "donzo pos = %.3f %.3f", output[0], output[1]);
		gw.MessageOnScreen(10, 10, msg, 255, 255, 255);
	}*/

	gw.FinishMessageDisplay();

	// Show Message End

	gw.SwapBuffers();
}
