
#include "Core.h"
#include "UnCore.h"
#include "AnimationRuntime.h"



const TArray<FCompactPose*>& FAnimationRuntime::LoadAsPoses(const CAnimSet* Anim)
{
	TArray<FCompactPose*> Poses;
	//TArray<FCompactPose*> Poses = new TArray<FCompactPose>();
	CAnimSequence* seq = Anim->Sequences[0];
	//Poses.Reserve(seq->NumFrames);
	Poses.SetNum(seq->NumFrames);
	for (int frameIndex = 0; frameIndex < seq->NumFrames; frameIndex++)
	{
		FCompactPose* CpactPose = new FCompactPose(Anim->BonePositions.Num());
		Poses[frameIndex] = CpactPose;
		for (int boneIndex = 0; boneIndex < Poses[frameIndex]->Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Anim->TrackBonesInfo[boneIndex];
			CAnimTrack* track = seq->Tracks[boneIndex];
			CVec3 BP;
			CQuat BO;
			BP.Set(0, 0, 0);			// GetBonePosition() will not alter BP and BO when animation tracks are not exists
			BO.Set(0, 0, 0, 1);
			track->GetBonePosition(frameIndex, seq->NumFrames, false, BP, BO);

			//
			FTransform NewTran;
			FVector TestVec;
			TestVec.Set(BP.X, BP.Y, BP.Z);
			NewTran.Translation = TestVec;
			//
			FQuat TestRot;
			TestRot.Set(BO.X, BO.Y, BO.Z, BO.W);
			NewTran.Rotation = TestRot;
			//
			FPoseBone* tert = new FPoseBone();
			tert->Name = boneInfo.Name;
			tert->ParentIndex = boneInfo.ParentIndex;
			tert->Transform = NewTran;

			tert->IsValidKey = frameIndex <= (track->KeyPos.Num() < track->KeyQuat.Num() ? track->KeyPos.Num() : track->KeyQuat.Num());

			Poses[frameIndex]->Bones[boneIndex] = tert;

		}
		//Poses[frameIndex] = CpactPose;
	}

	return Poses;
}
