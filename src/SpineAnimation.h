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
    spSkeleton*       skeleton;
    spAnimationState* animationState;

    inline SpineAnimation()
        : skeleton(0)
        , animationState(0)
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
    static void Render(SpineAnimation& spineAnimation, Mesh& mesh, Shader& shader);
};

#endif /* __SPINE_ANIMATION_H__ */