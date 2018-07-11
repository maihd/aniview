#include "SpineAnimation.h"

#include "memwise/membuf.h"
#include "memwise/table.hpp"

static table_t<const char*, spAtlas*>        atlases(membuf_heap());
static table_t<const char*, spSkeletonData*> skeletonDatas(membuf_heap());

static spAtlas* LoadAtlas(const char* path)
{
    spAtlas* atlas;
    if (table::tryget(atlases, path, atlas))
    {
        return atlas;
    }

    atlas = spAtlas_createFromFile(path, NULL);
    if (atlas)
    {
        table::set(atlases, path, atlas);
    }
    return atlas;
}

static spSkeletonData* LoadDataFromJson(spAtlas* atlas, const char* path)
{
    spSkeletonData* data;
    if (table::tryget(skeletonDatas, path, data))
    {
        return data;
    }

    spAtlasAttachmentLoader* loader = spAtlasAttachmentLoader_create(atlas);
    spSkeletonJson* json = spSkeletonJson_createWithLoader(&loader->super);

    data = spSkeletonJson_readSkeletonDataFile(json, path);

    spSkeletonJson_dispose(json);

    return data;
}

bool SpineAnimation::Create(SpineAnimation& spineAnimation, const char* atlasPath, const char* jsonPath)
{
    spAtlas* atlas = LoadAtlas(atlasPath);
    if (!atlas)
    {
        return false;
    }

    spSkeletonData* skeletonData = LoadDataFromJson(atlas, jsonPath);
    if (!skeletonData)
    {
        return false;
    }

    spSkeleton* skeleton = spSkeleton_create(skeletonData);
    spAnimationStateData* animationData = spAnimationStateData_create(skeletonData);
    spAnimationState* animationState = spAnimationState_create(animationData);

    spineAnimation.skeleton = skeleton;
    spineAnimation.animationState = animationState;

    return true;
}

bool SpineAnimation::Delete(SpineAnimation& spineAnimation)
{
    spSkeleton_dispose(spineAnimation.skeleton);
    spAnimationState_dispose(spineAnimation.animationState);

    spineAnimation.skeleton = NULL;
    spineAnimation.animationState = NULL;

    return true;
}

void SpineAnimation::Update(SpineAnimation& spineAnimation, float deltaTime)
{
    spAnimationState_update(spineAnimation.animationState, deltaTime);
}