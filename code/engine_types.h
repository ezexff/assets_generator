#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;

typedef uintptr_t umm;
typedef intptr_t smm;

typedef b32 b32x;
typedef u32 u32x;

#define flag8(type) u8
#define flag16(type) u16
#define flag32(type) u32
#define flag64(type) u64

#define enum8(type) u8
#define enum16(type) u16
#define enum32(type) u32
#define enum64(type) u64

#define U16Max 65535
#define S32Min ((s32)0x80000000)
#define S32Max ((s32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f
#define Pi32_2 1.57079632679f // pi/2 TODO(me): remove?
#define Pi32_4 0.78539816340f // pi/4 TODO(me): remove?
#define Tau32 6.28318530717958647692f

#if ENGINE_SLOW
#define Assert(Expression)                                                                                             \
    if(!(Expression))                                                                                                  \
    {                                                                                                                  \
        *(int *)0 = 0;                                                                                                 \
    }
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase                                                                                             \
    default: {                                                                                                         \
        InvalidCodePath;                                                                                               \
    }                                                                                                                  \
    break

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(me): swap, min, max (add if need)

#define AlignPow2(Value, Alignment) ((Value + ((Alignment)-1)) & ~((Alignment)-1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

//
// NOTE(me): String
//

struct buffer
{
    umm Count;
    u8 *Data;
};
typedef buffer string;

inline u32 StringLength(char *String)
{
    u32 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }

    return (Count);
}

//
// NOTE(me): Math vectors
//

union v2 {
    struct
    {
        real32 x, y;
    };
    real32 E[2];
};

union v3 {
    struct
    {
        real32 x, y, z;
    };
    struct
    {
        real32 r, g, b;
    };
    struct
    {
        v2 xy;
        real32 Ignored0_;
    };
    struct
    {
        real32 Ignored1_;
        v2 yz;
    };
    real32 E[3];
};

union v4 {
    struct
    {
        union {
            v3 xyz;
            struct
            {
                real32 x, y, z;
            };
        };

        real32 w;
    };
    struct
    {
        union {
            v3 rgb;
            struct
            {
                real32 r, g, b;
            };
        };

        real32 a;
    };
    struct
    {
        v2 xy;
        real32 Ignored0_;
        real32 Ignored1_;
    };
    struct
    {
        real32 Ignored2_;
        v2 yz;
        real32 Ignored3_;
    };
    struct
    {
        real32 Ignored4_;
        real32 Ignored5_;
        v2 zw;
    };
    real32 E[4];
};

//
// NOTE(me): Matrices
//

struct m4x4
{
    // NOTE(me): These are stored ROW MAJOR - E[ROW][COLUMN]!!!
    f32 E[4][4];
};

struct m4x4_inv
{
    m4x4 Forward;
    m4x4 Inverse;
};