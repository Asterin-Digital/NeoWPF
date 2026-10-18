/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Abi/NwRequests.h>

typedef struct NwBridgeApi {
    uint32_t structSize;
    uint32_t abiVersion;
    NwStatus (NW_CALL* LoadAssembly)(const NwLoadAssemblyRequest*, NwResult*);
    NwStatus (NW_CALL* ResolveType)(const NwResolveTypeRequest*, NwResult*);
    NwStatus (NW_CALL* ResolveMember)(const NwResolveMemberRequest*, NwResult*);
    NwStatus (NW_CALL* CreateObject)(const NwCreateObjectRequest*, NwResult*);
    NwStatus (NW_CALL* InvokeMember)(const NwInvokeRequest*, NwResult*);
    NwStatus (NW_CALL* GetProperty)(const NwPropertyRequest*, NwResult*);
    NwStatus (NW_CALL* SetProperty)(const NwPropertyRequest*, NwResult*);
    NwStatus (NW_CALL* RegisterCallback)(const NwRegisterCallbackRequest*, NwResult*);
    NwStatus (NW_CALL* UnregisterCallback)(NwReference);
    NwStatus (NW_CALL* RetainReference)(NwReference);
    NwStatus (NW_CALL* ReleaseReference)(NwReference);
    NwStatus (NW_CALL* GetOperationState)(NwReference, uint32_t*);
    NwStatus (NW_CALL* TakeOperationResult)(NwReference, NwResult*);
    NwStatus (NW_CALL* CancelOperation)(NwReference);
    NwStatus (NW_CALL* ReleaseOperation)(NwReference);
    NwStatus (NW_CALL* DescribeReference)(NwReference, NwResult*);
    NwStatus (NW_CALL* VerifyContract)(const NwVerifyContractRequest*, NwResult*);
    NwStatus (NW_CALL* ReleaseResult)(NwResult*);
    NwStatus (NW_CALL* ReleaseBuffer)(void*);
    NwStatus (NW_CALL* Shutdown)(uint32_t);
} NwBridgeApi;

typedef struct NwHostCallbacks {
    uint32_t structSize;
    uint32_t abiVersion;
    void* userData;
    void (NW_CALL* log)(void* userData, uint32_t level, NwStringView category, NwStringView message);
} NwHostCallbacks;

typedef struct NwBridgeInitInfo {
    uint32_t structSize;
    uint32_t abiVersion;
    uint32_t contextId;
    uint32_t flags;
    const NwHostCallbacks* host;
} NwBridgeInitInfo;

typedef NwStatus (NW_CALL* NwBridgeInitializeFn)(const NwBridgeInitInfo*, const NwBridgeApi**);

#if defined(__cplusplus)
static_assert(sizeof(NwBridgeApi) == 168);
static_assert(alignof(NwBridgeApi) == 8);
#endif