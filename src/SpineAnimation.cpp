#include "SpineAnimation.h"

#include "Texture.h"
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
    spAnimationState_apply(spineAnimation.animationState, spineAnimation.skeleton);
    spSkeleton_updateWorldTransform(spineAnimation.skeleton);
}

static float worldVerticesPositions[2048];
static Mesh::Vertex vertices[2048];
static unsigned indices[2048];

static void AddVertex(float x, float y, float u, float v, float r, float g, float b, float a, int* index)
{
    Mesh::Vertex& vertex = vertices[*index];
    vertex.pos.x = x;
    vertex.pos.y = y;
    vertex.uv.x = u;
    vertex.uv.y = v;
    vertex.color.r = r;
    vertex.color.g = g;
    vertex.color.b = b;
    vertex.color.a = a;

    *index += 1;
}

void SpineAnimation::Render(SpineAnimation& spineAnimation, Mesh& mesh, Shader& shader)
{
    mat4 proj = ortho(-400, 400, 0, 600, -10, 10);

    for (int i = 0, n = spineAnimation.skeleton->slotsCount; i < n; i++)
    {
        spSlot* slot = spineAnimation.skeleton->slots[i];

        spAttachment* attachment = slot->attachment;
        if (!attachment) continue;

        switch (slot->data->blendMode)
        {
        }

        float r = spineAnimation.skeleton->r * slot->r;
        float g = spineAnimation.skeleton->g * slot->g;
        float b = spineAnimation.skeleton->b * slot->b;
        float a = spineAnimation.skeleton->a * slot->a;

        Texture* texture = 0;
        int vertexIndex = 0;
        int indexCount = 0;
        if (attachment->type == SP_ATTACHMENT_REGION)
        {
            spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;

            texture = (Texture*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;

            spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, worldVerticesPositions);

            AddVertex(worldVerticesPositions[0], worldVerticesPositions[1],
                      regionAttachment->uvs[0], regionAttachment->uvs[1],
                      r, g, b, a, &vertexIndex);

            AddVertex(worldVerticesPositions[2], worldVerticesPositions[3],
                      regionAttachment->uvs[2], regionAttachment->uvs[3],
                      r, g, b, a, &vertexIndex);

            AddVertex(worldVerticesPositions[4], worldVerticesPositions[5],
                      regionAttachment->uvs[4], regionAttachment->uvs[5],
                      r, g, b, a, &vertexIndex);

            AddVertex(worldVerticesPositions[6], worldVerticesPositions[7],
                      regionAttachment->uvs[6], regionAttachment->uvs[7],
                      r, g, b, a, &vertexIndex);

            indexCount = 6;
            indices[0] = 0U;
            indices[1] = 1U;
            indices[2] = 2U;
            indices[3] = 2U;
            indices[4] = 3U;
            indices[5] = 0U;
        }
        else if (attachment->type == SP_ATTACHMENT_MESH)
        {
            spMeshAttachment* meshAttachment = (spMeshAttachment*)attachment;

            if (meshAttachment->super.worldVerticesLength > 2048) continue;

            texture = (Texture*)((spAtlasRegion*)meshAttachment->rendererObject)->page->rendererObject;

            spVertexAttachment_computeWorldVertices(&meshAttachment->super, slot, worldVerticesPositions);

            for (int i = 0; i < meshAttachment->trianglesCount; ++i)
            {
                int index = meshAttachment->triangles[i] << 1;
                
                indices[indexCount++] = index;

                AddVertex(worldVerticesPositions[index], worldVerticesPositions[index + 1],
                          meshAttachment->uvs[index], meshAttachment->uvs[index + 1],
                          r, g, b, a, &vertexIndex);
            }
        }
        else
        {
            continue;
        }

        if (!texture)
        {
            continue;
        }

        Mesh::SetIndices(mesh, indexCount, indices, GL_STREAM_DRAW);
        Mesh::SetVertices(mesh, vertexIndex, vertices, GL_STREAM_DRAW);

        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
        
        Shader::UseShader(shader);
        Shader::SetUniform(shader, "SpineView_MVP", proj);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->handle);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, NULL);
        //glDrawArrays(GL_TRIANGLES, 0, vertexIndex);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void SpineAnimation::Play(SpineAnimation& spineAnimation, const char* name, bool loop)
{
    spAnimationState_setAnimationByName(spineAnimation.animationState, 0, name, loop);
}

void SpineAnimation::Stop(SpineAnimation& spineAnimation)
{
    spAnimationState_setEmptyAnimation(spineAnimation.animationState, 0, 0.0f);
}