/* date = January 8th 2021 4:16 pm */

#ifndef FLIGHTCONTROL_ENTITY_H
#define FLIGHTCONTROL_ENTITY_H
// NOTE(MIGUEL): Renderer is a cricular include (included in entity)
//#include "FlightControl_Renderer.h"

typedef struct 
{
    f32 *vertices;
    u32 *indices;
    //u32  vertex_Buffer;
    //u32  element_Buffer;
    //u32  vertex_Attributes;
} Entity;


#endif //FLIGHTCONTROL_ENTITY_H
