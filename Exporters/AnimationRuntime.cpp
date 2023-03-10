
#include "Core.h"
#include "UnCore.h"
#include "AnimationRuntime.h"

const TArray<FCompactPose*>& FAnimationRuntime::LoadAsPoses(const CAnimSequence* seq, const USkeleton* Skeleton)
{
	TArray<FCompactPose*>* poses = new TArray<FCompactPose*>();
	//poses->SetNum(seq->NumFrames);
	for (int frameIndex = 0; frameIndex < seq->NumFrames; frameIndex++)
	{
		//FCompactPose& CpactPose = (*poses)[frameIndex];
		FCompactPose * CpactPose = new FCompactPose(Skeleton->BoneTree.Num());
		poses->Add(CpactPose);
		//CpactPose = (*poses)[frameIndex];
		CpactPose->Bones.SetNum(Skeleton->BoneTree.Num());
		for (int boneIndex = 0; boneIndex < CpactPose->Bones.Num(); boneIndex++)
		{
			//
			FTransform NewTran;
			FMeshBoneInfo boneInfo = Skeleton->ReferenceSkeleton.RefBoneInfo[boneIndex];
			CAnimTrack* track = seq->Tracks[boneIndex];
			CVec3 bonePosition;
			CQuat boneOrientation;
			bonePosition.Set(0, 0, 0);
			boneOrientation.Set(0, 0, 0, 1);
			//
			FVector BonePosUse;
			FQuat BoneRotUse;
			FVector BoneScaleUse;
			track->GetBonePosition(frameIndex, seq->NumFrames, false, bonePosition, boneOrientation);
			//
			BonePosUse.Set(bonePosition.X, bonePosition.Y, bonePosition.Z);
			BoneRotUse.Set(boneOrientation.X, boneOrientation.Y, boneOrientation.Z, boneOrientation.W);
			BoneScaleUse.Set(0, 0, 0);
			//
			NewTran.Translation = BonePosUse;
			NewTran.Rotation = BoneRotUse;
			NewTran.Scale3D = BoneScaleUse;
			FPoseBone& tert = CpactPose->Bones[boneIndex];
			tert.Name = boneInfo.Name;
			tert.ParentIndex = boneInfo.ParentIndex;
			tert.Transform = NewTran;
			tert.IsValidKey = frameIndex <= (track->KeyPos.Num() < track->KeyQuat.Num() ? track->KeyPos.Num() : track->KeyQuat.Num());
		}
	}
	return *poses;
}

void FAnimationRuntime::AccumulateLocalSpaceAdditivePoseInternal(FCompactPose* basePose, FCompactPose* additivePose, float weight)
{
	if (weight < 0.999989986419678)
	{
		return;
	}

	for (int index = 0; index < basePose->Bones.Num(); index++)
	{
		FTransform Beka = basePose->Bones[index].Transform;
		basePose->Bones[index].AccumulateWithAdditiveScale(additivePose->Bones[index].Transform, weight);
	}
}

void FAnimationRuntime::ConvertMeshRotationPoseToLocalSpaceV2(FCompactPose* pose)
{
	for (int boneIndex = pose->Bones.Num() - 1; boneIndex > 0; --boneIndex)
	{
		int parentIndex = pose->Bones[boneIndex].ParentIndex;
		FQuat localSpaceRotation = pose->Bones[parentIndex].Transform.Rotation.Inverse() * pose->Bones[boneIndex].Transform.Rotation;
		pose->Bones[boneIndex].Transform.Rotation = localSpaceRotation;
	}
}

void FAnimationRuntime::AccumulateMeshSpaceRotationAdditiveToLocalPoseInternal(FCompactPose* basePose, FCompactPose* additivePose, float weight)
{
	// base pose is not getting modified on functions
	FTransform idek = basePose->Bones[7].Transform;
	ConvertPoseToMeshRotation(basePose);
	AccumulateLocalSpaceAdditivePoseInternal(basePose, additivePose, weight);
	ConvertMeshRotationPoseToLocalSpaceV2(basePose);
}

void FAnimationRuntime::ConvertPoseToMeshRotation(FCompactPose* localPose)
{
	for (int boneIndex = 1; boneIndex < localPose->Bones.Num(); ++boneIndex)
	{
		int parentIndex = localPose->Bones[boneIndex].ParentIndex;
		FQuat meshSpaceRotation = localPose->Bones[parentIndex].Transform.Rotation * localPose->Bones[boneIndex].Transform.Rotation;
		localPose->Bones[boneIndex].Transform.Rotation = meshSpaceRotation;
	}
}

const TArray<FCompactPose*>& FAnimationRuntime::LoadAsPoses(const CAnimSequence* seq, const USkeleton* Skeleton, const int refFrame)
{
	//TArray<FCompactPose>* Poses = new TArray<FCompactPose>();
	TArray<FCompactPose*>* Poses = new TArray<FCompactPose*>();
	//Poses->SetNum(1);
	int idek = 1;
	for (int frameIndex = 0; frameIndex < idek; frameIndex++)
	{
		//FCompactPose& CpactPose = (*Poses)[frameIndex];
		FCompactPose* CpactPose = new FCompactPose(Skeleton->BoneTree.Num());
		Poses->Add(CpactPose);
		//CpactPose = (*Poses)[frameIndex];
		CpactPose->Bones.SetNum(Skeleton->BoneTree.Num());

		for (int boneIndex = 0; boneIndex < CpactPose->Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Skeleton->ReferenceSkeleton.RefBoneInfo[boneIndex];
			FTransform originalTransform = Skeleton->ReferenceSkeleton.RefBonePose[boneIndex];
			// getting correct transform.
			CAnimTrack* Track = seq->Tracks[boneIndex];
			CVec3 bonePosition;
			CQuat boneOrientation;
			bonePosition.Set(0, 0, 0);
			boneOrientation.Set(0, 0, 0, 1);
			Track->GetBonePosition(refFrame, seq->NumFrames, false, bonePosition, boneOrientation);

			//
			FVector Translation;
			Translation.Set(bonePosition.X, bonePosition.Y, bonePosition.Z);
			FQuat RotationF;
			RotationF.Set(boneOrientation.X, boneOrientation.Y, boneOrientation.Z, boneOrientation.W);
			// getting correct pos everything
			FTransform BoneTransform;
			BoneTransform.Translation = Translation;
			BoneTransform.Rotation = RotationF;

			//
			FPoseBone toct;
			toct.Name = boneInfo.Name;
			toct.ParentIndex = boneInfo.ParentIndex;
			toct.Transform = BoneTransform;
			toct.IsValidKey = true;
			(*Poses)[frameIndex]->Bones[boneIndex] = toct;
		}
	}

	return *Poses;
}

TArray<FCompactPose*>& FAnimationRuntime::LoadRestAsPoses(USkeleton* Skel)
{
	TArray<FCompactPose*>* Poses = new TArray<FCompactPose*>();
	Poses->SetNum(1);
	int onl = 1;
	for (int frameIndex = 0; frameIndex < onl; frameIndex++)
	{
		//FCompactPose& Pose = (*Poses)[frameIndex];
		FCompactPose* Pose = new FCompactPose(Skel->BoneTree.Num());
		Poses->Add(Pose);
		//Pose = (*Poses)[frameIndex];
		Pose->Bones.SetNum(Skel->BoneTree.Num());
		for (int boneIndex = 0; boneIndex < (*Poses)[frameIndex]->Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Skel->ReferenceSkeleton.RefBoneInfo[boneIndex];
			FPoseBone taqt;
			taqt.Name = boneInfo.Name;
			taqt.ParentIndex = boneInfo.ParentIndex;
			taqt.Transform = Skel->ReferenceSkeleton.RefBonePose[boneIndex].Clone();
			taqt.IsValidKey = true;
			(*Poses)[frameIndex]->Bones[boneIndex] = taqt;

		}
	}
	return *Poses;
}



