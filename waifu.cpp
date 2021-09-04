#include "waifu.h"
#include "window.h"
#include <time.h>

static float GetTheta(Quaternion &qa, Quaternion &qb, const Vector3 &n){

	qa.normalize();
	qb.normalize();

	// Vector3 n = {0,1,0};

	Vector3 vec1 = qa * n;
	Vector3 vec2 = qb * n;

	vec1.normalize();
	vec2.normalize();

	float ret = acos(vec1.dot(vec2));

	if(isnan(ret)) return 0;

	if(vec1.cross(vec2).dot(n) < 0)
		ret = -ret;

	return ret;
}

void LookAtMotion::Reset(){

	lastTime = Window_GetTicks();

	theta = GetTheta(startingRot, finalRot, up);

    timeInto = dampingVel = dampingX = 0;

	bone->overrideRotation = true;

	currRot = startingRot;
}

int LookAtMotion::Update(){

	// const float minVal = 0.00000001;

	// if(theta == 0 && (fabs(dampingVel) < minVal || isnan(dampingVel))) return 1;

	// int dt = Window_GetTicks() - lastTime;
	// if(!dt) return 0;

	// lastTime = Window_GetTicks();

	// dampingX = GetTheta(finalRot, currRot, up);

	// if(isnan(dampingX)) return 0;

	// // float force = (-(springConst*dampingX) - (dampingConst*dampingVel)) * dt;
	// float force = (-(springConst*dampingX));// - (dampingConst*dampingVel)) * dt;

	// if(isnan(force)) return 0;

	// dampingVel += force;

	// if(fabs(dampingVel) < minVal || isnan(dampingVel)){
	// 	float matrix[16];
	// 	Math_MatrixFromQuat(finalRot, matrix);
	// 	Math_CopyMatrix(bone->rotMatrix, matrix);
	// 	lastTime = -1;
	// 	bone->overrideRotation = false;
	// 	return 1;
	// }

	// float toAdd = (GetTheta(startingRot, currRot, up) + (force * dt));

	// currRot = Math_Slerp(startingRot, finalRot, (toAdd/theta));

	if(lastTime == -1) Reset();

	int dt = Window_GetTicks() - lastTime;
	lastTime = Window_GetTicks();

	timeInto += (dt * springConst);

	if(timeInto >= 1){
		if(returnAfterReached){
			float matrix[16];
			Math_MatrixFromQuat(finalRot, matrix);
			Math_CopyMatrix(bone->rotMatrix, matrix);
			lastTime = -1;
			bone->overrideRotation = false;
			return 1;
		}
		return 0;
	}

	currRot = Math_Slerp(startingRot, finalRot, timeInto);
	bone->currRot = currRot;

	float matrix[16];
	Math_MatrixFromQuat(currRot, matrix);
	Math_CopyMatrix(bone->rotMatrix, matrix);

	return 0;
}

LookAtMotion *Waifu::GetLookAt(const std::string &boneName){

	Bone *bone = meshInstance.GetBoneFromName(boneName);

	for(int k = 0; k < (int)lookAts.size(); k++)
		if(lookAts[k].bone == bone)
			return &lookAts[k];

	return nullptr;
}

void Waifu::UpdateLookAts(){

	for(int k = 0; k < (int)lookAts.size(); k++){
		if(lookAts[k].Update() == 1){

			lookAts.erase(lookAts.begin() + k);
			k--;
		}
	}
}

Vector3 Waifu::GetBonePos(const std::string &boneName){

	Bone *bone = meshInstance.GetBoneFromName(boneName);

	if(!bone) return {0,0,0};

	float matrix[16];
	Math_RotateMatrix(matrix, eulerAngles);
	Math_MatrixMatrixMult(matrix, matrix, bone->absMatrix);

	return Math_MatrixMult({0,0,0}, matrix) + waifuPos;
}

void Waifu::LookAt(const std::string &boneName, Quaternion &qa, Quaternion &qb, Vector3 up, float dmp, float spr,
		Quaternion &startingRot, float maxAngle){

	Bone *bone = meshInstance.GetBoneFromName(boneName);

	if(!bone) return;

	Vector3 forwardW = {0,0,-1};
	up = forwardW.cross(up);

	float theta = GetTheta(qa, qb, up);

	if(theta == 0)
		return;

	theta = fabs(GetTheta(startingRot, qb, up));

	if(theta > fabs(maxAngle))
		return;
		// qb = Math_Slerp(startingRot, qb, maxAngle/theta);

	for(int k = 0; k < (int)lookAts.size(); k++){
		if(lookAts[k].bone == bone){
			if(GetTheta(qa, qb, up) == 0) return;
			lookAts[k].up = up;
			lookAts[k].theta = GetTheta(qa, qb, up);
		    lookAts[k].startingRot = qa;
		    lookAts[k].currRot = qa;
		    lookAts[k].finalRot = qb;
		    lookAts[k].dampingConst = dmp;
		    lookAts[k].springConst = spr;
		    lookAts[k].timeInto = 0;
			return;
		}
	}

	lookAts.push_back({up, qa, qb, bone, dmp, spr});
}

void Waifu::LookAt(const std::string &boneName, const Vector3 &target, Vector3 up,  float dmp, float spr,
		Quaternion &startingRot, float maxAngle){

	Bone *bone = meshInstance.GetBoneFromName(boneName);

	if(!bone) return;

	Quaternion qa = Math_MatrixToQuat(bone->absRotationMatrix);

	Vector3 bonePos = GetBonePos(boneName);

    Vector3 forward = (target - bonePos).normalize();

	Quaternion qb = Math_QuatLookAt(forward, up);

	LookAt(boneName, qa, qb, up, dmp, spr, startingRot, maxAngle);
}

void Waifu::RemoveLookAt(const std::string &boneName, float speed){
	Bone *bone = meshInstance.GetBoneFromName(boneName);
	for(int k = 0; k < (int)lookAts.size(); k++){
		if(lookAts[k].bone == bone){
			bone->currRot = lookAts[k].currRot;
			bone->overrideRotation = false;
			bone->slerpSpeed = speed;
			bone->startSlerpTime = Window_GetTicks();
			bone->overrideRotation = false;

			if(bone->parent)
				bone->currRot *= Math_MatrixToQuat(bone->parent->absRotationMatrix).inverse();

			lookAts.erase(lookAts.begin() + k);
			return;
		}
	}
}

void Waifu::UpdateIK(){

	// // object is rotated in modelspace too

	// static std::vector<Bone *> bones = {
	// 	meshInstance.GetBoneFromName("Bone.008"),
	// 	meshInstance.GetBoneFromName("Bone.009"),
	// 	meshInstance.GetBoneFromName("Bone.010")
	// };

	// Vector3 bonePositions[3];
	// bonePositions[0] = GetBonePos("Bone.008");
	// bonePositions[1] = GetBonePos("Bone.009");
	// bonePositions[2] = GetBonePos("Bone.010");

	// static std::shared_ptr<Joint> joint1 = std::shared_ptr<Joint>(
	// 	new Joint(bonePositions[0], bonePositions[1], bones[0]->currRot, nullptr, (bonePositions[1]-bonePositions[0]).magnitude()));
	// static std::shared_ptr<Joint> joint2 = std::shared_ptr<Joint>(
	// 	new Joint(bonePositions[1], bonePositions[2], bones[1]->currRot, joint1.get(), (bonePositions[2]-bonePositions[1]).magnitude()));

	// static std::vector<std::shared_ptr<Joint>> joints = { joint1, joint2 };

	// static Arm arm = Arm(bonePositions[0], joints);

	// // arm.Reset();
	// arm.base = bonePositions[0];

	// Vector3 goal = oHandler->GetPosition() + (oHandler->GetForward() * ((oHandler->GetPosition()-arm.base).magnitude()*0.5));
	// // goal = oHandler->GetPosition() + (Vector3){0,-0.35,0} + (oHandler->GetUp().cross(oHandler->GetForward()) * -0.2);

	// arm.Solve(goal);

	// for(int k = 0; k < (int)arm.joints.size(); k++){
	// 	bones[k]->overrideRotation = true;
	// 	float matrix[16];
	// 	Math_MatrixFromQuat(arm.joints[k]->T, matrix);
	// 	Math_CopyMatrix(bones[k]->rotMatrix, matrix);
	// }
}

void Waifu::CheckIfBeingLookedAt(){

	Vector3 forward = oHandler->GetForward().normalize();
	Vector3 vec = (oHandler->GetPosition() - GetBonePos("Head"));
	Vector3 fromPos = vec.normalize();
	float dist = vec.magnitude();

	float theta = acos(forward.dot(fromPos));


	if(theta > PI/1.5 || dist < eyeCloseDistance)
		isBeingLookedAt = true;
	else
		isBeingLookedAt = false;

}

void Waifu::LookAtPlayer(){

	float dist = (GetBonePos("Head") - oHandler->GetPosition()).magnitude();

	if(dist < eyeCloseDistance){

		meshInstance.SetOverrideMaterialAnim("Kissing");

	} else {

		meshInstance.SetOverrideMaterialAnim("Template");

		Quaternion maxRot = meshInstance.GetBoneFromName("Head")->startingAbsRot;
		LookAt("Head", oHandler->GetPosition(), {0,1,0}, 0.007, 0.005, maxRot, PI/2);

		// maxRot = meshInstance.GetBoneFromName("SpineLow")->startingAbsRot;

		// Vector3 chestVec = oHandler->GetPosition();
		// chestVec.y = GetBonePos("SpineLow").y;
		// LookAt("SpineLow", chestVec, {0,1,0}, 0.007, 0.001, maxRot, PI/3);
	}
}

void Waifu::LookAwayFromPlayer(){

	float dist = (GetBonePos("Head") - oHandler->GetPosition()).magnitude();

	if(dist < eyeCloseDistance) return;

	RemoveLookAt("Head");
	// RemoveLookAt("SpineLow");

	lookingAtPlayer = false;
}

void Waifu::PlayAnimation(const std::string &name, bool loop){
	meshInstance.StopAnimation(playingAnim);
	playingAnim = name;
	meshInstance.StartAnimation(playingAnim, 1, loop);
	meshInstance.SetOverrideMaterialAnim(playingAnim);
}

void Waifu::SetNextSleepTime(){
	srand(clock());
	float nextTime = (rand()%(60000*10))+(60000*5);
	nextSleepTime = Window_GetTicks() + nextTime;
}

void NeptuneWaifu::Init(OculusHandler *o){
	nextLookAtTime = Window_GetTicks() + 1000;
	nextSleepTime = Window_GetTicks() + 20000;
	oHandler = o;
	mesh.LoadSkel("Resources/Neptune/Nep.skl");
	mesh.LoadMesh("Resources/Neptune/Nep.yuk");
	mesh.LoadAnim("Resources/Neptune/Nep_SleepingOne.anm", "Sleeping");
	mesh.LoadAnim("Resources/Neptune/Nep_Kissing.anm", "Kissing");
	mesh.LoadAnim("Resources/Neptune/Nep_SleepStartOne.anm", "SleepingStart");
	mesh.LoadAnim("Resources/Neptune/Nep_YawnOneRough.anm", "Yawn");
	mesh.LoadAnim("Resources/Neptune/Nep_SleepEndOneRough.anm", "SleepEnd");
	mesh.LoadAnim("Resources/Neptune/Nep_IdleActionOne.anm", "IdleAction");
	mesh.LoadAnim("Resources/Neptune/Nep_HeartRough.anm", "Heart");
	mesh.LoadAnim("Resources/Neptune/Nep_Template.anm", "Template");
	mesh.LoadAnim("Resources/Neptune/Nep_FingerPushRough.anm", "FingerPush");
	meshInstance = RiggedMeshInstance(&mesh);
	PlayAnimation("Template");
	meshInstance.Update();
}

void NeptuneWaifu::Draw(const Vector3 &lightDir, bool useShadows){
	Shaders_UseProgram(SKELETAL_ANIMATION_SHADER);

	Shaders_SetUniformColor({color.x,color.y,color.z,0.5});
	Shaders_UpdateViewMatrix();
	Shaders_UpdateProjectionMatrix();
	Shaders_UpdateDepthMvpMatrix();
	Shaders_SetLightInvDir({-lightDir.x, -lightDir.y, -lightDir.z});

	if(!useShadows) Shaders_EnableShadows();
	else Shaders_DisableShadows();

	meshInstance.Translate(waifuPos);
	meshInstance.Rotate(eulerAngles);
	meshInstance.UpdateMatrix();
	meshInstance.Draw();
}

void NeptuneWaifu::Update(){

	const int timeBeforeLook = 1000;

	UpdateLookAts();

	meshInstance.Update();

	CheckIfBeingLookedAt();

	if(isBeingLookedAt && Window_GetTicks() - nextSleepTime > 0){

		float dist = (GetBonePos("Head") - oHandler->GetPosition()).magnitude();

		if(dist > eyeCloseDistance || !lookingAtPlayer){
			sleeping = true;
			PlayAnimation("Yawn");
			SetNextSleepTime();

			if(lookingAtPlayer) LookAwayFromPlayer();
		}
	}

	// if(isBeingLookedAt && !sleeping && Window_GetTicks() - nextLookAtTime > 0)
	if(isBeingLookedAt && !lookingAtPlayer && !sleeping){
		lookingAtPlayerStartTime = Window_GetTicks();
		lookingAtPlayer = true;
	}

	if(lookingAtPlayer && Window_GetTicks() - lookingAtPlayerStartTime > timeBeforeLook)
		LookAtPlayer();

	if(isBeingLookedAt == false && lookingAtPlayer){

		LookAwayFromPlayer();

		if(Window_GetTicks() - nextSleepTime > 0)
			nextSleepTime = Window_GetTicks() + 20000;
	}

	const int maxSleeps = 5;

	if(meshInstance.AnimationEnded(playingAnim)){
		if(playingAnim == "Yawn")
			PlayAnimation("SleepingStart");
		else if(playingAnim == "SleepingStart")
			PlayAnimation("Sleeping");
		// else if(playingAnim == "Template")
			// PlayAnimation("Template");
		else if(playingAnim == "Sleeping"){
			if(sleepPlayCount++ < maxSleeps){
				meshInstance.StartAnimation(playingAnim, 1, false);
			} else {
				sleepPlayCount = 0;
				PlayAnimation("SleepEnd");
			}
		} else if(playingAnim == "SleepEnd"){

			sleeping = false;
			PlayAnimation("Template");
		}
	}
}