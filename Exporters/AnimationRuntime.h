#pragma once

#include "UnObject.h"
#include "Mesh/SkeletalMesh.h"
#include "UnrealMesh/UnMesh4.h"
struct FPoseBone
{
public:
	FTransform Transform{};
	int32 ParentIndex;
	FString Name;
	bool IsValidKey;
	bool Accumulated;
	FPoseBone()
	{
		Transform = FTransform();
		ParentIndex = -1;
		Name = "";
		IsValidKey = false;
		Accumulated = false;
	}
	FPoseBone Clone() const
	{
		FPoseBone NewBone;
		NewBone.Transform = Transform.Clone();
		NewBone.ParentIndex = ParentIndex;
		NewBone.Name = Name;
		NewBone.IsValidKey = IsValidKey;
		NewBone.Accumulated = Accumulated;
		return NewBone;
	}

	void  AccumulateWithAdditiveScale(FTransform atom, float weight)
	{
		FVector OneVec = FVector(1, 1, 1);
		Transform.Rotation = atom.Rotation * weight * Transform.Rotation;
		Transform.Translation += atom.Translation * weight;
		OneVec.Add(atom.Scale3D);
		Transform.Scale3D *= OneVec * weight;
		Accumulated = true;
	}
};

struct FCompactPose
{
public:
	TArray<FPoseBone> Bones;
	int AnimFrame;
	bool Processed;

	FCompactPose(const FReferenceSkeleton& refSkel)
	{
		Bones.SetNum(refSkel.RefBoneInfo.Num());
		AnimFrame = 0;
		Processed = false;

		for (int i = 0; i < Bones.Num(); ++i)
		{
			FPoseBone NewBone;
			Bones[i] = NewBone;
		}
	}

	FCompactPose() : AnimFrame(0), Processed(false)
	{
	}

	FCompactPose(int boneLength) : AnimFrame(0), Processed(false)
	{
		Bones.SetNum(boneLength);

		for (int i = 0; i < Bones.Num(); ++i)
		{
			FPoseBone NewBone;
			Bones[i] = NewBone;
		}
	}



	void NormalizeRotations()
	{
		for (FPoseBone& bone : Bones)
		{
			bone.Transform.Rotation.Normalize();
		}
	}

	void PushTransformAtFrame(TArray<CAnimTrack*>& tracks, int frame)
	{
		assert(tracks.Num() == Bones.Num());

		for (int32 Index = 0; Index < Bones.Num(); ++Index)
		{
			if (!Bones[Index].IsValidKey) continue;

			FTransform Transform = Bones[Index].Transform;
			CQuat QuatFrameRot;
			QuatFrameRot.Set(Transform.Rotation.X, Transform.Rotation.Y, Transform.Rotation.Z, Transform.Rotation.W);
			tracks[Index]->KeyQuat[frame] = QuatFrameRot;
			tracks[Index]->KeyPos[frame] = Transform.Translation;
			tracks[Index]->KeyScale[frame] = Transform.Scale3D;
		}
	}
	FCompactPose(const FCompactPose& other)
	{
		for (const FPoseBone& Bone : other.Bones)
		{
			Bones.Add(Bone);
		}
		AnimFrame = other.AnimFrame;
		Processed = other.Processed;
	}
	FCompactPose* Clone() const
	{
		FCompactPose* ClonePose = new FCompactPose(Bones.Num());
		for (int i = 0; i < ClonePose->Bones.Num(); i++)
		{
			ClonePose->Bones[i] = Bones[i].Clone();
		}
		return ClonePose;
	}

};



class CAnimSet;

class FAnimationRuntime
{
public:
	static TArray<FCompactPose*>& LoadRestAsPoses(USkeleton* Skel);
	static const TArray<FCompactPose*>& LoadAsPoses(const CAnimSequence* Anim, const USkeleton* Skeleton, const int refFrame);
	static const TArray<FCompactPose*>& LoadAsPoses(const CAnimSequence* Anim, const USkeleton* Skeleton);
	static void ConvertMeshRotationPoseToLocalSpaceV2(FCompactPose* pose);
	static void AccumulateLocalSpaceAdditivePoseInternal(FCompactPose* basePose, FCompactPose* additivePose, float weight);
	static void AccumulateMeshSpaceRotationAdditiveToLocalPoseInternal(FCompactPose* basePose, FCompactPose* additivePose, float weight);
	static void ConvertPoseToMeshRotation(FCompactPose* localPose);



};