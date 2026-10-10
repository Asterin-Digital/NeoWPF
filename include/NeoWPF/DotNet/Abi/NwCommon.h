/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>

#define NW_ABI_MAJOR 1u
#define NW_ABI_MINOR 0u
#define NW_ABI_VERSION ((NW_ABI_MAJOR << 16) | NW_ABI_MINOR)

#if defined(_WIN32) || defined(__linux__)
#define NW_CALL
#else
#define NW_CALL
#endif

typedef int32_t NwStatus;

enum NwStatusCode {
    NwStatus_Ok = 0,
    NwStatus_InvalidArgument = 1,
    NwStatus_AbiMismatch = 2,
    NwStatus_WrongContext = 3,
    NwStatus_StaleReference = 4,
    NwStatus_WrongKind = 5,
    NwStatus_NotFound = 6,
    NwStatus_Ambiguous = 7,
    NwStatus_SignatureMismatch = 8,
    NwStatus_ConversionFailed = 9,
    NwStatus_ManagedException = 10,
    NwStatus_WrongThread = 11,
    NwStatus_RenderThreadForbidden = 12,
    NwStatus_Canceled = 13,
    NwStatus_Unsupported = 14,
    NwStatus_ContractMismatch = 15,
    NwStatus_SessionEnded = 16,
    NwStatus_OutOfMemory = 17,
    NwStatus_InternalError = 18
};

typedef struct NwReference {
    uint32_t contextId;
    uint32_t index;
    uint32_t generation;
    uint32_t kind;
} NwReference;

enum NwReferenceKind {
    NwReferenceKind_None = 0,
    NwReferenceKind_Assembly = 1,
    NwReferenceKind_Type = 2,
    NwReferenceKind_Member = 3,
    NwReferenceKind_Object = 4,
    NwReferenceKind_Delegate = 5,
    NwReferenceKind_Exception = 6,
    NwReferenceKind_Operation = 7
};

typedef struct NwStringView {
    const uint8_t* data;
    uint64_t length;
} NwStringView;

typedef struct NwByteView {
    const uint8_t* data;
    uint64_t length;
} NwByteView;

typedef struct NwArrayView {
    const void* data;
    uint64_t count;
} NwArrayView;

typedef struct NwDecimal {
    int32_t lo;
    int32_t mid;
    int32_t hi;
    int32_t flags;
} NwDecimal;

enum NwValueKind {
    NwValueKind_Null = 0,
    NwValueKind_Bool = 1,
    NwValueKind_Int8 = 2,
    NwValueKind_Int16 = 3,
    NwValueKind_Int32 = 4,
    NwValueKind_Int64 = 5,
    NwValueKind_UInt8 = 6,
    NwValueKind_UInt16 = 7,
    NwValueKind_UInt32 = 8,
    NwValueKind_UInt64 = 9,
    NwValueKind_Float32 = 10,
    NwValueKind_Float64 = 11,
    NwValueKind_Decimal = 12,
    NwValueKind_String = 13,
    NwValueKind_Bytes = 14,
    NwValueKind_Enum = 15,
    NwValueKind_Reference = 16,
    NwValueKind_Array = 17
};

enum NwValueFlags {
    NwValueFlags_Owned = 1u,
    NwValueFlags_ElementKindMask = 0x0000ff00u
};

enum NwExecutionPolicy {
    NwExecutionPolicy_Unspecified = 0,
    NwExecutionPolicy_Managed = 1,
    NwExecutionPolicy_Calling = 2,
    NwExecutionPolicy_Application = 3,
    NwExecutionPolicy_Background = 4
};

typedef union NwValuePayload {
    uint8_t boolean;
    int64_t i64;
    uint64_t u64;
    float f32;
    double f64;
    NwDecimal decimal;
    NwStringView string;
    NwByteView bytes;
    NwArrayView array;
    NwReference reference;
} NwValuePayload;

typedef struct NwValue {
    uint32_t kind;
    uint32_t flags;
    NwValuePayload payload;
} NwValue;

#if defined(__cplusplus)
static_assert(sizeof(NwReference) == 16);
static_assert(alignof(NwReference) == 4);
static_assert(sizeof(NwStringView) == 16);
static_assert(alignof(NwStringView) == 8);
static_assert(sizeof(NwByteView) == 16);
static_assert(sizeof(NwArrayView) == 16);
static_assert(sizeof(NwDecimal) == 16);
static_assert(alignof(NwDecimal) == 4);
static_assert(sizeof(NwValue) == 24);
static_assert(alignof(NwValue) == 8);
#endif