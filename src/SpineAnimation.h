#ifndef __SPINE_ANIMATION_H__
#define __SPINE_ANIMATION_H__

#include <assert.h>
#include <GL/glew.h>

#include "spine/spine.h"

#include "Mesh.h"
#include "Shader.h"

struct SpineAnimation
{
public:
    spAtlas*              atlas;
    spSkeleton*           skeleton;
    spSkeletonData*       skeletonData;
    spAnimationState*     animationState;
    spAnimationStateData* animationStateData;

    inline SpineAnimation()
        : atlas(0)
        , skeleton(0)
        , skeletonData(0)
        , animationState(0)
        , animationStateData(0)
    {
    }

    inline ~SpineAnimation()
    {
        // EMPTY
    }

public:
    static bool Create(SpineAnimation& spineAnimation, const char* atlasPath, const char* jsonPath);
    static bool Delete(SpineAnimation& spineAnimation);

    static void Update(SpineAnimation& spineAnimation, float deltaTime);
    static void Render(SpineAnimation& spineAnimation, Mesh& mesh, Shader& shader, const float4x4& transform = float4x4::identity());

	static void Play(SpineAnimation& spineAnimation, int index, bool loop = true);
    static void Play(SpineAnimation& spineAnimation, const char* name, bool loop = true);
    static void Stop(SpineAnimation& spineAnimation);

    static int  AnimationCount(SpineAnimation& spineAnimation);
    static bool AnimationNames(SpineAnimation& spineAnimation, char* names[], int count);
};

#endif /* __SPINE_ANIMATION_H__ */