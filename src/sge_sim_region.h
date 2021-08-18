/* date = August 9th 2021 8:27 am */
#ifndef SGE_SIM_REGION_H
#define SGE_SIM_REGION_H


#define HITPOINT_SUB_COUNT (4)
typedef struct HitPoint HitPoint;
struct HitPoint
{
    u8 flags;
    u8 filled_amount;
}; 


typedef struct MoveSpec MoveSpec;
struct MoveSpec
{
    b32 unitmaxaccel;
    f32 drag;
    f32 speed;  // m/s^2
};

typedef struct EntitySim EntitySim;

typedef union EntityReference EntityReference;
union EntityReference
{
    EntitySim *ptr;
    u32        index;
};

typedef struct EntitySim EntitySim;
struct EntitySim
{
    u32 index_storage;
    
    V2  position;
    s32 tile_abs_z; // NOTE(MIGUEL): should this be chunk z??
    u32 facing_direction;
    
    f32 z;
    f32 delta_z;
    
    EntityType type;
    
    f32 width, height;
    V2  velocity;
    
    f32 bob_t;
    
    // NOTE(MIGUEL): for stairs
    s32 delta_tile_abs_z;
    b32 collides;
    
    u32 index_high;
    
    u32 hit_point_max;
    HitPoint hit_points[16];
    
    EntityReference sword;
    f32 distance_remaining; // sword 
};

typedef struct EntitySimHash EntitySimHash;
struct EntitySimHash
{
    EntitySim *ptr  ;
    u32        index;
};

typedef struct SimRegion SimRegion;
struct SimRegion
{
    World *world;
    WorldCoord origin;
    RectV2     bounds;
    
    u32 max_entity_count;
    u32 entity_count;
    EntitySim *entities;
    
    EntitySimHash hash[4096];
};


#endif //SGE_SIMULATION_REGION_H
