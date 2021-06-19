#include "SGE_FileIO.h"

#include "stdio.h"
#include "stdlib.h"

u32 
FileIO_GetFileSize(readonly u8 *path) 
{
    FILE *File;
    File = fopen(path, "r");
    ASSERT(File);
    
    fseek(File, 0L, SEEK_END);
    u32 size_bytes = ftell(File);
    
    return size_bytes;
}


void 
BuildPrefixSuffixTable(u32* table, u32 table_size,readonly u8 *SearchTerm)
{
    //printf("Before: %p \n", table);
    //printf("Before: %p \n", table);
    u32* tableOG = table;
    u32 i = 1;
    u32 j = 0;
    
    for(u32 i = 0; i < table_size; i++)
    {
        *(table + i) = 0;
    }
    
    
    while(i < (table_size))
    {
        // LAST ELEMENT
        if(i == (table_size - 1))
        {
            if(*(SearchTerm + i) == *(SearchTerm + j) || j == 0)
            {
                break;
            }
            while(*(SearchTerm + i) != *(SearchTerm + j) && j != 0)
            {
                j = *(table + j - 1);
            }
            *(table + i) = ++j;
            i++;
        }
        // MISMATCH
        else if(*(SearchTerm + i) != *(SearchTerm + j))
        {
            *(table + i) = j = 0;
            i++;
            //printf("Searchi : %c  <- i : %d| Searchj : %c <- j : %d | PSST %d \n", *(SearchTerm + i), i, *(SearchTerm + j), j, *(table + i));
        }
        // MATCH
        else
        {
            //printf("Searchi : %c  <- i : %d| Searchj : %c <- j : %d | PSST %d \n", *(SearchTerm + i), i, *(SearchTerm + j), j, *(table + i));
            j++;
            *(table + i) = j;
            i++;
            
        }
    }
    
    ASSERT(table == tableOG);
    
    return;
}


// TODO(MIGUEL): Take out file and use u8 * instead
u32 
StringMatchKMP(readonly u8 *Text, readonly u32 BytesToRead, readonly u8 *SearchTerm)
{
    // NOTE(MIGUEL): This implemenation only returns the first match!!
    readonly u32 SearchTermLength = (u32)strlen(SearchTerm);
    u32 result = 0;
    u32 i = 0;
    u32 j = 0;
    
    u32 *table     = calloc(SearchTermLength, sizeof(u32)); 
    u32 *tableCopy = table;
    
    ASSERT(Text && table);
    
    
    BuildPrefixSuffixTable(table , SearchTermLength, SearchTerm);
    //for(u32 i = 0; i < SearchTermLength ; i++) { printf("%d ", *(table + i));}
    
    //printf("Before: %p \n", table);
    //printf(SearchTerm);
    
    while(i < BytesToRead)
    {
        //printf("i : %d | j : %d | Text : %c | | Pattern : %c \n",i, j, (u8)*(Text + i), (u8)*(SearchTerm + j));
        
        // CASE: Match
        
        if(*(SearchTerm + j) == *(Text + i))
        {
            i++;
            j++;
        }
        
        if(j == SearchTermLength)
        {
            result = (i - j);
            //printf("MATCH: i = %d - %d = j = %d \n", i , j, (i - j));
            //j = *(table + j - 1);
            break;
        }
        
        // CASE: Mis-match
        
        else if( (i < BytesToRead) && (*(SearchTerm + j) != *(Text + i)))
        {
            if(j != 0)
            {
                j = *(table + j - 1);
                
            }
            else
            {
                i++;
            }
        }
    }
    //printf("MATCH: i = %d - %d = j = %d \n", i , j, (i -j));
    //printf("After : %p \n", table);
    
    ASSERT(table == tableCopy);
    
    free(table);
    
    return (i - j);
}

