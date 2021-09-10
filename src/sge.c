#include "sge.h"
#include "sge_world.c"
#include "sge_random.h"
#include "sge_sim_region.c"
#include "sge_entity.c"
// TODO(MIGUEL): App cannnot Crash when stick is not connected
// TODO(MIGUEL): App cannnot Crash when MCU is not connected
// TODO(MIGUEL): App should give use an oppertunity to connect a device(stick, mcu) thoughout app life time
// TODO(MIGUEL): Integrate FreeType library
// TODO(MIGUEL): Implement a simple gui
// TODO(MIGUEL): GUI: No cursor, Buttons with key
// TODO(MIGUEL): Integrate Opencv once app has access to camera feed
// NOTE(MIGUEL): Check Network.h for Networking TODOS
//fuck git

SGE_GET_SOUND_SAMPLES(SGEGetSoundSamples)
{
    GameState *game_state = (GameState *)sge_memory->permanent_storage;
    
    Game_update_sound_buffer(game_state, sound_buffer, 400);
    
    return;
}

SGE_INIT(SGEInit)
{
    // NOTE(MIGUEL): get rid of this it makes no sense
    
    return;
}



//~ Entity Interface

typedef struct CreateEntitySimResult CreateEntitySimResult;
struct CreateEntitySimResult
{
    EntityLow *entity_low;
    u32         index_low;
};

internal CreateEntitySimResult
Entity_create_entity_low(GameState *game_state, EntityType type, WorldCoord position)
{
    ASSERT(game_state->entity_count_low < ARRAYCOUNT(game_state->entities_low));
    
    u32 index_low = game_state->entity_count_low++;
    
    EntityLow *entity_low = game_state->entities_low + index_low;
    
    memset(entity_low, 0, sizeof(EntitySim));
    entity_low->sim.type     = type;
    entity_low->position = World_null_position();
    
    World_change_entity_location(game_state->world,
                                 index_low,
                                 entity_low,
                                 position,
                                 &game_state->world_arena);
    
    CreateEntitySimResult result;
    result.index_low  = index_low;
    result.entity_low = entity_low;
    
    return result;
}

internal void
init_hitpoint(EntityLow *entity_low, u32 hitpointcount)
{
    ASSERT(hitpointcount <= ARRAYCOUNT(entity_low->sim.hit_points));
    
    entity_low->sim.hit_point_max = hitpointcount;
    
    for(u32 hitpoint_index = 0; 
        hitpoint_index < entity_low->sim.hit_point_max; hitpoint_index++)
    {
        HitPoint *hitpoint = entity_low->sim.hit_points + hitpoint_index;
        hitpoint->flags = 0;
        hitpoint->filled_amount = HITPOINT_SUB_COUNT;
    }
    
    return;
}


internal CreateEntitySimResult
Game_add_sword(GameState *game_state)
{ 
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_sword,
                                                            World_null_position());
    
    result.entity_low->sim.height          = 0.5f; //           UNITS: meters
    result.entity_low->sim.width           = 1.0f; //           UNITS: meters
    Entity_set_entity_sim_flags(&result.entity_low->sim, EntitySimFlag_nonspatial);
    
    return result;
}


internal CreateEntitySimResult
Game_add_player(GameState *game_state, u32 player_number)
{
    WorldCoord position = game_state->camera_position;
    position.rel_.x += player_number * 2;
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_player,
                                                            position);
    
    CreateEntitySimResult weapon = Game_add_sword (game_state);
    
    result.entity_low->sim.sword.index = weapon.index_low;
    
    init_hitpoint(result.entity_low, 3);
    
    result.entity_low->sim.height          = 0.5f; //           UNITS: meters
    result.entity_low->sim.width           = 1.0f; //           UNITS: meters
    Entity_set_entity_sim_flags(&result.entity_low->sim, EntitySimFlag_collides);
    
    if(game_state->camera_following_entity_index == 0)
    {
        game_state->camera_following_entity_index = result.index_low;
    }
    
    return result;
}


internal CreateEntitySimResult
Game_add_friendly(GameState *game_state, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldCoord position = World_worldcoord_from_tilecoord(game_state->world,
                                                          tile_abs_x,
                                                          tile_abs_y,
                                                          tile_abs_z);
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_friendly,
                                                            position);
    
    //init_hitpoint(result.entity_low, 3); // NOTE(MIGUEL): these have no hitpoints yet
    
    result.entity_low->sim.height          = 0.5f; // UNITS: meters
    result.entity_low->sim.width           = 1.0f; // UNITS: meters
    Entity_set_entity_sim_flags(&result.entity_low->sim, EntitySimFlag_collides);
    
    return result;
}


internal CreateEntitySimResult
Game_add_hostile(GameState *game_state, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    WorldCoord position = World_worldcoord_from_tilecoord(game_state->world,
                                                          tile_abs_x,
                                                          tile_abs_y,
                                                          tile_abs_z);
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_hostile,
                                                            position);
    
    
    init_hitpoint(result.entity_low, 3);
    
    
    result.entity_low->sim.height          = 0.5f; //           UNITS: meters
    result.entity_low->sim.width           = 1.0f; //           UNITS: meters
    Entity_set_entity_sim_flags(&result.entity_low->sim, EntitySimFlag_collides);
    
    return result;
}

internal CreateEntitySimResult
Game_add_wall(GameState *game_state, s32 tile_abs_x, s32 tile_abs_y, s32 tile_abs_z)
{
    
    WorldCoord position = World_worldcoord_from_tilecoord(game_state->world,
                                                          tile_abs_x,
                                                          tile_abs_y,
                                                          tile_abs_z);
    
    CreateEntitySimResult result = Entity_create_entity_low(game_state,
                                                            EntityType_wall,
                                                            position);
    
    result.entity_low->sim.height   = game_state->world->side_in_meters_tile; //UNITS: meters
    result.entity_low->sim.width    = result.entity_low->sim.height; //UNITS: meters
    Entity_set_entity_sim_flags (&result.entity_low->sim, EntitySimFlag_collides);
    
    return result;
}

internal void
Game_draw_mini_map(GameState *game_state,
                   GameBackBuffer *back_buffer,
                   SimRegion *sim_region,
                   s32 tile_side_in_pixels,
                   V2 camera_range_in_meters,
                   s32 x, s32 y, f32 normalized_zoom)
{
    World *world = game_state->world;
    
    
    // NOTE(MIGUEL): This is only good for positioning of the camera
    EntityLow *camera_following_entity = Entity_get_entity_low(game_state,
                                                               game_state->camera_following_entity_index);
    f32 meters_to_pixels = ((f32)tile_side_in_pixels / (f32)world->side_in_meters_tile);
    
    /// CAMERA BASIS CHANGE
    V2 screen_center =
    {
        0.5f * (f32)back_buffer->width,
        0.5f * (f32)back_buffer->height
    };
    
    V2 minimap_center = 
    {
        (f32)screen_center.x + x,
        (f32)screen_center.y + y,
    };
    
    V2_scale((f32)meters_to_pixels, &camera_range_in_meters);
    
    V2 draw_pos = { 0 };
    
    if(camera_following_entity)
    {
        draw_pos.x = camera_following_entity->sim.position.x * meters_to_pixels;
        draw_pos.y = camera_following_entity->sim.position.y * meters_to_pixels;
    }
    
    V2 draw_size_in_meters =
    {
        .x = 1.3f * meters_to_pixels,
        .y = 1.3f * meters_to_pixels,
    };
    
    
    
    RectV2 minimap_draw_area = RectV2_center_dim(minimap_center, camera_range_in_meters);
    
    // TODO(MIGUEL): calculate the which chunks are in in the minimap view space.
    //               Grab low entities from those chunks to render their status.
    s32 view_scale = 1;
    camera_range_in_meters.x *= view_scale;
    camera_range_in_meters.y *= view_scale;
    
    
    V3 color  = { 0 };
    V3 red    = {1.0f, 0.0f, 0.0f};
    V3 green  = {0.0f, 1.0f, 0.0f};
    V3 blue   = {0.0f, 0.0f, 1.0f};
    V3 yellow = {1.0f, 1.0f, 0.0f};
    V3 purple = {1.0f, 0.0f, 1.0f};
    V3 grey   = {0.6f, 0.6f, 0.6f};
    V3 white  = {1.0f, 1.0f, 1.0f};
    V3 black  = {0.0f, 0.0f, 0.0f};
    V3 dark_gray = {0.3f, 0.3f, 0.3f};
    
    /// MINIMAP BACKGROUND
    Game_draw_rectangle(back_buffer,
                        (V2){minimap_draw_area.min.x,minimap_draw_area.min.y},
                        (V2){minimap_draw_area.max.x,minimap_draw_area.max.y},
                        green,
                        0);
    
    
    f32 meters_to_pixels_with_zoom_comp = meters_to_pixels * ((4.0f * normalized_zoom));
    EntityLow *entity_low = game_state->entities_low;
    
    Game_draw_rectangle(back_buffer,
                        sim_region->bounds.min,
                        sim_region->bounds.max,
                        color,
                        0);
    
    for(u32 entity_low_index = 1;
        entity_low_index < game_state->entity_count_low;
        entity_low_index++, entity_low++)
    {
        V3 draw_color = red;
        
        draw_pos.x = entity_low->sim.position.x * meters_to_pixels_with_zoom_comp;
        draw_pos.y = entity_low->sim.position.y * meters_to_pixels_with_zoom_comp;
        
        draw_size_in_meters.x = 1.3f * meters_to_pixels_with_zoom_comp;
        draw_size_in_meters.y = 1.3f * meters_to_pixels_with_zoom_comp;
        
        EntitySimHash *entry = SimRegion_get_hash_from_storage_index(sim_region, entity_low_index);
        
        if(entry->ptr == NULLPTR)
        {
            draw_color = grey;
            
        }
        
        Game_draw_rectangle(back_buffer,
                            (V2)
                            {
                                (minimap_center.x + draw_pos.x) - (draw_size_in_meters.x * 0.5f),
                                (minimap_center.y - draw_pos.y) - (draw_size_in_meters.y * 0.5f)
                            },
                            (V2)
                            {
                                (minimap_center.x + draw_pos.x) + (draw_size_in_meters.x * 0.5f),
                                (minimap_center.y - draw_pos.y) + (draw_size_in_meters.y * 0.5f),
                            },
                            draw_color,
                            0);
    }
    
#if 0
    for(s32 rel_column  = (s32)-half_camera_range_in_tiles.x;
        rel_column < half_camera_range_in_tiles.x;
        rel_column++)
    {
        for(s32 rel_row = (s32)-half_camera_range_in_tiles.y;
            rel_row < half_camera_range_in_tiles.y;
            rel_row++)
        {
            
            if(camera_following_entity)
            {
                //(meters_to_pixels * camera_following_entity->position.rel_.x) +
                V2 minimap_center = 
                {
                    screen_center.x - (f32)(rel_column * tile_side_in_pixels),
                    screen_center.y + (f32)(rel_row    * tile_side_in_pixels)
                };
                
                RectV2 tile =
                {
                    .min.x = minimap_center.x - 0.5f * tile_side_in_pixels + offset_from_screen_center.x,
                    .min.y = minimap_center.y - 0.5f * tile_side_in_pixels + offset_from_screen_center.y,
                    .max.x = minimap_center.x + 0.5f * tile_side_in_pixels + offset_from_screen_center.x,
                    .max.y = minimap_center.y + 0.5f * tile_side_in_pixels + offset_from_screen_center.y,
                };
                
                /*
                s32 column = camera_following_entity->position.chunk_x + rel_column;
                s32 row    = camera_following_entity->position.chunk_y + rel_row;
                */
                //UX2
                
                switch(camera_following_entity->sim.type)
                {
                    case EntityType_player:
                    {
                        color = blue;
                    } break;
                    
                    case EntityType_friendly:
                    {
                        color = yellow;
                    } break;
                    
                    case EntityType_hostile:
                    {
                        color = red;
                    } break;
                    
                    case EntityType_wall:
                    {
                        color = purple;
                    } break;
                    
                    case EntityType_sword:
                    {
                        color = grey;
                    } break;
                    
                    case EntityType_null:
                    {
                        color = dark_gray;
                    } break;
                    
                }
                
                Game_draw_rectangle(back_buffer,
                                    (V2){tile.min.x, tile.min.y},
                                    (V2){tile.max.x, tile.max.y},
                                    color,
                                    1);
                
                
                WorldCoord world_origin = {0};
                
                if((rel_column % WORLD_TILES_PER_CHUNK == 0) ||
                   (rel_row    % WORLD_TILES_PER_CHUNK == 0))
                {
                    Game_draw_rectangle(back_buffer,
                                        (V2){tile.min.x,tile.min.y},
                                        (V2){tile.max.x,tile.max.y},
                                        white,
                                        0);
                }
                
                
                if((rel_column == 0) && (rel_row == 0))
                {
                    Game_draw_rectangle(back_buffer,
                                        (V2){tile.min.x,tile.min.y},
                                        (V2){tile.max.x,tile.max.y},
                                        red,
                                        0);
                }
                
                if((rel_column == camera_following_entity->sim.position.y) &&
                   (rel_row    == camera_following_entity->sim.position.x))
                {
                    
                }
                
#if 0
                /// DRAWING THE CAMERA BOUNDS AND SIM REGION BOUNDS
                V2 tile_span_in_meters = (V2){ (17 * 3) / 2 , (9 * 3) / 2};
                V2 camera_point        = 
                { 
                    (f32)game_state->camera_position.chunk_x,
                    (f32)game_state->camera_position.chunk_y 
                };
                
                // TODO(MIGUEL): Use sim region origin and bound in stead of
                //               this bullshit. Also make sure to visulaze the 
                //               chunks and highligh chunk in the sim resion.
                RectV2 high_frequency_bounds = RectV2_center_half_dim(game_state->camera_position.rel_,
                                                                      tile_span_in_meters);
                
                V2_add(camera_point, high_frequency_bounds.min, &high_frequency_bounds.min);
                V2_add(camera_point, high_frequency_bounds.max, &high_frequency_bounds.max);
                
                tile_span_in_meters  = (V2){ 17/ 2 , 9/2};
                RectV2 camera_bounds = RectV2_center_half_dim(game_state->camera_position.rel_,
                                                              tile_span_in_meters);
                
                V2_add(camera_point, camera_bounds.min, &camera_bounds.min);
                V2_add(camera_point, camera_bounds.max, &camera_bounds.max);
                
                /// DRAW SIM REGION BOUNDS
                if((((column >= (s32)high_frequency_bounds.min.x) && (column <= (s32)high_frequency_bounds.max.x)) &&
                    ((row    == (s32)high_frequency_bounds.min.y) || (row    == (s32)high_frequency_bounds.max.y))) 
                   ||
                   ((row    >= (s32)high_frequency_bounds.min.y) && (row    <= (s32)high_frequency_bounds.max.y)) &&
                   ((column == (s32)high_frequency_bounds.min.x) || (column == (s32)high_frequency_bounds.max.x)))
                {
                    Game_draw_rectangle(back_buffer,
                                        (V2){tile.min.x,tile.min.y},
                                        (V2){tile.max.x,tile.max.y},
                                        red,
                                        0);
                }
                
                // DRAW CAMERA BOUNDS
                if((((column >= (s32)camera_bounds.min.x) && (column <= (s32)camera_bounds.max.x)) &&
                    ((row    == (s32)camera_bounds.min.y) || (row    == (s32)camera_bounds.max.y))) 
                   ||
                   ((row    >= (s32)camera_bounds.min.y) && (row    <= (s32)camera_bounds.max.y)) &&
                   ((column == (s32)camera_bounds.min.x) || (column == (s32)camera_bounds.max.x)))
                {
                    Game_draw_rectangle(back_buffer,
                                        (V2){tile.min.x,tile.min.y},
                                        (V2){tile.max.x,tile.max.y},
                                        blue,
                                        0);
                }
#endif
            }
        }
    }
#endif
    
    return;
}

internal void
Game_draw_bitmap(GameBackBuffer *buffer, BitmapData *bitmap,
                 f32 real_x, f32 real_y,
                 f32 c_alpha)
{
    /// rounding / ruling
    s32 min_x = round_f32_to_s32(real_x);
    s32 min_y = round_f32_to_s32(real_y);
    
    s32 max_x = min_x + bitmap->width ;
    s32 max_y = min_y + bitmap->height;
    
    /// clipping
    // NOTE(MIGUEL): will right to but not including the final row 
    s32 src_offset_x = 0;
    if(min_x < 0)
    {
        src_offset_x = -min_x;
        min_x = 0;
    }
    s32 src_offset_y = 0;
    if(min_y < 0)
    {
        src_offset_y = -min_y;
        min_y = 0;
    }
    if(max_x > buffer->width)
    {
        max_x = buffer->width;
    }
    if(max_y > buffer->height)
    {
        max_y = buffer->height;
    }
    
    // NOTE(MIGUEL): why is it supposed to be pixel_height - 1
    u32 *src_row  = bitmap->pixels + (bitmap->width * (bitmap->height - 1));
    
    src_row += -bitmap->width * src_offset_y + src_offset_x;
    
    u8  *dest_row = ((u8 *)  (buffer->data)            + 
                     (min_x * buffer->bytes_per_pixel) +
                     (min_y * buffer->pitch));
    
    for(s32 y = min_y; y < max_y; y++)
    {
        u32 *dest = (u32 *)dest_row;
        u32 *src  = (u32 *)src_row;
        
        for(s32 x = min_x; x < max_x; x++)
        {
            f32 a  = (f32)((*src >> 24) & 0xFF) / 255.0f;
            a *= c_alpha;
            
            f32 sr = (f32)((*src >> 16) & 0xFF);
            f32 sg = (f32)((*src >>  8) & 0xFF);
            f32 sb = (f32)((*src >>  0) & 0xFF);
            
            f32 dr = (f32)((*dest >> 16) & 0xFF);
            f32 dg = (f32)((*dest >>  8) & 0xFF);
            f32 db = (f32)((*dest >>  0) & 0xFF);
            
            f32 r = dr * (1.0f - a) + sr * a;
            f32 g = dg * (1.0f - a) + sg * a;
            f32 b = db * (1.0f - a) + sb * a;
            
            *dest = (
                     ((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) <<  8) |
                     ((u32)(b + 0.5f) <<  0));
            
            dest++;
            src++;
        }
        
        dest_row += buffer->pitch;
        src_row  -= bitmap->width;
    }
    
    
    return;
}

internal BitmapData
Game_debug_load_bmp(ThreadContext *thread, DEBUG_PlatformReadEntireFile *read_entire_file, u8 *file_name)
{
    BitmapData result = { 0 };
    
    DebugReadFileResult read_result = read_entire_file(thread, file_name);
    
    //ASSERT(read_result.contents);
    if(read_result.contents_size > 0)
    {
        BitmapHeader *header = (BitmapHeader *)read_result.contents;
        
        u32 *pixels = (u32 *)((u8 *)read_result.contents + header->bitmap_offset);
        
        result.pixels = pixels;
        result.width  = header->width;
        result.height = header->height;
        
        if(header->compression > 0)
        {
            u32 red_mask   = header->red_mask;
            u32 green_mask = header->green_mask;
            u32 blue_mask  = header->blue_mask;
            u32 alpha_mask = ~(red_mask | green_mask | blue_mask);
            
            bit_scan_result red_shift   = find_least_significant_set_bit(red_mask  );
            bit_scan_result green_shift = find_least_significant_set_bit(green_mask);
            bit_scan_result blue_shift  = find_least_significant_set_bit(blue_mask );
            bit_scan_result alpha_shift = find_least_significant_set_bit(alpha_mask);
            
            
            ASSERT(  red_shift.found);
            ASSERT(green_shift.found);
            ASSERT( blue_shift.found);
            ASSERT(alpha_shift.found);
            
            u32 *src_dest = pixels;
            
            for(    s32 y = 0; y < header->height; y++)
            {
                for(s32 x = 0; x < header->width; x++)
                {
                    u32 c = *src_dest;
                    *src_dest  = ((((c >> alpha_shift.index) & 0xFF) << 24)|
                                  (((c >>   red_shift.index) & 0xFF) << 16)|
                                  (((c >> green_shift.index) & 0xFF) <<  8)|
                                  (((c >>  blue_shift.index) & 0xFF) <<  0));
                    src_dest++;
                }
            }
        }
    }
    
    return result;
}

internal void
push_piece(EntityVisiblePieceGroup *group, BitmapData *bitmap,
           V2 dim, V3 offset, V2 align, f32 entity_zc, V4 color)
{
    ASSERT(group->piece_count < ARRAYCOUNT(group->pieces));
    EntityVisiblePiece *piece = group->pieces + group->piece_count++;
    piece->bitmap = bitmap;
    
    V3_scale(group->game_state->meters_to_pixels, &offset);
    V2_sub(offset.xy, align, &offset.xy);
    piece->offset = offset;
    
    piece->entity_zc = entity_zc;
    piece->color = color;
    piece->dim = dim;
    
    return;
} 


internal void
push_bitmap(EntityVisiblePieceGroup *group, BitmapData *bitmap,
            V3 offset, f32 entity_zc, V2 align, f32 alpha)
{
    push_piece(group, bitmap, (V2){0,0}, offset, align, entity_zc, (V4){0,0,0,alpha});
    
    return;
} 


//f32 r, f32 g, f32 b, f32 a
internal void
push_rect(EntityVisiblePieceGroup *group,
          V2 dim, V3 offset, f32 entity_zc,
          V4 color)
{
    //V2_scale(group->game_state->meters_to_pixels, &dim);
    
    push_piece(group, NULLPTR, dim, offset, (V2){0.0f,0.0f}, entity_zc, color);
    
    return;
} 


internal void
draw_hitpoint(EntitySim *entity_sim, EntityVisiblePieceGroup *piece_group)
{
    
    if(entity_sim->hit_point_max >= 1)
    {
        V2 health_dim = { 0.2f, 0.2f};
        f32 spacing_x = 1.5f * health_dim.x;
        f32 first_x = 0.5f *  (entity_sim->hit_point_max - 1) * spacing_x;
        // NOTE(MIGUEL): pixel space. y axis upside down
        V3 hit_position = {-0.5f * (entity_sim->hit_point_max - 1) * spacing_x, 0.5f, 0};
        V2 hit_position_delta = {spacing_x, 0.0f};
        
        for(u32 health_index = 0; health_index < entity_sim->hit_point_max;
            health_index++)
        {
            HitPoint *hitpoint = entity_sim->hit_points + health_index;
            V4 color = { 1.0f, 0.0f, 0.0f, 1.0f };
            
            if(hitpoint->filled_amount == 0)
            {
                color = (V4){ 0.2f, 0.2f, 0.2f, 1.0f };
            }
            
            push_rect(piece_group, health_dim, hit_position, 1.0f, color);
            hit_position.x += hit_position_delta.x;
        }
    }
    
    return;
}

SGE_UPDATE(SGEUpdate)
{
    ASSERT((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
           (ARRAYCOUNT(input->controllers[0].buttons)));
    
    ASSERT(sizeof(GameState) <= sge_memory->permanent_storage_size);
    
    
    GameState *game_state = (GameState *)sge_memory->permanent_storage;
    if(!sge_memory->is_initialized)
    {   
        // NOTE(MIGUEL): Entity at slot 0 reserved as null
        //Entity_create_entity_low(game_state, EntityType_null, NULLPTR);
        
        //~ BITMAP LOADING
        game_state->back_drop = Game_debug_load_bmp(thread,
                                                    sge_memory->debug_platform_read_entire_file,
                                                    "../res/images/test_background.bmp");
        game_state->shadow    = Game_debug_load_bmp(thread,
                                                    sge_memory->debug_platform_read_entire_file,
                                                    "../res/images/test_hero_shadow.bmp");
        
        game_state->tree      = Game_debug_load_bmp(thread,
                                                    sge_memory->debug_platform_read_entire_file,
                                                    "../res/images/tree00.bmp");
        
        game_state->sword      = Game_debug_load_bmp(thread,
                                                     sge_memory->debug_platform_read_entire_file,
                                                     "../res/images/rock03.bmp");
        
        DEBUG_PlatformReadEntireFile *read_file_callback = sge_memory->debug_platform_read_entire_file;
        PlayerBitmaps               *bitmap             = game_state->playerbitmaps;
        
        //RIGHT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_right_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_right_cape.bmp");
        bitmap->align = (V2){72, 182};
        bitmap++;
        
        //BACK
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_back_cape.bmp");
        bitmap->align = (V2){72, 182};
        bitmap++;
        
        //LEFT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_left_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_left_cape.bmp");
        bitmap->align = (V2){72, 182};
        bitmap++;
        
        //FRONT
        bitmap->head  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/shadow_front_head.bmp");
        bitmap->torso = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_torso.bmp");
        bitmap->cape  = Game_debug_load_bmp(thread, read_file_callback, "../res/images/test_hero_front_cape.bmp");
        bitmap->align = (V2){72, 182};
        
        //~ WORLD GENERATION
        
        MemoryArena_init(&game_state->world_arena,
                         sge_memory->permanent_storage_size  - sizeof(GameState),
                         (u8 *)sge_memory->permanent_storage + sizeof(GameState));
        
        
        game_state->world = MEMORY_ARENA_PUSH_STRUCT(&game_state->world_arena, World); 
        World *world  = game_state->world;
        World_init(world, 1.4f); 
        
        u32 tiles_per_chunk_width  = 17;
        u32 tiles_per_chunk_height =  9;
        
        u32 random_number_index = 0;
        
        u16 screen_base_x = 0;
        u16 screen_base_y = 0;
        u16 screen_base_z = 0;
        
        
        u32 screenx = screen_base_x;
        u32 screeny = screen_base_y;
        u32 screenz = screen_base_z;
        
        b32 door_left   = 0;
        b32 door_top    = 0;
        b32 door_right  = 0;
        b32 door_bottom = 0;
        b32 door_up     = 0;
        b32 door_down   = 0;
        
        for(u32 screen_index = 0; screen_index < 1; screen_index++)
        {
            ASSERT(random_number_index < ARRAYCOUNT(random_number_table));
            u32 random_choice;
            if(1) //(door_up || door_down)
            {
                random_choice = random_number_table[random_number_index++] % 2;
                
            }
            else
            {
                random_choice = random_number_table[random_number_index++] % 3;
            }
            b32 created_z_door = 0;
            if(random_choice == 2)
            {
                created_z_door = 1;
                if(screenz  == 0)
                {
                    door_up = 1;
                }
                else
                {
                    door_down = 1;
                    
                }
            }
            else if(random_choice == 1)
            {
                door_right = 1;
            }
            else
            {
                door_top = 1;
            }
            
            for(u32 tiley = 0 ; tiley < tiles_per_chunk_height; tiley++ )
            {
                for(u32 tilex = 0; tilex < tiles_per_chunk_width; tilex++ )
                {
                    u32 tile_abs_x = screenx * tiles_per_chunk_width  + tilex;
                    u32 tile_abs_y = screeny * tiles_per_chunk_height + tiley;
                    
                    u32 tile_value = 1;
                    // MAKE LEFT WALL AND DOOR
                    if((tilex == 0) && (!door_left || (tiley != (tiles_per_chunk_height / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    // MAKE RIGHT WALL AND DOOR
                    if((tilex == (tiles_per_chunk_width - 1)) && (!door_right || (tiley != (tiles_per_chunk_height / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    if ((tiley == 0) && (!door_bottom || (tilex != (tiles_per_chunk_width / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    if((tiley == (tiles_per_chunk_height - 1)) && (!door_top || (tilex != (tiles_per_chunk_width / 2))))
                    {
                        tile_value = 2;
                    }
                    
                    if((tilex == 10) && (tiley == 6))
                    {
                        if(door_up)
                        {
                            tile_value = 3;
                        }
                        if(door_down)
                        {
                            tile_value = 4;
                        }
                    }
                    
                    //debug
                    //tile_value = 1;
                    
                    if(tile_value == 2) 
                    {
                        // WALL
                        Game_add_wall(game_state, tile_abs_x, tile_abs_y, screenz);
                        
                    }
                }
            }
            
            door_left   = door_right;
            door_bottom = door_top;
            
            if(created_z_door)
            {
                door_up   = !door_up;
                door_down = !door_down;
                
            }
            else
            {
                door_up   = 0;
                door_down = 0;
            }
            
            door_right    = 0;
            door_top      = 0;
            
            if(random_choice == 2)
            {
                if(screenz == screen_base_z)
                {
                    screenz = screen_base_z + 1;
                }
                else
                {
                    screenz = screen_base_z;
                }
            }
            else if(random_choice == 1)
            {
                screenx += 1;
            }
            else
            {
                screeny += 1;
            }
        }
        
        WorldCoord initial_camera_position = { 0 };
        
        u32 camera_tile_x = screen_base_x * tiles_per_chunk_width  + 17 / 2;
        u32 camera_tile_y = screen_base_y * tiles_per_chunk_height +  9 / 2;
        u32 camera_tile_z = screen_base_z;
        
        initial_camera_position = World_worldcoord_from_tilecoord(world,
                                                                  camera_tile_x,
                                                                  camera_tile_y,
                                                                  camera_tile_z);
        
        game_state->camera_position = initial_camera_position;
        
        Game_add_hostile(game_state,
                         camera_tile_x + 2,
                         camera_tile_y + 2,
                         camera_tile_z);
        
        for(f32 friendly_index = 0; friendly_index < 2; friendly_index++)
        {
            s32 friendly_offset_x = (random_number_table[random_number_index++] % 10) - 7;
            s32 friendly_offset_y = (random_number_table[random_number_index++] % 10) - 3;
            
            if((friendly_offset_x != 0) &&
               (friendly_offset_y != 0))
            {
                Game_add_friendly(game_state,
                                  camera_tile_x + friendly_offset_x,
                                  camera_tile_y + friendly_offset_y,
                                  camera_tile_z);
            }
        }
        
        sge_memory->is_initialized = 1; 
    }
    
    World *world  = game_state->world;
    
    f32 meters_to_pixels = game_state->meters_to_pixels;
    s32 tile_side_in_pixels = 60;
    game_state->meters_to_pixels    = ((f32)tile_side_in_pixels / (f32)world->side_in_meters_tile);
    
    f32 lower_left_x = -(f32)tile_side_in_pixels / (f32)2;
    f32 lower_left_y =  (f32)back_buffer->height;
    
    // ************************************************
    // INPUT RESPONSE
    //*************************************************
    for(u32 controller_index = 0; controller_index < 5; controller_index++)
    {
        GameControllerInput *controller = &input->controllers[controller_index];
        
        ControlledPlayer *player = game_state->controlled_players + controller_index;
        
        if(player->entity_index == 0)
        {
            if(controller->button_start.ended_down)
            { 
                memset(player, 0, sizeof(player));
                player->entity_index = Game_add_player(game_state, controller_index).index_low; 
            }
        }
        else
        {
            player->acceleration = (V2){ 0.0f, 0.0f };
            
            if(controller->is_analog)
            {
                // NOTE(MIGUEL): use analog mov tunning
                player->acceleration = (V2){ controller->stick_avg_x,controller->stick_avg_y };
                
            }
            else
            {
                if(controller->button_y.ended_down)
                {
                    // up
                    player->acceleration.y = 1.0f;
                }
                if(controller->button_a.ended_down)
                {
                    // down
                    player->acceleration.y = -1.0f;
                }
                
                if(controller->button_x.ended_down)
                {
                    // left
                    player->acceleration.x = -1.0f;
                }
                
                if(controller->button_b.ended_down)
                {
                    // right
                    player->acceleration.x = 1.0f;
                }
            }
            
            player->delta_sword = (V2){ 0.0f, 0.0f };
            if(controller->action_up.ended_down)
            {
                player->delta_sword.y =  1.0f;
            }
            if(controller->action_down.ended_down)
            {
                player->delta_sword.y = -1.0f;
            }
            if(controller->action_left.ended_down)
            {
                player->delta_sword.x = -1.0f;
            }
            if(controller->action_right.ended_down)
            {
                player->delta_sword.x =  1.0f;
            }
            
            if(controller->button_start.ended_down)
            {
                player->delta_z = 3.0f;
            }
            if(controller->shoulder_left.ended_down)
            {
                // TODO(MIGUEL): fix my beloved camera pan to other player code :(
                // focus camera on this player
                // NOTE(MIGUEL): the following code is from the "set camera" old system
                //game_state->camera_following_entity_index = game_state->player_controller_entity_index[controller_index];
            }
        }
        
    } /// END OF INPUT LOOP
    
    V2 view_tile_span = { 17, 9 };
    V2_scale(3.0f, &view_tile_span); // NOTE(MIGUEL): broaden view span for simspace
    V2_scale(world->side_in_meters_tile, &view_tile_span);
    V2_scale(0.5f, &view_tile_span);
    
    RectV2 high_frequency_bounds = RectV2_center_half_dim((V2){0 , 0},
                                                          view_tile_span);
    MemoryArena sim_arena = { 0 };
    
    MemoryArena_init(&sim_arena,
                     sge_memory->transient_storage_size,
                     (u8 *)sge_memory->transient_storage);
    
    SimRegion *sim_region = SimRegion_begin_sim(&sim_arena,
                                                game_state,
                                                game_state->world,
                                                game_state->camera_position,
                                                high_frequency_bounds,
                                                back_buffer);
    
    //ASSERT(validate_sim_entities(sim_region));
    
    
    /// debug purp background clear
    Game_draw_rectangle(back_buffer,
                        (V2){0.0f, 0.0f} ,
                        (V2){(f32)back_buffer->width,(f32)back_buffer->height},
                        (V3){0.4f, 0.8f, 1.0f},
                        0);
    
    
    game_state->clock += input->delta_t * 0.1f;
    // Game_render_weird_shit(back_buffer, 10, 40, game_state->clock);
    
    //~DRAW ALL HIGH ENTITIES
    
    V2 screen_center = 
    {
        0.5f * (f32)back_buffer->width,
        0.5f * (f32)back_buffer->height,
    };
    
    EntityVisiblePieceGroup piece_group = { 0 };
    piece_group.game_state = game_state;
    
    EntitySim *entity_sim = sim_region->entities;
    for(u32 entity_index = 1;
        entity_index < sim_region->entity_count;
        entity_index++, entity_sim++)
    {
        piece_group.piece_count = 0;
        
        V2 entity_bottom_right = { 0 };
        
        
        // JUMP CODE
        f32 shadow_alpha = 1.0f - 0.5f * entity_sim->z;
        
        if(shadow_alpha < 0.0f)
        {
            shadow_alpha = 0.0f;
        }
        // END OF JUMP CODE
        
        
        switch(entity_sim->type )
        {
            
            case EntityType_player:
            {
                for(u32 player_index = 0;
                    player_index < ARRAYCOUNT(game_state->controlled_players);
                    player_index++)
                {
                    ControlledPlayer *player = game_state->controlled_players + player_index;
                    
                    // NOTE(MIGUEL): these index naming differences are fucked!!!
                    //                what array do they refer to ????
                    if(entity_sim->index_storage == player->entity_index)
                    {
                        MoveSpec movespec = default_movespec();
                        movespec.unitmaxaccel = 1;
                        movespec.speed = 300.0f;
                        movespec.drag  = 12.0f;
                        
                        
                        SimRegion_move_entity(sim_region, entity_sim, &movespec, input->delta_t, player->acceleration);
                        
                        if((player->delta_sword.x != 0.0f) || (player->delta_sword.y != 0.0f))
                        {
                            EntitySim *sword = entity_sim->sword.ptr;
                            
                            if(sword)
                            {
                                sword->position = entity_sim->position;
                                sword->distance_remaining = 5.0f;
                                V2 delta_sword = player->delta_sword;
                                V2_scale(5.0f, &delta_sword);
                                sword->velocity = delta_sword;
                            }
                        }
                    }
                }
                
                PlayerBitmaps *playerbitmaps = &game_state->playerbitmaps[entity_sim->facing_direction];
                push_bitmap(&piece_group, &game_state->shadow  , (V3){0, 0, 0}, shadow_alpha, playerbitmaps->align, 1.0f);
                push_bitmap(&piece_group, &playerbitmaps->torso, (V3){0, 0, entity_sim->z}, 1.0f        , playerbitmaps->align, 1.0f);
                push_bitmap(&piece_group, &playerbitmaps->cape , (V3){0, 0, entity_sim->z}, 1.0f        , playerbitmaps->align, 1.0f); 
                push_bitmap(&piece_group, &playerbitmaps->head , (V3){0, 0, entity_sim->z}, 1.0f        , playerbitmaps->align, 1.0f);
                
                draw_hitpoint(entity_sim, &piece_group);
            } break;
            case EntityType_wall:
            {
                push_bitmap(&piece_group, &game_state->tree, (V3){0, 0, 0}, 0.0f, (V2){40, 80}, 1.0f);
                
            } break;
            case EntityType_sword:
            {
                Entity_update_sword(sim_region, entity_sim, input->delta_t);
                push_bitmap(&piece_group, &game_state->shadow, (V3){0, 0, 0}, 1.0f, (V2){72, 182}, 1.0f);
                push_bitmap(&piece_group, &game_state->sword , (V3){0, 0, 0}, 0.0f, (V2){29, 10}, 1.0f);
            } break;
            
            case EntityType_friendly: 
            {
                Entity_update_friendly(sim_region, entity_sim, input->delta_t);
                entity_sim->bob_t += 2.0f * input->delta_t;
                if(entity_sim->bob_t > (2.0f * PI_32BIT))
                {
                    entity_sim->bob_t -= 2.0f * PI_32BIT;
                }
                PlayerBitmaps *playerbitmaps = &game_state->playerbitmaps[entity_sim->facing_direction];
                f32 bobsin = math_sin(entity_sim->bob_t);
                push_bitmap(&piece_group, &game_state->shadow , (V3){0, 0, 0}, (0.5f * shadow_alpha) + 0.2f * bobsin, playerbitmaps->align, 1.0f);
                push_bitmap(&piece_group, &playerbitmaps->head, (V3){0, 0, 1.0f * bobsin}, 1.0f, playerbitmaps->align, 1.0f);
                
                draw_hitpoint(entity_sim, &piece_group);
            } break;
            case EntityType_hostile: 
            {
                Entity_update_hostile(sim_region, entity_sim, input->delta_t);
                PlayerBitmaps *playerbitmaps = &game_state->playerbitmaps[entity_sim->facing_direction];
                push_bitmap(&piece_group, &game_state->shadow  , (V3){0, 0, 0}, shadow_alpha, playerbitmaps->align, 0.0f);
                push_bitmap(&piece_group, &playerbitmaps->torso, (V3){0, 0, 0},  1.0f, playerbitmaps->align, 0.0f);
                
            } break;
            default:
            {
                INVALID_CODE_PATH;
            } break;
        }
        
        f32 acceleration = -9.8f;
        
        // JUMP CODE
        f32 z = -entity_sim->z;
        entity_sim->z = 0.5f * acceleration * square(input->delta_t) + entity_sim->delta_z * input->delta_t + entity_sim->z;
        entity_sim->delta_z = acceleration * input->delta_t + entity_sim->delta_z;
        
        if(entity_sim->z < 0.0f)
        {
            entity_sim->z = 0.0f;
        }
        // END OF JUMP CODE
        
        f32 entity_ground_point_x = screen_center.x + meters_to_pixels * entity_sim->position.x;
        f32 entity_ground_point_y = screen_center.y - meters_to_pixels * entity_sim->position.y;
        
        
        for(u32 piece_index = 0; piece_index < piece_group.piece_count; piece_index++)
        {
            EntityVisiblePiece *piece = piece_group.pieces + piece_index;
            
            V2 center =
            {
                entity_ground_point_x + piece->offset.x,
                entity_ground_point_y + piece->offset.y + piece->offset.z
            }; 
            
            if(piece->bitmap)
            {
                
                Game_draw_bitmap(back_buffer, piece->bitmap,
                                 center.x,
                                 center.y,
                                 piece->color.a);
            }
            else
            {
                V2 halfdim = piece->dim ;
                V2_scale(0.5f, &halfdim);
                V2 point[2];
                V2_sub(center, halfdim, &point[0]);
                V2_add(center, halfdim, &point[1]);
                
                Game_draw_rectangle(back_buffer,
                                    point[0],
                                    point[1],
                                    (V3){piece->color.r, piece->color.g, piece->color.b},
                                    0);
            }
        }
    }
    //UX1
    //SimRegion DEBUG
    V3 color  = { 0 };
    V3 red    = {1.0f, 0.0f, 0.0f};
    V3 green  = {0.0f, 1.0f, 0.0f};
    V3 blue   = {0.0f, 0.0f, 1.0f};
    V3 yellow = {1.0f, 1.0f, 0.0f};
    V3 purple = {1.0f, 0.0f, 1.0f};
    V3 grey   = {0.6f, 0.6f, 0.6f};
    V3 white  = {1.0f, 1.0f, 1.0f};
    V3 black  = {0.0f, 0.0f, 0.0f};
    V3 dark_gray = {0.3f, 0.3f, 0.3f};
    
    EntitySim *entity = sim_region->entities;
    
    f32 array_debug_zoom = 86.0f;
    f32 padding = 10.0f;
    V2  offset  = {padding, padding};
    f32 height  = (f32)floor_f32_to_s32((back_buffer->height - (padding * 2)) / (f32)(sim_region->max_entity_count * 0.001f * (100.1f - array_debug_zoom)));
    f32 width   = 200.0f;
    
    for(u32 entity_index = 0;
        entity_index < sim_region->max_entity_count;
        entity_index++, entity++)
    {
        V3 divider_color = black;
        V3 empty_color = green;
        
        if(entity_index <= sim_region->entity_count)
        {
            switch(entity->type)
            {
                case EntityType_player:
                {
                    color = blue;
                } break;
                
                case EntityType_friendly:
                {
                    color = yellow;
                } break;
                
                case EntityType_hostile:
                {
                    color = red;
                } break;
                
                case EntityType_wall:
                {
                    color = purple;
                } break;
                
                case EntityType_sword:
                {
                    color = grey;
                } break;
                
                case EntityType_null:
                {
                    color = dark_gray;
                } break;
                
            }
            
            
            Game_draw_rectangle(back_buffer,
                                (V2){ padding, height * (f32)entity_index + padding},
                                (V2){width + padding, height * ((f32)entity_index + 1.0f) + padding},
                                color, 0);
            
            Game_draw_rectangle(back_buffer,
                                (V2){ padding, (height * (f32)entity_index + padding) + (height * 0.90f)},
                                (V2){width + padding, height * ((f32)entity_index + 1.0f) + padding},
                                divider_color, 0);
            
            
            continue;
        }
        
        Game_draw_rectangle(back_buffer,
                            (V2){ padding, height * (f32)entity_index + padding},
                            (V2){width + padding, height * ((f32)entity_index + 1.0f) + padding},
                            empty_color, 0);
        
        Game_draw_rectangle(back_buffer,
                            (V2){ padding, (height * (f32)entity_index + padding) + (height * 0.95f)},
                            (V2){width + padding, height * ((f32)entity_index + 1.0f) + padding},
                            divider_color, 0);
        
    }
    
    
#if 1
    Game_draw_mini_map(game_state,
                       back_buffer,
                       sim_region,
                       4,
                       (V2){120.0f, 80.0f},
                       300, 140,
                       0.5f);
#endif
    
    
    SimRegion_end_sim  (sim_region, game_state);
    
    return;
}


internal void Game_update_sound_buffer(GameState *game_state, GameSoundOutputBuffer *sound_buffer, u32 tone_hz)
{
    local_persist f32 t_sin;
    
    u16 tone_volume = 100;
    u16 wave_period = (u16)(sound_buffer->samples_per_second / tone_hz);
    
    u16 *sample_out = sound_buffer->samples;
    
    for(u32 sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++)
    {
#if 0
        f32 sine_value   = sin(t_sin);
        u16 sample_value = (u16)(sine_value * tone_volume);
#else
#endif
        u16 sample_value = 0;
        
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;
        
#if 0
        t_sin += 2.0f * PI_32BIT * 1.0f / (f32)wave_period;
#endif
    }
    
    return;
} 


internal void
Game_draw_rectangle(GameBackBuffer *buffer,
                    V2 min, V2 max,
                    V3 color_vec, b32 grid)
{
    f32 r = color_vec.x; 
    f32 g = color_vec.y;
    f32 b = color_vec.z;
    
    /// rounding / ruling
    s32 min_x = round_f32_to_s32(min.x);
    s32 min_y = round_f32_to_s32(min.y);
    
    s32 max_x = round_f32_to_s32(max.x);
    s32 max_y = round_f32_to_s32(max.y);
    
    /// clipping
    // NOTE(MIGUEL): will right to but not including the final row 
    if(min_x < 0)
    {
        min_x = 0;
    }
    if(min_y < 0)
    {
        min_y = 0;
    }
    if(max_x > buffer->width)
    {
        max_x = buffer->width;
    }
    if(max_y > buffer->height)
    {
        max_y = buffer->height;
    }
    
    /// bit pattern 0x AA RR GG BB
    u32 color = ((round_f32_to_u32(r * 255.0f) << 16) |
                 (round_f32_to_u32(g * 255.0f) <<  8) |
                 (round_f32_to_u32(b * 255.0f) <<  0));
    
    /// drawing
    u8 *line =  ((u8 *)  (buffer->data)            + 
                 (min_x * buffer->bytes_per_pixel) +
                 (min_y * buffer->pitch));
    
    for(s32 y = min_y; y < max_y; y++ )
    {
        u32 *pixel = (u32 *)line;
        
        for(s32 x = min_x; x < max_x; x++)
        {
            *pixel = color;
            
            //debug
            if(grid)
            {
                
                if(x > (max_x - 2))
                {
                    *pixel = 0xAAaaaaaa;
                }
                if(y > (max_y - 2))
                {
                    *pixel = 0xAAaaaaaa;
                }
            }
            
            pixel++;
        }
        
        line += buffer->pitch;
    }
    
    return;
}

#if 0
internal void Game_render_weird_shit(GameBackBuffer *buffer, s32 x_offset, s32 y_offset, f32 time)
{
    u8 *line = ((u8*)buffer->data);
    
    for(s32 y = 0; y < buffer->height; y++) 
    {
        
        u32 *pixel  = (u32 *)line;
        
        for(s32 x = 0; x < buffer->width; x++) 
        {
            u32 fragment = 2;
            
            f32 alpha_channel = 0.5f;
            f32 red_channel   = 0.0f;
            f32 green_channel = 0.0f;
            f32 blue_channel  = math_cos(time );
            
#if 1
            f32 amplitude = 0.0001f;
            s32 crest = 20;
            local_persist u64 move = 0;
            move += 5000;
            RectV2 bounds = { {0 ,0}, {(f32)buffer->width, (f32)buffer->height} };
            
            //printf("N-Y: %f | X-CMP: %f  \n", y_comp, x_comp);
            if(((floor_f32_to_s32(math_cos(time * 2.0f * ((move + x) * 0.04f) * amplitude) * (f32)buffer->height)) == y))
            {
                //alpha_channel = 220;
                red_channel   = math_cos(time);
                green_channel += 0.9f;
                blue_channel  += 1.0f;
            } 
            
            
            if((floor_f32_to_s32(math_sin(time * 1.0f * (x * 0.001f)) * (f32)buffer->height) == y))
            {
                alpha_channel = 1.0f;
                red_channel   = 0.4f;
                green_channel = 0.3f;
                blue_channel  = 0.9f;
                
            }
#endif
            //ALPHA BLENDING
            u8 frag_red   = (u8)(255.0f *   red_channel);
            u8 frag_green = (u8)(255.0f * green_channel);
            u8 frag_blue  = (u8)(255.0f *  blue_channel);
            
            u8 pixel_red   = (u8)((*pixel & 0x00FF0000) >> 16);
            u8 pixel_green = (u8)((*pixel & 0x0000FF00) >>  8);
            u8 pixel_blue  = (u8)((*pixel & 0x000000FF) >>  0);
            
            fragment = ((u32)                                        ((u32)(    255.0f * alpha_channel)  << 24) |
                        ((u32)(pixel_red   * (1.0f - alpha_channel) + (u32)(  frag_red * alpha_channel)) << 16) |
                        ((u32)(pixel_green * (1.0f - alpha_channel) + (u32)(frag_green * alpha_channel)) <<  8) |
                        ((u32)(pixel_blue  * (1.0f - alpha_channel) + (u32)( frag_blue * alpha_channel)) <<  0));
            
            *pixel = fragment;
            
            pixel++;
        }
        
        line += buffer->pitch;
    }
    
    return;
}
#endif

