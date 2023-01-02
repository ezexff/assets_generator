/*
Функция CreateModelFile(...) создайт файл с моделью вида:
    Name
    Meshes [1 to n]
        Name
        Vertices
        Material
        BoneOfssets (local transforms)
        BoneTransformsHierarchy (global transforms)
        Animations [1 to n]
*/

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure
#include <stdio.h>
#include <stdlib.h>

#include "engine_types.h"
#include "engine_intrinsics.h"
#include "engine_math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb/stb_image.h"          // Bitmaps read
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb/stb_image_write.h"    // Bitmaps write

internal string
PushString(char* Source)
{
    string Result;

    Result.Count = StringLength(Source);
    if (Result.Count > 0)
    {
        Result.Data = (u8*)malloc(Result.Count + 1);
        for (u32 i = 0; i < Result.Count; i++)
        {
            Result.Data[i] = Source[i];
        }
        Result.Data[Result.Count] = '\0';
    }
    else
    {
        Result.Data = NULL;
    }

    return(Result);
}

inline b32
operator==(string A, string B)
{
    b32 Result = false;

    if (A.Count == B.Count)
    {
        for (u32 i = 0; i < A.Count; i++)
        {
            if (A.Data[i] != B.Data[i])
            {
                return (Result);
            }
        }
        Result = true;
    }

    return(Result);
}

void WriteStringIntoFile(string* Source, FILE* Out)
{
    fwrite(&Source->Count, sizeof(u64), 1, Out);
    for (u64 j = 0; j < Source->Count; j++)
    {
        fwrite(&Source->Data[j], sizeof(char), 1, Out);
    }
}

void WriteAiStringIntoFile(aiString Source, FILE* Out)
{
    u64 StringSize = (u64)Source.length;
    fwrite(&StringSize, sizeof(u64), 1, Out);
    for (u32 j = 0; j < StringSize; j++)
    {
        fwrite(&Source.data[j], sizeof(char), 1, Out);
    }
}