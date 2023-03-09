#pragma once

#include "UnObject.h"
#include "Mesh/SkeletalMesh.h"
#include "UnrealMesh/UnMesh4.h"
class FPoseBone
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

	void AddToTracks(TArray<CAnimTrack>& tracks)
	{
		assert(tracks.Num() == Bones.Num());

		for (int32 Index = 0; Index < Bones.Num(); ++Index)
		{
			if (!Bones[Index].IsValidKey) continue;

			FTransform Transform = Bones[Index].Transform;
			CQuat idek;
			idek.X = Transform.Rotation.X;
			idek.Y = Transform.Rotation.Y;
			idek.Z = Transform.Rotation.Z;
			idek.W = Transform.Rotation.W;
			tracks[Index].KeyQuat[AnimFrame] = idek;
			tracks[Index].KeyPos[AnimFrame] = Transform.Translation;
			tracks[Index].KeyScale[AnimFrame] = Transform.Scale3D;
		}
	}
};



class CAnimSet;

class FAnimationRuntime
{
public:
	static TArray<FCompactPose>& LoadRestAsPoses(USkeleton* Skel);
	static const TArray<FCompactPose>& LoadAsPoses(const CAnimSequence* Anim, const USkeleton* Skeleton, const int refFrame);
	static const TArray<FCompactPose>& LoadAsPoses(const CAnimSequence* Anim, const USkeleton* Skeleton);
};
