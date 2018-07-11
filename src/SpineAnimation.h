#ifndef __SPINE_ANIMATION_H__
#define __SPINE_ANIMATION_H__

#include <assert.h>
#include <GL/glew.h>

#include "spine/spine.h"

struct SpineAnimation
{
public:
    GLuint            vao;
    GLuint            vbo;
    GLuint            ebo;

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
};

#endif /* __SPINE_ANIMATION_H__ */