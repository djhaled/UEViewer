
#include "Core.h"
#include "UnCore.h"
#include "AnimationRuntime.h"

const TArray<FCompactPose>& FAnimationRuntime::LoadAsPoses(const CAnimSequence* seq, const USkeleton* Skeleton)
{
	TArray<FCompactPose>* poses = new TArray<FCompactPose>();
	poses->SetNum(seq->NumFrames);
	for (int frameIndex = 0; frameIndex < poses->Num(); frameIndex++)
	{
		FCompactPose& CpactPose = (*poses)[frameIndex];
		CpactPose.Bones.SetNum(Skeleton->BoneTree.Num());
		for (int boneIndex = 0; boneIndex < CpactPose.Bones.Num(); boneIndex++)
		{
			//
			FTransform NewTran;
			FMeshBoneInfo boneInfo = Skeleton->ReferenceSkeleton.RefBoneInfo[boneIndex];
			FTransform originalTransform = Skeleton->ReferenceSkeleton.RefBonePose[boneIndex];
			CAnimTrack* track = seq->Tracks[boneIndex];
			CVec3 bonePosition;
			CQuat boneOrientation;
			bonePosition.Set(0, 0, 0);
			boneOrientation.Set(0, 0, 0, 1);
			track->GetBonePosition(frameIndex, seq->NumFrames, false, bonePosition, boneOrientation);
			//
			FPoseBone& tert = CpactPose.Bones[boneIndex];
			tert.Name = boneInfo.Name;
			tert.ParentIndex = boneInfo.ParentIndex;
			tert.Transform = NewTran;
			tert.IsValidKey = frameIndex <= (track->KeyPos.Num() < track->KeyQuat.Num() ? track->KeyPos.Num() : track->KeyQuat.Num());
		}
	}
	return *poses;
}

void FAnimationRuntime::AccumulateLocalSpaceAdditivePoseInternal(FCompactPose basePose, FCompactPose additivePose, float weight)
{
	if (weight < 0.999989986419678)
	{
		return;
	}

	for (int index = 0; index < basePose.Bones.Num(); index++)
	{
		basePose.Bones[index].AccumulateWithAdditiveScale(additivePose.Bones[index].Transform, weight);
	}
}

void FAnimationRuntime::ConvertMeshRotationPoseToLocalSpaceV2(FCompactPose pose)
{
	for (int boneIndex = pose.Bones.Num() - 1; boneIndex > 0; --boneIndex)
	{
		int parentIndex = pose.Bones[boneIndex].ParentIndex;
		FQuat localSpaceRotation = pose.Bones[parentIndex].Transform.Rotation.Inverse() * pose.Bones[boneIndex].Transform.Rotation;
		pose.Bones[boneIndex].Transform.Rotation = localSpaceRotation;
	}
}


void FAnimationRuntime::AccumulateMeshSpaceRotationAdditiveToLocalPoseInternal(FCompactPose basePose, FCompactPose additivePose, float weight)
{
	ConvertPoseToMeshRotation(basePose);
	AccumulateLocalSpaceAdditivePoseInternal(basePose, additivePose, weight);
	ConvertMeshRotationPoseToLocalSpaceV2(basePose);
}

void FAnimationRuntime::ConvertPoseToMeshRotation(FCompactPose localPose)
{
	for (int boneIndex = 1; boneIndex < localPose.Bones.Num(); ++boneIndex)
	{
		int parentIndex = localPose.Bones[boneIndex].ParentIndex;
		FQuat meshSpaceRotation = localPose.Bones[parentIndex].Transform.Rotation * localPose.Bones[boneIndex].Transform.Rotation;
		localPose.Bones[boneIndex].Transform.Rotation = meshSpaceRotation;
	}
}


const TArray<FCompactPose>& FAnimationRuntime::LoadAsPoses(const CAnimSequence* seq, const USkeleton* Skeleton, const int refFrame)
{
	TArray<FCompactPose>* Poses = new TArray<FCompactPose>();
	Poses->SetNum(1);
	for (int frameIndex = 0; frameIndex < Poses->Num(); frameIndex++)
	{
		FCompactPose& CpactPose = (*Poses)[frameIndex];
		CpactPose.Bones.SetNum(Skeleton->BoneTree.Num());

		for (int boneIndex = 0; boneIndex < CpactPose.Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Skeleton->ReferenceSkeleton.RefBoneInfo[boneIndex];
			FTransform originalTransform = Skeleton->ReferenceSkeleton.RefBonePose[boneIndex];
			CAnimTrack* Track = seq->Tracks[boneIndex];
			CVec3 bonePosition;
			CQuat boneOrientation;
			bonePosition.Set(0, 0, 0);
			boneOrientation.Set(0, 0, 0, 1);
			Track->GetBonePosition(refFrame, seq->NumFrames, false, bonePosition, boneOrientation);

			switch (Skeleton->BoneTree[boneIndex])
			{
			    case EBoneTranslationRetargetingMode::Skeleton:
			    {
				    CSkeletonBonePosition BonePos;
				    CQuat TT;
				    TT.Set(originalTransform.Rotation.X, originalTransform.Rotation.Y, originalTransform.Rotation.Z, originalTransform.Rotation.W);
				    BonePos.Orientation = TT;
				    BonePos.Position = originalTransform.Translation;
				    CSkeletonBonePosition targetTransform = seq->RetargetBasePose.IsValidIndex(boneIndex) ? seq->RetargetBasePose[boneIndex] : BonePos;
				    bonePosition = targetTransform.Position;
				    break;
			    }
				case EBoneTranslationRetargetingMode::AnimationScaled:
				{
					CVec3 Convert;
					Convert = originalTransform.Translation;
					float sourceTranslationLength = Convert.Size();
					if (sourceTranslationLength > 1e-4f)
					{
						float targetTranslationLength = seq->RetargetBasePose.IsValidIndex(boneIndex) ? seq->RetargetBasePose[boneIndex].Position.Size() : sourceTranslationLength;
						bonePosition.Scale(targetTranslationLength / sourceTranslationLength);
					}
					break;

				}
				case EBoneTranslationRetargetingMode::AnimationRelative:
				{
					// can't tell if it's working or not
					CSkeletonBonePosition BonePosZ;
					CQuat TTZ;
					TTZ.Set(originalTransform.Rotation.X, originalTransform.Rotation.Y, originalTransform.Rotation.Z, originalTransform.Rotation.W);
					BonePosZ.Orientation = TTZ;
					BonePosZ.Position = originalTransform.Translation;
					//
					FVector sourceSkelTrans = originalTransform.Translation;
					CSkeletonBonePosition refPoseTransform = seq->RetargetBasePose.IsValidIndex(boneIndex) ? seq->RetargetBasePose[boneIndex] : BonePosZ;
					BonePosZ.Orientation.Conjugate();
					boneOrientation = boneOrientation * BonePosZ.Orientation * refPoseTransform.Orientation;
					//boneOrientation = boneOrientation * FQuat::Conjugate(originalTransform.Rotation) * refPoseTransform.Rotation;
					bonePosition.Add(refPoseTransform.Position - sourceSkelTrans);
					//boneScale *= refPoseTransform. * originalTransform.Scale3D;
					boneOrientation.Normalize();
					break;

				}
				case EBoneTranslationRetargetingMode::OrientAndScale:
				{
					FVector sourceSkelTrans = originalTransform.Translation;
					CVec3 sourceSkelTransCv = sourceSkelTrans;
					CVec3 targetSkelTrans = seq->RetargetBasePose.IsValidIndex(boneIndex) ? seq->RetargetBasePose[boneIndex].Position : sourceSkelTransCv;
					if (sourceSkelTransCv != targetSkelTrans)
					{
						float sourceSkelTransLength = sourceSkelTransCv.Size();
						float targetSkelTransLength = targetSkelTrans.Size();
						if (sourceSkelTransLength * targetSkelTransLength > 0.001f)
						{
							float Scale = targetSkelTransLength / sourceSkelTransLength;
							CQuat TransRotation;
							TransRotation.FromTwoVectors(sourceSkelTransCv, targetSkelTrans);
							TransRotation.RotateVector(bonePosition, bonePosition);
							bonePosition.Scale(Scale);
						}
					}
					break;
				}
			}
			FPoseBone toct;
			toct.Name = boneInfo.Name;
			toct.ParentIndex = boneInfo.ParentIndex;
			toct.Transform = Skeleton->ReferenceSkeleton.RefBonePose[boneIndex];
			toct.IsValidKey = true;
			(*Poses)[frameIndex].Bones[boneIndex] = toct;
		}
	}

	return *Poses;
}

TArray<FCompactPose>& FAnimationRuntime::LoadRestAsPoses(USkeleton* Skel)
{
	TArray<FCompactPose>* Poses = new TArray<FCompactPose>();
	Poses->SetNum(1);
	for (int frameIndex = 0; frameIndex < Poses->Num(); frameIndex++)
	{
		FCompactPose& Pose = (*Poses)[frameIndex];
		Pose.Bones.SetNum(Skel->BoneTree.Num());
		for (int boneIndex = 0; boneIndex < (*Poses)[frameIndex].Bones.Num(); boneIndex++)
		{
			FMeshBoneInfo boneInfo = Skel->ReferenceSkeleton.RefBoneInfo[boneIndex];
			FPoseBone taqt;
			taqt.Name = boneInfo.Name;
			taqt.ParentIndex = boneInfo.ParentIndex;
			taqt.Transform = Skel->ReferenceSkeleton.RefBonePose[boneIndex];
			taqt.IsValidKey = true;
			(*Poses)[frameIndex].Bones[boneIndex] = taqt;

		}
	}
	return *Poses;
}



