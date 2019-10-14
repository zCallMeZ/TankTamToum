#include <sfml_engine/spine.h>
#include "engine/log.h"
#include <graphics/graphics.h>
#include <sfml_engine/transform.h>

/*
namespace neko::sfml
{


BasicSpineDrawable::BasicSpineDrawable() : skeletonDrawable(nullptr)
{

}

BasicSpineDrawable::~BasicSpineDrawable()
{
    if (skeletonData)
        SkeletonData_dispose(skeletonData);
    if (atlas)
        Atlas_dispose(atlas);
}

void BasicSpineDrawable::SetPosition(const sf::Vector2f& position)
{
    if(skeletonDrawable)
    {
        skeletonDrawable->skeleton->x = position.x;
        skeletonDrawable->skeleton->y = position.y;
        Skeleton_updateWorldTransform(skeletonDrawable->skeleton);
    }
}

sf::Vector2f BasicSpineDrawable::GetPosition()
{
    if(!skeletonDrawable)
        return sf::Vector2f();
    return sf::Vector2f(skeletonDrawable->skeleton->x,skeletonDrawable->skeleton->y);
}

void BasicSpineDrawable::SetSkinByName(std::string_view skinName)
{
    if(skeletonDrawable == nullptr)
        return;
    Skeleton_setSkinByName(skeletonDrawable->skeleton, skinName.data());
    Skeleton_setSlotsToSetupPose(skeletonDrawable->skeleton);
}

void BasicSpineDrawable::SetAnimationByName(std::string_view animName)
{
    if(skeletonDrawable == nullptr)
        return;
    AnimationState_setAnimationByName(skeletonDrawable->state, 0, animName.data(), 1);
}


SkeletonData* readSkeletonJsonData(const char* filename, Atlas* atlas, float scale)
{
    SkeletonJson* json = SkeletonJson_create(atlas);
    json->scale = scale;
    SkeletonData* skeletonData = SkeletonJson_readSkeletonDataFile(json, filename);
    if (!skeletonData)
    {
        logDebug(json->error);
        return nullptr;
    }
    SkeletonJson_dispose(json);
    return skeletonData;
}

SkeletonData* readSkeletonBinaryData(const char* filename, Atlas* atlas, float scale)
{
    SkeletonBinary* binary = SkeletonBinary_create(atlas);
    binary->scale = scale;
    SkeletonData* skeletonData = SkeletonBinary_readSkeletonDataFile(binary, filename);
    if (!skeletonData)
    {
        logDebug(binary->error);
        return nullptr;
    }
    SkeletonBinary_dispose(binary);
    return skeletonData;
}


SpineManager::SpineManager() : ComponentManager::ComponentManager()
{
    ResizeIfNecessary(infos_, INIT_ENTITY_NMB - 1, {});
};


void SpineManager::Update(EntityManager& entityManager, float dt)
{
    for (size_t i = 0; i < components_.size(); i++)
    {
        if (entityManager.HasComponent(neko::Entity(i), EntityMask(NekoComponentType::SPINE_ANIMATION)))
        {
            if (components_[i].skeletonDrawable != nullptr)
            {
                components_[i].skeletonDrawable->update(dt);
                components_[i].transform = sf::Transform();
            }
        }
    }
}


bool SpineManager::AddSpineDrawable(Entity entity,
                                    const std::string_view atlasFilename,
                                    const std::string_view skeletonFilename)
{
    auto& spineAnimation = components_[entity];
    if (spineAnimation.atlas)
    {
        Atlas_dispose(spineAnimation.atlas);
        spineAnimation.atlas = nullptr;
    }
    spineAnimation.atlas = Atlas_createFromFile(atlasFilename.data(), 0);
    if (!spineAnimation.atlas)
    {
        logDebug("[Error] Could not load spine atlas file");
        return false;
    }
    if (spineAnimation.skeletonData)
    {
        SkeletonData_dispose(spineAnimation.skeletonData);
        spineAnimation.skeletonData = nullptr;
    }
    spineAnimation.skeletonData = readSkeletonJsonData(skeletonFilename.data(),
                                                       spineAnimation.atlas, 1.0f);
    if (!spineAnimation.skeletonData)
    {
        logDebug("[Error] while trying to load spine skeleton file");
        return false;
    }
    spineAnimation.skeletonDrawable = std::make_shared<spine::SkeletonDrawable>(spineAnimation.skeletonData);
    return true;
}

void SpineManager::ParseComponentJson(json& componentJson, Entity entity)
{
    const std::string atlasFilename = componentJson["atlas"];
    const std::string skeletonDataFilename = componentJson["skeletonData"];
    AddSpineDrawable(entity, atlasFilename, skeletonDataFilename);


}

void SpineManager::CopyAllTransformPositions(EntityManager& entityManager, SfmlPosition2dManager& position2Manager)
{
    EntityMask entityMask = EntityMask(NekoComponentType::POSITION2D) | EntityMask(NekoComponentType::SPINE_ANIMATION);

    for (Entity entity = 0; entity < entityManager.GetEntitiesSize(); entity++)
    {
        if (entityManager.HasComponent(entity, entityMask))
        {
            const auto& pos = position2Manager.GetComponent(entity);
            components_[entity].SetPosition(pos);
        }
    }
}

void SpineManager::CopyAllTransformScales(EntityManager& entityManager, Scale2dManager& scale2DManager)
{
    EntityMask entityMask = EntityMask(NekoComponentType::SCALE2D) |
            EntityMask(NekoComponentType::SPINE_ANIMATION);

    for (Entity entity = 0; entity < entityManager.GetEntitiesSize(); entity++)
    {
        if (entityManager.HasComponent(entity, entityMask))
        {
            const auto& scale = scale2DManager.GetComponent(entity);
            components_[entity].transform = components_[entity].transform.scale(scale, components_[entity].GetPosition());
        }
    }
}

void SpineManager::CopyAllTransformAngles(EntityManager& entityManager, SfmlRotation2dManager& angle2DManager)
{
    EntityMask entityMask = EntityMask(NekoComponentType::ROTATION2D) |
                            EntityMask(NekoComponentType::SPINE_ANIMATION);

    for (Entity entity = 0; entity < entityManager.GetEntitiesSize(); entity++)
    {
        if (entityManager.HasComponent(entity, entityMask))
        {
            const auto& angle = angle2DManager.GetComponent(entity);
            components_[entity].transform = components_[entity].transform.rotate(angle, components_[entity].GetPosition());
        }
    }
}


void SpineManager::PushAllCommands(EntityManager& entityManager, SfmlGraphicsManager& graphicsManager)
{
    EntityMask entityMask = EntityMask(NekoComponentType::SPINE_ANIMATION);

    for (Entity entity = 0; entity < entityManager.GetEntitiesSize(); entity++)
    {
        if (entityManager.HasComponent(entity, entityMask))
        {
            if (components_[entity].skeletonDrawable != nullptr)
            {
                sf::RenderStates states(components_[entity].transform);
                graphicsManager.Draw(&components_[entity]);
            }
        }
    }
}

Index SpineManager::AddComponent(EntityManager& entityManager, Entity entity)
{
    ResizeIfNecessary(infos_, entity, {});
    return ComponentManager::AddComponent(entityManager, entity);
}

json SpineManager::SerializeComponentJson(Entity entity)
{
    json componentJson;
    const auto& componentInfo = infos_[entity];
    componentJson["atlas"] = componentInfo.atlasPath;
    componentJson["skeletonData"] = componentInfo.skeletonDataPath;
    return componentJson;
}

SpineDrawableInfo& SpineManager::GetInfo(Entity entity)
{
    return infos_[entity];
}

void SpineManager::CopyAllTransforms(EntityManager& entityManager, SfmlTransform2dManager& transformManager)
{
    const auto entityMask = EntityMask(NekoComponentType::TRANSFORM2D) | EntityMask(NekoComponentType::SPINE_ANIMATION);
    for(Entity entity = 0; entity < entityManager.GetEntitiesSize(); entity++)
    {
        if(entityManager.HasComponent(entity, entityMask))
        {
            const auto transform = transformManager.CalculateTransform(entity);
            components_[entity].transform = transform;
        }
    }
}

void SpineManager::DestroyComponent(EntityManager& entityManager, Entity entity)
{
    ComponentManager::DestroyComponent(entityManager, entity);
    components_[entity] = BasicSpineDrawable();
}

void SpineManager::CopyLayer(int layer, size_t start, size_t length)
{
    for (auto i = start; i < start + length; i++)
    {
        components_[i].layer = layer;
    }
}

}
*/