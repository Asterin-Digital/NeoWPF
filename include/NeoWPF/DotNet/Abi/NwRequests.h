/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Abi/NwCommon.h>

typedef struct NwRequestHeader {
    uint32_t structSize;
    uint32_t abiVersion;
} NwRequestHeader;

typedef struct NwError {
    uint32_t structSize;
    uint32_t code;
    NwStringView message;
    NwStringView managedTypeName;
    NwStringView stackTrace;
    NwReference exception;
} NwError;

typedef struct NwResult {
    uint32_t structSize;
    uint32_t abiVersion;
    NwStatus status;
    uint32_t reserved;
    NwValue value;
    NwError error;
} NwResult;

typedef struct NwLoadAssemblyRequest {
    NwRequestHeader header;
    NwStringView path;
    NwStringView loadContextName;
    uint32_t flags;
    uint32_t reserved;
} NwLoadAssemblyRequest;

typedef struct NwResolveTypeRequest {
    NwRequestHeader header;
    NwReference assembly;
    NwStringView fullName;
    const NwReference* typeArguments;
    uint32_t typeArgumentCount;
    uint32_t shape;
    uint32_t arrayRank;
    uint32_t reserved;
    NwReference elementType;
} NwResolveTypeRequest;

typedef struct NwParameterDescriptor {
    NwReference type;
    uint32_t passing;
    uint32_t reserved;
} NwParameterDescriptor;

typedef struct NwResolveMemberRequest {
    NwRequestHeader header;
    NwReference type;
    NwStringView name;
    uint32_t memberKind;
    uint32_t flags;
    uint32_t genericArity;
    uint32_t parameterCount;
    const NwParameterDescriptor* parameters;
    const NwReference* genericArguments;
} NwResolveMemberRequest;

typedef struct NwOperationSink {
    void* userData;
    void (NW_CALL* completed)(void* userData, NwReference operation, uint32_t state);
} NwOperationSink;

typedef struct NwCreateObjectRequest {
    NwRequestHeader header;
    NwReference constructor;
    const NwValue* arguments;
    uint32_t argumentCount;
    uint32_t policy;
} NwCreateObjectRequest;

typedef struct NwInvokeRequest {
    NwRequestHeader header;
    NwReference member;
    NwReference target;
    const NwValue* arguments;
    uint32_t argumentCount;
    uint32_t policy;
    uint32_t flags;
    uint32_t reserved;
    const NwOperationSink* sink;
    NwValue* outputs;
    uint32_t outputCount;
    uint32_t reserved2;
} NwInvokeRequest;

typedef struct NwPropertyRequest {
    NwRequestHeader header;
    NwReference member;
    NwReference target;
    NwValue value;
    uint32_t policy;
    uint32_t reserved;
} NwPropertyRequest;

typedef struct NwCallbackArgs {
    uint32_t structSize;
    uint32_t argumentCount;
    const NwValue* arguments;
    NwReference subscription;
    uint64_t sequence;
} NwCallbackArgs;

typedef struct NwCallbackTarget {
    void* userData;
    void (NW_CALL* invoke)(void* userData, const NwCallbackArgs* arguments);
    void (NW_CALL* release)(void* userData);
} NwCallbackTarget;

typedef struct NwRegisterCallbackRequest {
    NwRequestHeader header;
    NwReference target;
    NwReference member;
    NwCallbackTarget callback;
    uint32_t flags;
    uint32_t reserved;
} NwRegisterCallbackRequest;

typedef struct NwVerifyContractRequest {
    NwRequestHeader header;
    NwStringView bindingName;
    const uint8_t* fingerprint;
    uint32_t assemblyCount;
    uint32_t reserved;
    const uint8_t* assemblyMvids;
} NwVerifyContractRequest;