
#include "Core.h"
#include "UnCore.h"
#include "AnimationRuntime.h"

const TArray<FCompactPose*>& FAnimationRuntime::LoadAsPoses(const CAnimSequence* seq, const USkeleton* Skeleton)
{

	TArray<FCompactPose*>* poses = new TArray<FCompactPose*>();
	for (int frameIndex = 0; frameIndex < seq->NumFrames; frameIndex++)
	{
		FCompactPose * CurrentPose = new FCompactPose(Skeleton->BoneTree.Num());
		poses->Add(CurrentPose);
		CurrentPose->Bones.SetNum(Skeleton->BoneTree.Num());
		for (int boneIndex = 0; boneIndex < CurrentPose->Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Skeleton->ReferenceSkeleton.RefBoneInfo[boneIndex];
			CAnimTrack* track = seq->Tracks[boneIndex];
			CVec3 bonePositionRef;
			CQuat boneOrientationRef;
			bonePositionRef.Set(0, 0, 0);
			boneOrientationRef.Set(0, 0, 0, 1);
			track->GetBonePosition(frameIndex, seq->NumFrames, false, bonePositionRef, boneOrientationRef);
			FVector BonePosUse = FVector(bonePositionRef.X, bonePositionRef.Y, bonePositionRef.Z);
			FQuat BoneRotUse = FQuat(boneOrientationRef.X, boneOrientationRef.Y, boneOrientationRef.Z, boneOrientationRef.W);
			// Create PoseBone
			FTransform BoneTransform = FTransform(BonePosUse, BoneRotUse, FVector());
			FPoseBone& poseBone = CurrentPose->Bones[boneIndex];
			poseBone.Name = boneInfo.Name;
			poseBone.ParentIndex = boneInfo.ParentIndex;
			poseBone.Transform = BoneTransform;
			poseBone.IsValidKey = frameIndex <= (track->KeyPos.Num() < track->KeyQuat.Num() ? track->KeyPos.Num() : track->KeyQuat.Num());
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
	TArray<FCompactPose*>* Poses = new TArray<FCompactPose*>();
	UAnimSequence4* animSequence4 = static_cast<UAnimSequence4*>(const_cast<UObject*>(seq->OriginalSequence));
	for (int frameIndex = 0; frameIndex < 1; frameIndex++)
	{
		FCompactPose* CurrentPose = new FCompactPose(Skeleton->BoneTree.Num());
		Poses->Add(CurrentPose);
		CurrentPose->Bones.SetNum(Skeleton->BoneTree.Num());
		for (int boneIndex = 0; boneIndex < CurrentPose->Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Skeleton->ReferenceSkeleton.RefBoneInfo[boneIndex];
			FTransform originalTransform = Skeleton->ReferenceSkeleton.RefBonePose[boneIndex];
			CAnimTrack* Track = seq->Tracks[boneIndex];
			CVec3 bonePositionRef;
			CQuat boneOrientationRef;
			bonePositionRef.Set(0, 0, 0);
			boneOrientationRef.Set(0, 0, 0, 1);
			Track->GetBonePosition(frameIndex, seq->NumFrames, false, bonePositionRef, boneOrientationRef);

			FVector BonePosUse = FVector(bonePositionRef.X, bonePositionRef.Y, bonePositionRef.Z);
			FQuat BoneRotUse = FQuat(boneOrientationRef.X, boneOrientationRef.Y, boneOrientationRef.Z, boneOrientationRef.W);
			FTransform BoneTransform = FTransform(BonePosUse, BoneRotUse, FVector());
			// Creating PoseBone
			FPoseBone PoseBone;
			PoseBone.Name = boneInfo.Name;
			PoseBone.ParentIndex = boneInfo.ParentIndex;
			PoseBone.Transform = BoneTransform;
			PoseBone.IsValidKey = true;
			(*Poses)[frameIndex]->Bones[boneIndex] = PoseBone;
		}
	}

	return *Poses;
}

TArray<FCompactPose*>& FAnimationRuntime::LoadRestAsPoses(USkeleton* Skel)
{
	TArray<FCompactPose*>* Poses = new TArray<FCompactPose*>();
	Poses->SetNum(1);
	for (int frameIndex = 0; frameIndex < Poses->Num(); frameIndex++)
	{
		FCompactPose* Pose = new FCompactPose(Skel->BoneTree.Num());
		Poses->Add(Pose);
		Pose->Bones.SetNum(Skel->BoneTree.Num());
		for (int boneIndex = 0; boneIndex < (*Poses)[frameIndex]->Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Skel->ReferenceSkeleton.RefBoneInfo[boneIndex];
			FPoseBone PoseBone;
			PoseBone.Name = boneInfo.Name;
			PoseBone.ParentIndex = boneInfo.ParentIndex;
			PoseBone.Transform = Skel->ReferenceSkeleton.RefBonePose[boneIndex].Clone();
			PoseBone.IsValidKey = true;
			(*Poses)[frameIndex]->Bones[boneIndex] = PoseBone;

		}
	}
	return *Poses;
}



