/* date = August 9th 2021 9:19 pm */

#ifndef SGE_ENTITY_H
#define SGE_ENTITY_H

#define ENTITY_INVALID_POSITION (V2){100000.0f, 100000.0f}

inline void Entity_set_entity_sim_flags(EntitySim *entity, u32 flags)
{
    entity->flags |= flags;
    
    return;
};

inline void Entity_clear_entity_sim_flags(EntitySim *entity, u32 flags)
{
    entity->flags &= ~flags;
    
    return;
};

inline b32 Entity_is_entity_sim_flags_set(EntitySim *entity, u32 flags)
{
    b32 result = entity->flags & flags;
    
    return result;
};

inline void
Entity_make_nonspatial(EntitySim *entity)
{
    Entity_set_entity_sim_flags(entity, EntitySimFlag_nonspatial);
    entity->position = ENTITY_INVALID_POSITION;
    
    return;
}

inline void
Entity_make_spatial(EntitySim *entity, V2 pos, V2 vel)
{
    Entity_clear_entity_sim_flags(entity, EntitySimFlag_nonspatial);
    entity->position = pos;
    entity->velocity = vel;
    
    return;
}



#endif //SGE_ENTITY_H
