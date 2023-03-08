#pragma once

#include "UnObject.h"
#include "Mesh/SkeletalMesh.h"
#include "UnrealMesh/UnMesh4.h"
class FPoseBone
{
public:
	FTransform Transform;
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
	}
	FCompactPose() : AnimFrame(0), Processed(false)
	{
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
	//static TArray<FCompactPose> LoadRestAsPoses(CAnimSet* Anim);
	//static TArray<FCompactPose> LoadAsPoses(CAnimSet* Anim, int32 RefFrame);
	static TArray<FCompactPose> LoadAsPoses(const CAnimSet* Anim);
};
