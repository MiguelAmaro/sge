/* date = December 29th 2020 6:40 pm */

#ifndef SGE_MEMORY_H
#define SGE_MEMORY_H

#define KILOBYTES(size) (size * 1024)
#define MEGABYTES(size) (KILOBYTES(size) * 1024)
#define GIGABYTES(size) (MEGABYTES(size) * 1024)

// TODO(MIGUEL): Not being used
typedef struct 
{
    b32   Is_Initialized;
    u64   Permenant_Storage_Size;
    void *Permenant_Storage;
    u64   Transient_Storage_Size;
    void *Transient_Storage;
    
}Game_Memory;


#endif //SGE_MEMORY_H
