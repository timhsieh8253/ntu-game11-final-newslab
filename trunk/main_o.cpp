#include "TheFlyWin32.h"
#include "utils.h"

#define STATE_IDLE	0
#define STATE_W		1
#define STATE_A		2
#define STATE_S		4
#define STATE_D		8
#define STATE_ATT1	16
#define STATE_ATT2	32
#define STATE_ATT3	64
#define STATE_ATT4	128

class State
{
	private:
		int state;

	public:
		int getState()
		{
			return state;
		}
		void setState(int s)
		{

			state = s;
		}

		void addState(int s)
		{
			state = state | s;
		}

		void removeState(int s)
		{
			state = state ^ s;
		}
		
		bool hasState(int s)
		{
			if((state & s) != 0)
				return true;
			return false;
		}
};

WORLDid gID;
VIEWPORTid vID;
SCENEid sID;
OBJECTid cID, tID, lyubuID, lID;
ACTIONid curActID, idleID, runID, att1ID, att2ID, att3ID, att4ID;

State walk_state;
int turn_state;
int mf_state;

int hit_tag, hitRotate;
float hitLen = 0.0f, cameraLen = 0.0f;
float lastRad;

float turn_target = 0.0f, angle;

float minHeight = 400.0f, Height;
float maxDistance = 700.0f, Distance;

void Keydown(WORLDid, BYTE, BOOL);
void RenderFunc(int);
void GameAI(int);
void ResetCamera();

int main(int argc, char **argv)
{
	// Create a new World
	FnWorld gw;
	gID = FyWin32CreateWorld("Game Programming HW2", 0, 0, 800, 600, 32, FALSE);
	gw.Object(gID);

	// Set Resource Path
	gw.SetScenePath("NTU4\\Scenes");
	gw.SetObjectPath("NTU4\\Scenes");
	gw.SetTexturePath("NTU4\\Scenes\\Textures");
	gw.SetShaderPath("NTU4\\Shaders");

	// Create a viewport
	FnViewport vp;
	vID = gw.CreateViewport(0, 0, 800, 600);
	vp.Object(vID);
	vp.SetBackgroundColor(0.3f, 0.4f, 0.5f);

	// Create a 3D scene and Load Objects
	FnScene scene;
	FnTerrain terrain;

	sID = gw.CreateScene(10);
	scene.Object(sID);
	scene.Load("scene2");

	tID = scene.CreateTerrain(ROOT);
	terrain.Object(tID);
	terrain.Load("terrain");
	terrain.GenerateTerrainData();
	
	// Reset path for lyubu
	gw.SetObjectPath("NTU4\\Characters");
	gw.SetTexturePath("NTU4\\Characters");
	gw.SetCharacterPath("NTU4\\Characters");
	gw.SetShaderPath("NTU4\\Shaders");

	FnActor lyubu;
	lyubuID = scene.LoadActor("Lyubu");
	lyubu.Object(lyubuID);
	float pos[2] = {3569.0f, -3208.0f};
	lyubu.SetPosition(pos);

/*	float   xyzPos[3];
	xyzPos[0] = 3569.0f; xyzPos[1] = -3208.0f; xyzPos[2] = 0.f;
    lyubu.SetPosition(xyzPos);*/

	//lyubu.TurnRight(37);

	if(!lyubu.PutOnTerrain(tID, FALSE, 0.0f))
	   exit(1);

	// Load Actions
	idleID = lyubu.GetBodyAction(NULL, "Idle");
	runID = lyubu.GetBodyAction(NULL, "Run");
	att1ID = lyubu.GetBodyAction(NULL, "NormalAttack1");
	att2ID = lyubu.GetBodyAction(NULL, "NormalAttack2");
	att3ID = lyubu.GetBodyAction(NULL, "NormalAttack3");
	att4ID = lyubu.GetBodyAction(NULL, "NormalAttack4");

	// Default Action is Idle
	lyubu.MakeCurrentAction(0, NULL, idleID);
	curActID = idleID;
	lyubu.Play(0, START, 0.0f, FALSE, TRUE);

	// Camera
	cID = scene.CreateCamera(ROOT);
	ResetCamera();

	// Light
	FnLight light;
	lID = scene.CreateLight(ROOT);
	light.Object(lID);
	light.SetName("MainLight");
	light.Translate(-50.0f, -50.0f, 50.0f, GLOBAL);

	walk_state.setState(STATE_IDLE);

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

	/* bind a timer, frame rate = 30 fps */
	FyBindTimer(0, 30.0f, RenderFunc, TRUE);
	FyBindTimer(1, 30.0f, GameAI, TRUE);

	FyInvokeTheFly(TRUE);

	return 0;
}

void ResetCamera()
{
	float fdir[3], udir[3];
	float pos[3], pos2[3], cdir[3];
	float cross1[3], cross2[3];

	FnCamera camera;
	FnActor lyubu;
	lyubu.Object(lyubuID);
	camera.Object(cID);
	lyubu.GetPosition(pos);
	lyubu.GetDirection(fdir, udir);
	FVector::CrossProduct(udir, fdir, cross1);
	pos2[0] = pos[0] - fdir[0]*maxDistance + udir[0]*minHeight;
	pos2[1] = pos[1] - fdir[1]*maxDistance + udir[1]*minHeight;
	pos2[2] = pos[2] - fdir[2]*maxDistance + udir[2]*minHeight;
	Distance = maxDistance;
	Height = minHeight;
	FVector::Minus(pos, pos2, cdir);
	FVector::GetUnit(cdir, cdir);
	FVector::CrossProduct(cdir, cross1, cross2);
	camera.SetPosition(pos2);
	camera.SetDirection(cdir, cross2);
}

void GameAI(int skip)
{
	FnCamera camera;
	camera.Object(cID);
	FnActor lyubu;
	lyubu.Object(lyubuID);
	float lfdir[3], ludir[3];
	float cfdir[3], cudir[3];
	lyubu.GetDirection(lfdir, ludir);
	camera.GetDirection(cfdir, cudir);

	
	if(turn_state == TRUE)
	{
		float tmp[3], tmp2[3];
		float turn_speed = 25;
		FVector::CrossProduct(ludir, cfdir, tmp);
		FVector::GetUnit(tmp, tmp);
		FVector::CrossProduct(tmp, ludir, tmp);
		FVector::GetUnit(tmp, tmp);
		angle = FVector::Angle(tmp, lfdir);
		FVector::CrossProduct(tmp, lfdir, tmp2);
		if(FVector::InnerProduct(ludir, tmp2)>0)
			angle = 360.0f-angle;
		lyubu.Play(0, LOOP, (float)skip, FALSE, TRUE);
		angle = fmod(angle, 360.0f);
		if(turn_target != angle)
		{
			float diff = turn_target - angle;
			
			if(fabs(diff) < 0.05)
			{
				turn_state = FALSE;
				return;
			}

			if(diff < 0)
				diff = 360.0f + diff;
			diff = fmod(diff, 360.0f);
			if(diff > 180)
			{
				if(diff >turn_speed)
					lyubu.TurnRight(360-turn_speed);
				else
					lyubu.TurnRight(360-diff);
			}
			else
			{
				if(diff >turn_speed)
					lyubu.TurnRight(turn_speed);
				else
					lyubu.TurnRight(diff);
				
			}
		}
		else
			turn_state = FALSE;
	}
	else
	{
		if(walk_state.hasState(STATE_ATT1))
		{
			BOOL flag;
			if(curActID == att1ID)
				flag = lyubu.Play(0, ONCE, (float)skip, FALSE, TRUE);
			else
			{
				lyubu.MakeCurrentAction(0, NULL, att1ID);
				curActID = att1ID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			if(flag==FALSE)
				walk_state.removeState(STATE_ATT1);
			return ;
		}
		if(walk_state.hasState(STATE_ATT2))
		{
			BOOL flag;
			if(curActID == att2ID)
				flag = lyubu.Play(0, ONCE, (float)skip, FALSE, TRUE);
			else
			{
				lyubu.MakeCurrentAction(0, NULL, att2ID);
				curActID = att2ID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			if(flag==FALSE)
				walk_state.removeState(STATE_ATT2);
			return ;
		}
		if(walk_state.hasState(STATE_ATT3))
		{
			BOOL flag;
			if(curActID == att3ID)
				flag = lyubu.Play(0, ONCE, (float)skip, FALSE, TRUE);
			else
			{
				lyubu.MakeCurrentAction(0, NULL, att3ID);
				curActID = att3ID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			if(flag==FALSE)
				walk_state.removeState(STATE_ATT3);
			return ;
		}
		if(walk_state.hasState(STATE_ATT4))
		{
			BOOL flag;
			if(curActID == att4ID)
				flag = lyubu.Play(0, ONCE, (float)skip, FALSE, TRUE);
			else
			{
				lyubu.MakeCurrentAction(0, NULL, att4ID);
				curActID = att4ID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			if(flag==FALSE)
				walk_state.removeState(STATE_ATT4);
			return ;
		}
		if(walk_state.getState() == STATE_IDLE)
		{
			if(curActID == idleID)
				lyubu.Play(0, LOOP, (float)skip, FALSE, TRUE);
			else
			{
				lyubu.MakeCurrentAction(0, NULL, idleID);
				curActID = idleID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			return ;
		}
		if((walk_state.getState() & STATE_W) != 0)
		{

			float pos_lyubu[3], pos_camera[3], walk[3], walkdist;
			float newpos_lyubu[3], newface_camera[3];
			float height, dist, angle; 
			
			lyubu.GetPosition(pos_lyubu);
			camera.GetPosition(pos_camera);
			height = fabs(pos_camera[2] - pos_lyubu[2]);
			dist = sqrt(pow(pos_camera[0] - pos_lyubu[0],2) + pow(pos_camera[1] - pos_lyubu[1],2));
			angle = acos(height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));

			mf_state = lyubu.MoveForward(10, TRUE, TRUE, 0, TRUE);
			lyubu.GetPosition(newpos_lyubu);
			walkdist = FVector::ComputeDistance(pos_lyubu,newpos_lyubu);
			FVector::Minus(newpos_lyubu, pos_lyubu, walk);

			/* 將抬高的camera降下來 */
			
			FnTerrain terrain;
			float hitPos[3], origin[3] = {pos_camera[0], pos_camera[1], newpos_lyubu[2]};
			float hitDis;
			int camera_result;
			terrain.Object(tID);
			if(height > minHeight)
			{
				
				hit_tag = 1;
				Distance = dist + walkdist;
				float new_height = sqrt(pow(maxDistance,2) + pow(minHeight, 2) - pow(Distance, 2));
				float new_angle = acos(new_height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));
				float axis[3];
				
				// 平移
				if((walk_state.getState() & STATE_A) != 0 || (walk_state.getState() & STATE_D) != 0)
				{
					FVector::Add(pos_camera, walk, pos_camera);
				}
				// 平移 end
				pos_camera[2] = new_height + newpos_lyubu[2];
				FVector::CrossProduct(ludir, cfdir, axis);
				FVector::Minus(newpos_lyubu, pos_camera, cfdir);
				FVector::CrossProduct(cfdir, axis, cudir);
				camera.SetPosition(pos_camera);
				camera.SetDirection(cfdir, cudir);
			}
			else
			{
				hit_tag = 0;
				FVector::Add(walk, pos_camera, pos_camera);
				camera.SetPosition(pos_camera);
			}

			if(curActID != runID)
			{
				lyubu.MakeCurrentAction(0, NULL, runID);
				curActID = runID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			else
				lyubu.Play(0, LOOP, (float)skip, FALSE, TRUE);

		}
		else if((walk_state.getState() & STATE_S) != 0)
		{
			float pos_lyubu[3], pos_camera[3], walk[3], walkdist;
			float newpos_lyubu[3], newface_camera[3];//, newpos_camera[3];
			float height, dist, angle; 
			
			lyubu.GetPosition(pos_lyubu);
			camera.GetPosition(pos_camera);
			height = fabs(pos_camera[2] - pos_lyubu[2]);
			dist = sqrt(pow(pos_camera[0] - pos_lyubu[0],2) + pow(pos_camera[1] - pos_lyubu[1],2));
			angle = acos(height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));

			mf_state = lyubu.MoveForward(10, TRUE, TRUE, 0, TRUE);
			lyubu.GetPosition(newpos_lyubu);
			walkdist = FVector::ComputeDistance(pos_lyubu,newpos_lyubu);
			FVector::Minus(newpos_lyubu, pos_lyubu, walk);

			/* camera撞牆，將camera向上抬 */
			
			FnTerrain terrain;
			float hitPos[3], origin[3] = {pos_camera[0], pos_camera[1], newpos_lyubu[2]};
			float hitDis;
			int camera_result;
			terrain.Object(tID);
			float hitNorm[3];
			if(terrain.HitTest(origin,lfdir,hitPos,FALSE, NULL, hitNorm))
			{
				
				hitDis = FVector::ComputeDistance(origin, hitPos);
				// camera up
				if( hitDis < walkdist){
					hit_tag = 1;
					float axis[3], camwalk[3];

					Distance = sqrt(pow(pos_camera[0] - pos_lyubu[0],2) + pow(pos_camera[1] - pos_lyubu[1],2));
					float new_height = sqrt(pow(maxDistance,2) + pow(minHeight, 2) - pow(Distance, 2));
					float new_angle = acos(new_height / sqrt(pow(maxDistance,2) + pow(minHeight, 2)));
					
					pos_camera[2] = new_height + newpos_lyubu[2];
					FVector::CrossProduct(ludir, cfdir, axis);
					FVector::Minus(newpos_lyubu, pos_camera, cfdir);
					FVector::CrossProduct(cfdir, axis, cudir);
					camera.SetPosition(pos_camera);
					camera.SetDirection(cfdir, cudir);
				}
				else
				{
					hit_tag = 0;
					FVector::Add(walk, pos_camera, pos_camera);
					camera.SetPosition(pos_camera);
				}
			}
			else
			{
				hit_tag = 0;
				FVector::Add(walk, pos_camera, pos_camera);
				camera.SetPosition(pos_camera);
			}

			if(curActID != runID)
			{
				lyubu.MakeCurrentAction(0, NULL, runID);
				curActID = runID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			else
				lyubu.Play(0, LOOP, (float)skip, FALSE, TRUE);
		}
		else
		{
			float dis = 10.0f;
			float rad = (dis/2/Distance) * 180.0/3.1415926 * 2;
			if((walk_state.getState() & STATE_A) == 0)
				rad = -rad;

			// camera 沒被擋住，或camera被擋住但呂布改向另一邊跑才可以改變呂布臉方向
			FVector::Rotate(lfdir, ludir, rad/2, lfdir); // Rotate(src, axis, angle, dst)
			if(hitRotate != 2 || lastRad != rad) lyubu.SetDirection(lfdir, ludir);

			mf_state = lyubu.MoveForward(dis, TRUE, TRUE, 0, TRUE);

			// 旋轉camera
			FVector::Rotate(cfdir, ludir, rad, cfdir);
			FVector::Rotate(cudir, ludir, rad, cudir);

			FVector::Rotate(lfdir, ludir, rad/2, lfdir);
			if(hitRotate != 2 || lastRad != rad) lyubu.SetDirection(lfdir, ludir);

			if(mf_state == -1)
			{
				float pos1[3], pos2[3];
				float camera_old[3], camera_new[3], camera_dir[3];

				lyubu.GetPosition(pos1);
				camera.GetPosition(pos2);
				FVector::Copy(pos2,camera_old);

				FVector::Minus(pos2, pos1, pos2);
				
				// 繞著呂布頭頂轉camera
				FVector::Rotate(pos2, ludir, rad, pos2); 
				FVector::Add(pos1, pos2, pos2);
				FVector::Copy( pos2, camera_new);
				FVector::Minus( camera_new, camera_old , camera_dir);

				// camera旋轉時碰撞偵測與處理
				/*float hitPos[3], origin[3] = {camera_old[0], camera_old[1], pos1[2]+180};
				float hitDis;

				FnTerrain terrain;
				terrain.Object(tID);

				if(terrain.HitTest(origin, camera_dir,hitPos,FALSE))
				{
					hitLen = FVector::ComputeDistance(hitPos,camera_old);

					origin[0] = camera_new[0];
					origin[1] = camera_new[1]; 
					origin[2] = pos1[2];
					cameraLen = FVector::ComputeDistance(pos1, origin);

					if( hitLen < cameraLen) hitRotate = 2;
					else{
						hitRotate = 1;
						camera.SetDirection(cfdir, cudir);
						camera.SetPosition(pos2);
					}

				}
				else
				{*/
					camera.SetDirection(cfdir, cudir);
					camera.SetPosition(pos2);
				//}

			}
			else{
					hitRotate = 0;
					camera.SetDirection(cfdir, cudir);
			}

			lastRad = rad;

			if(curActID != runID)
			{
				lyubu.MakeCurrentAction(0, NULL, runID);
				curActID = runID;
				lyubu.Play(0, START, 0.0f, FALSE, TRUE);
			}
			else
				lyubu.Play(0, LOOP, (float)skip, FALSE, TRUE);
		}
		// simon camera hit
		/*float pos_hit[3], side[3], diff[3];
		float pos_camera[3], pos_lyubu[3];
		FnTerrain terrain;
		terrain.Object(tID);
		
		lyubu.GetDirection(lfdir, ludir);
		camera.GetDirection(cfdir, cudir);
		lyubu.GetPosition(pos_lyubu);
		camera.GetPosition(pos_camera);
		FVector::Minus(pos_camera, pos_lyubu, diff);
		FVector::CrossProduct(cudir, cfdir, side);
		if(terrain.HitTest(pos_lyubu, diff, pos_hit, TRUE))
		{
			hit_tag = 1;
			FVector::Scale(ludir, 50.0f, ludir);
			FVector::Add(pos_camera, ludir, pos_camera);
			FVector::Minus(pos_lyubu, pos_camera, cfdir);
			FVector::GetUnit(cfdir, cfdir);
			FVector::CrossProduct(cfdir, side, cudir);

		}
		else
		{
			hit_tag = 0;
			float tmp_pos_camera[3], tmp_cudir[3], tmp_cfdir[3];
			
			FVector::Scale(ludir, -50.0f, ludir);
			FVector::Add(pos_camera, ludir, tmp_pos_camera);
			FVector::Minus(pos_lyubu, tmp_pos_camera, tmp_cfdir);
			FVector::GetUnit(tmp_cfdir, tmp_cfdir);
			FVector::CrossProduct(tmp_cfdir, side, tmp_cudir);
			
			FVector::Minus(tmp_pos_camera, pos_lyubu, diff);
			float tmp[3];
			FVector::CrossProduct(side, ludir, tmp);
			float tangle = FVector::Angle(tmp, diff);
			
			if(FVector::Magnitude(diff)*sin(tangle*3.1415926/180) > minHeight &&
				!terrain.HitTest(pos_lyubu, diff, pos_hit, TRUE) )
			{
				FVector::Copy(tmp_pos_camera, pos_camera);
				FVector::Copy(tmp_cudir, cudir);
				FVector::Copy(tmp_cfdir, cfdir);
			}
		}
		camera.SetWorldPosition(pos_camera);
		camera.SetWorldDirection(cfdir, cudir);*/
	}

	return;
}

void Keydown(WORLDid gID, BYTE code, BOOL value)
{
	switch(code)
	{
		case FY_J:
			if(value)
				walk_state.addState(STATE_ATT1);
			break;
		case FY_K:
			if(value)
				walk_state.addState(STATE_ATT2);
			break;
		case FY_L:
			if(value)
				walk_state.addState(STATE_ATT3);
			break;
		case FY_I:
			if(value)
				walk_state.addState(STATE_ATT4);
			break;
		case FY_W:
		case FY_UP:
			turn_state = TRUE;
			if(value)
				walk_state.addState(STATE_W);
			else
				walk_state.removeState(STATE_W);
			break;
		case FY_A:
		case FY_LEFT:
			turn_state = TRUE;
			if(value)
				walk_state.addState(STATE_A);
			else
				walk_state.removeState(STATE_A);
			break;
		case FY_S:
		case FY_DOWN:
			turn_state = TRUE;
			if(value)
				walk_state.addState(STATE_S);
			else
				walk_state.removeState(STATE_S);
			break;
		case FY_D:
		case FY_RIGHT:
			turn_state = TRUE;
			if(value)
				walk_state.addState(STATE_D);
			else
				walk_state.removeState(STATE_D);
			break;
	}

	switch(walk_state.getState())
	{
		case STATE_W:
			turn_target = 0;
			break;
		case STATE_A:
			turn_target = 270;
			break;
		case STATE_S:
			turn_target = 180;
			break;
		case STATE_D:
			turn_target = 90;
			break;
		case (STATE_W | STATE_A):
			turn_target = 315;
			break;
		case (STATE_W | STATE_D):
			turn_target = 45;
			break;
		case (STATE_S | STATE_D):
			turn_target = 135;
			break;
		case (STATE_S | STATE_A):
			turn_target = 225;
			break;
		case (STATE_A | STATE_D):
			turn_target = 270;
			break;
	}
}

void RenderFunc(int skip)
{
	FnViewport vp;
	vp.Object(vID);
	vp.Render(cID, TRUE, TRUE);

	FnWorld gw;
	gw.Object(gID);
	
	// Show Message
	/*
	char msg[200];
	FnActor lyubu;
	FnCamera camera;
	lyubu.Object(lyubuID);
	camera.Object(cID);
	float lpos[3], cpos[3], lfdir[3], ludir[3];
	lyubu.GetPosition(lpos);
	lyubu.GetDirection(lfdir, ludir);
	camera.GetPosition(cpos);
	
	gw.StartMessageDisplay();
	sprintf(msg, "Lyubu: %.03f %.03f %.03f\nCamera: %.03f %.03f %.03f", lpos[0], lpos[1], lpos[2], cpos[0], cpos[1], cpos[2]);
	gw.MessageOnScreen(10, 10, msg, 255, 255, 255);
	sprintf(msg, "Lyubu face: %.03f %.03f %.03f\nLyubu up: %.03f %.03f %.03f", lfdir[0], lfdir[1], lfdir[2], ludir[0], ludir[1], ludir[2]);
	gw.MessageOnScreen(10, 50, msg, 255, 255, 255);
	camera.GetDirection(lfdir, ludir);
	sprintf(msg, "Camera face: %.03f %.03f %.03f\nCamera up: %.03f %.03f %.03f", lfdir[0], lfdir[1], lfdir[2], ludir[0], ludir[1], ludir[2]);
	gw.MessageOnScreen(10, 90, msg, 255, 255, 255);
	sprintf(msg, "State = %d, Turn = %d, MF = %d, Turn_target = %.02f, Angle = %.02f, Hit tag:%d, Hit rotate:%d, cameraLen= %f, hitLen = %f", walk_state, turn_state, mf_state, turn_target, angle,  hit_tag, hitRotate, cameraLen, hitLen);
	gw.MessageOnScreen(10, 130, msg, 255, 255, 255);

	gw.FinishMessageDisplay();
	*/
	// Show Message End
	
	gw.SwapBuffers();
}