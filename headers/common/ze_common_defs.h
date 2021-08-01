#ifndef ZE_COMMON_DEFS_H
#define ZE_COMMON_DEFS_H

#include <stdint.h>	// for types
#include <stdio.h>	// for printf
#include <string.h>	// for memcpy

///////////////////////////////////////////////////////////////////////
// PRIMITIVE TYPES
///////////////////////////////////////////////////////////////////////
#define f32 float
#define f64 double

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#define u8 uint8_t
#define uChar uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define bool32 uint32_t

#define YES 1
#define NO 0

typedef i64 frameInt;
typedef f64 timeFloat;
typedef i32 zErrorCode;
typedef i32 zeHandle;

static_assert(sizeof(char) == 1, "Code requires char size == 1");
static_assert(sizeof(i8) == 1, "Code requires i8 size == 1");
static_assert(sizeof(u8) == 1, "Code requires u8 size == 1");
static_assert(sizeof(i16) == 2, "Code requires i16 size == 2");
static_assert(sizeof(u16) == 2, "Code requires u16 size == 2");
static_assert(sizeof(i32) == 4, "Code requires int size == 4");
static_assert(sizeof(u32) == 4, "Code requires uint size == 4");

static_assert(sizeof(f32) == 4, "Code requires f32 size == 4");
static_assert(sizeof(f64) == 8, "Code requires f64 size == 8");

#define internal static
#define local_persist static

//#define ze_internal static
#ifndef ze_internal
#define ze_internal

#endif

// Quick cross platform assert. Read from address zero if expression is false
// TODO: Message box assert with a print of __FILE__, __LINE__ and __TIME__ possible?
// Yes, yes it is, eg:
/* 
#define log_message(guard,format,...) \
if (guard) printf("%s:%d: " format "\n", __FILE__, __LINE__,__VA_ARGS_);

log_message( foo == 7, "x %d", x)
*/

/*
#if PARANOID
#define Assert(expression) if(!(expression)) {*(int *)0 = 0; }
#else
#define Assert(expression)
#endif
#define AssertAlways(expression) if(!(expression)) { *(int *)0 = 0; }
*/

#define ZE_FORCE_SEG_FAULT \
	if (true)              \
	{                      \
		*(int *)0 = 0;     \
	}

#define KiloBytes(bytes) ((bytes)*1024LL)
#define MegaBytes(bytes) (KiloBytes(bytes) * 1024LL)
#define BytesAsKB(bytes) (bytes / 1024LL)
#define BytesAsMB(bytes) (bytes / (1024LL * 1024LL))

#define F32_EPSILON 1.19209290E-07F // decimal constant
#define ZALMOST_ZERO(value) (value < F32_EPSILON && value > -F32_EPSILON)

// shorten some bitwise stuff
#define IF_BIT(uintFlags, uintBit) ((uintFlags & uintBit) != 0)
#define IF_TO_BIT(bitCondition, uintFlags, uintBit) \
	{                                               \
		if (bitCondition)                           \
		{                                           \
			uintFlags |= uintBit;                   \
		}                                           \
		else                                        \
		{                                           \
			uintFlags &= ~uintBit;                  \
		}                                           \
	}

#define pi32 3.14159265359f
#define DEG2RAD 3.141593f / 180.0f
#define RAD2DEG 57.2958f
#define FULL_ROTATION_RADIANS (pi32 * 2)
#define Z_INFINITY 0x7F800000

#define ZABS(value) (value = (value >= 0 ? value : -value))
#define ZMIN(x, y) ((x) < (y) ? (x) : (y))
#define ZMAX(x, y) ((x) > (y) ? (x) : (y))

internal f32 ZAbsf(f32 value)
{
	return value >= 0 ? value : -value;
}

// convert a 2d position to an index in a linear buffer
#define ZE_2D_INDEX(positionX, positionY, gridWidth) \
	positionX + (positionY * gridWidth)

#define ze_external extern "C"

// used for serialise/deserialise validation
#define ZE_SENTINEL 0xDEADBEEF
#define ZE_SENTINEL_B 0xEFBEBEEF

#define ZE_DEBUG_PORT 59594
#define ZE_MONITOR_PORT 59595

#define DLL_EXPORT __declspec(dllexport)

#define ZE_BUILD_STRING(stringBufName, stringBufSize, stringFormat, ...) \
	char stringBufName##[##stringBufSize##];                             \
	snprintf(##stringBufName##, stringBufSize##, stringFormat##, ##__VA_ARGS__##);

// printFunc must match signature void Func(char* str);
#define ZE_CALL_PRINT(printFunc, stringBufSize, stringFormat, ...)             \
	{                                                                          \
		char stringBuf[##stringBufSize##];                                     \
		snprintf(stringBuf, stringBufSize##, stringFormat##, ##__VA_ARGS__##); \
		printFunc##(stringBuf);                                                \
	}

///////////////////////////////////////////////////////////////////////
// Buffer macros
///////////////////////////////////////////////////////////////////////

inline u32 ZE_Copy(void *dest, void *source, u32 numBytes)
{
	memcpy(dest, source, numBytes);
	return numBytes;
}

#ifndef ZE_CREATE_CAST_PTR
#define ZE_CREATE_CAST_PTR(ptr, newPtrStructType, newPtrName) \
newPtrStructType* newPtrName = (newPtrStructType*)ptr;
#endif

// I hate writing casts all the time okay?
// returns amount of bytes copied. so you can do
// readPosition += ZE_COPY(readPosition, writePosition, sizeof(SomeStruct));
#if 1
#ifndef ZE_COPY
#define ZE_COPY(ptrSource, ptrDestination, numBytes) \
	ZE_Copy((void *)##ptrDestination##, (void *)##ptrSource##, numBytes##);
#endif
#endif

#if 0
#ifndef ZE_COPY
#define ZE_COPY(ptrSource, ptrDestination, numBytes) \
	ZE_COPY\((u8 *)##ptrSource##, (u8 *)##ptrDestination##, numBytes##)
#endif
#endif

// copy but automatically do a size of
#ifndef ZE_COPY_STRUCT
#define ZE_COPY_STRUCT(ptrSource, ptrDestination, structType) \
	ZE_Copy((void *)##ptrDestination##, (void *)##ptrSource##, sizeof(##structType##));
#endif

#ifndef ZE_SET_ZERO
#define ZE_SET_ZERO(ptrToMemory, numberOfBytesToZero) \
	memset(##ptrToMemory, 0, numberOfBytesToZero##);
#endif

	///////////////////////////////////////////////////////////////////////
	// Error handling
	///////////////////////////////////////////////////////////////////////
	typedef int ErrorCode;

#define ZE_ERROR_NONE 0

#define ZE_ERROR_BAD_INDEX -1
#define ZE_ERROR_UNKNOWN 1
#define ZE_ERROR_UNSUPPORTED_OPTION 2
#define ZE_ERROR_OPEN_SOCKET_FAILED 3
#define ZE_ERROR_MISSING_FILE 4
#define ZE_ERROR_UNKNOWN_COMMAND 5
#define ZE_ERROR_NO_SPACE 6
#define ZE_ERROR_SERIALISE_FAILED 7
#define ZE_ERROR_DESERIALISE_FAILED 8
#define ZE_ERROR_BAD_SIZE 9
#define ZE_ERROR_NOT_FOUND 10
#define ZE_ERROR_BAD_ARGUMENT 11
#define ZE_ERROR_NULL_ARGUMENT 12
#define ZE_ERROR_NOT_IMPLEMENTED 13
#define ZE_ERROR_ALLOCATION_FAILED 14
#define ZE_ERROR_OPERATION_FAILED 15
#define ZE_ERROR_FUNC_RAN_AWAY 16
#define ZE_ERROR_OUT_OF_BOUNDS 17
#define ZE_ERROR_LINK_UP_FAILED 18
#define ZE_ERROR_TEST_FAILED 19
#define ZE_ERROR_STRING_TOO_LONG 20

typedef void (*ZE_FatalErrorFunction)(const char *message);
typedef void* (*ZE_mallocFunction)(const size_t numBytes);
typedef void (*ZE_freeFunction)(const void * memory);

static ZE_FatalErrorFunction ze_fatalErrorFunc = NULL;

static void ZE_SetFatalError(ZE_FatalErrorFunction func)
{
	if (ze_fatalErrorFunc != NULL)
	{
		printf("ZE Error handler already set\n");
		return;
	}
	printf("ZE Set error handler\n");
	ze_fatalErrorFunc = func;
}

static void ZE_Fatal(char *msg)
{
	if (ze_fatalErrorFunc == NULL)
	{
		printf("FATAL - %s\n", msg);
		ZE_FORCE_SEG_FAULT;
	}
	ze_fatalErrorFunc(msg);
}

#define ZE_ASSERT(expression, msg)                                         \
	if (!(expression))                                                     \
	{                                                                      \
		char assertBuf[512];                                               \
		snprintf(assertBuf, 512, "%s, %d: %s\n", __FILE__, __LINE__, msg); \
		ZE_Fatal(assertBuf);                                               \
	}

#define ILLEGAL_CODE_PATH \
	ZE_ASSERT(NO, "Illegal Code Path")

struct DateTime
{
	i32 year;
	i32 month;
	i32 dayOfTheMonth;
	i32 dayOfTheWeek;

	i32 hour;
	i32 minute;
	i32 second;
};

struct PlatformTime
{
	timeFloat deltaTime;
	f32 sessionEllapsed;
	u32 frameNumber;
};

#endif // ZE_COMMON_DEFS_H
