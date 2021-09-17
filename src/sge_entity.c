#include "sge_entity.h"



inline MoveSpec
default_movespec(void)
{
    MoveSpec result = { 0 };
    result.unitmaxaccel = 0;
    result.speed = 1.0f;
    result.drag  = 0.0f;
    
    return result;
}
