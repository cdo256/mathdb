#include "mdb_global.h"
#include "mdb_alloc_mem.h" //IMPLEMENTS
#include "mdb_get_mem_info.h" //IMPLEMENTS
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "mdb_error.h"

#undef verify
#if !DEBUG
#define verify(x) (void)(x)
#else
#define verify(x) do if (!(x)) ErrorExit( #x ); while (0)
#endif
static UP _allocations = 0;

#ifdef _WIN32

#pragma warning(push, 0)
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>
#include <strsafe.h>
#pragma warning(pop)

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "user32.lib")

#if NTDDI_VERSION < NTDDI_VISTA
#error "Windows must be Vista or later."
#endif


#pragma region https://jpassing.com/2008/03/12/walking-the-stack-of-the-current-thread/
typedef struct _STACKTRACE
{
    //
    // Number of frames in Frames array.
    //
    UINT FrameCount;

    //
    // PC-Addresses of frames. Index 0 contains the topmost frame.
    //
    ULONGLONG Frames[ ANYSIZE_ARRAY ];
} STACKTRACE, *PSTACKTRACE;

/*++
Routine Description:
Capture the stack trace based on the given CONTEXT.

Parameters:
Context    Thread context. If NULL, the current context is
used.
Event      Event structure to populate.
MaxFrames  Max number of frames the structure can hold.
--*/
#ifdef _M_IX86
//
// Disable global optimization and ignore /GS waning caused by 
// inline assembly.
//
#pragma optimize( "g", off )
#endif
HRESULT CaptureStackTrace(
    __in_opt CONST PCONTEXT InitialContext,
    __out PSTACKTRACE StackTrace,
    __in UINT MaxFrames 
)
{
    DWORD MachineType;
    CONTEXT Context;
    STACKFRAME64 StackFrame;

    assert( StackTrace );
    assert( MaxFrames > 0 );

    if ( InitialContext == NULL )
    {
        //
        // Use current context.
        //
        // N.B. GetThreadContext cannot be used on the current thread.
        // Capture own context - on i386, there is no API for that.
        //
#ifdef _M_IX86
        ZeroMemory( &Context, sizeof( CONTEXT ) );

        Context.ContextFlags = CONTEXT_CONTROL;

        //
        // Those three registers are enough.
        //
        __asm
        {
Label:
            mov [Context.Ebp], ebp;
            mov [Context.Esp], esp;
            mov eax, [Label];
            mov [Context.Eip], eax;
        }
#else
        RtlCaptureContext( &Context );
#endif  
    }
    else
    {
        CopyMemory( &Context, InitialContext, sizeof( CONTEXT ) ); 
    }

    //
    // Set up stack frame.
    //
    ZeroMemory( &StackFrame, sizeof( STACKFRAME64 ) );
#ifdef _M_IX86
    MachineType                 = IMAGE_FILE_MACHINE_I386;
    StackFrame.AddrPC.Offset    = Context.Eip;
    StackFrame.AddrPC.Mode      = AddrModeFlat;
    StackFrame.AddrFrame.Offset = Context.Ebp;
    StackFrame.AddrFrame.Mode   = AddrModeFlat;
    StackFrame.AddrStack.Offset = Context.Esp;
    StackFrame.AddrStack.Mode   = AddrModeFlat;
#elif _M_X64
    MachineType                 = IMAGE_FILE_MACHINE_AMD64;
    StackFrame.AddrPC.Offset    = Context.Rip;
    StackFrame.AddrPC.Mode      = AddrModeFlat;
    StackFrame.AddrFrame.Offset = Context.Rsp;
    StackFrame.AddrFrame.Mode   = AddrModeFlat;
    StackFrame.AddrStack.Offset = Context.Rsp;
    StackFrame.AddrStack.Mode   = AddrModeFlat;
#elif _M_IA64
    MachineType                 = IMAGE_FILE_MACHINE_IA64;
    StackFrame.AddrPC.Offset    = Context.StIIP;
    StackFrame.AddrPC.Mode      = AddrModeFlat;
    StackFrame.AddrFrame.Offset = Context.IntSp;
    StackFrame.AddrFrame.Mode   = AddrModeFlat;
    StackFrame.AddrBStore.Offset= Context.RsBSP;
    StackFrame.AddrBStore.Mode  = AddrModeFlat;
    StackFrame.AddrStack.Offset = Context.IntSp;
    StackFrame.AddrStack.Mode   = AddrModeFlat;
#else
#error "Unsupported platform"
#endif

    StackTrace->FrameCount = 0;

    //
    // Dbghelp is is singlethreaded, so acquire a lock.
    //
    // Note that the code assumes that 
    // SymInitialize( GetCurrentProcess(), NULL, TRUE ) has 
    // already been called.
    //
    //EnterCriticalSection( &DbgHelpLock );

    while ( StackTrace->FrameCount < MaxFrames )
    {
        if ( ! StackWalk64(
            IMAGE_FILE_MACHINE_I386,
            GetCurrentProcess(),
            GetCurrentThread(),
            &StackFrame,
            MachineType == IMAGE_FILE_MACHINE_I386 
            ? NULL
            : &Context,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL ) ) break;

        if ( StackFrame.AddrPC.Offset != 0 )
            StackTrace->Frames[ StackTrace->FrameCount++ ] = 
                StackFrame.AddrPC.Offset;
        else break;
    }

    //LeaveCriticalSection( &DbgHelpLock );

    return S_OK;
}
#ifdef _M_IX86
#pragma optimize( "g", on )
#endif
#pragma endregion

void ErrorExit(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code
    
    char* lpMsgBuf;
    char* lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen(lpMsgBuf) + lstrlen(lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintfA(lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    fprintf(stderr, "ERROR\n%s\n\n", lpDisplayBuf); 
    DebugBreak();
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

void PrintStackTrace(STACKTRACE* st) {
    for (UINT i = 0; i < st->FrameCount; i++) {
        ULONGLONG pc = st->Frames[i];
        PSYMBOL_INFO si = malloc(sizeof(SYMBOL_INFO)+255);
        *si = (SYMBOL_INFO){
            .SizeOfStruct = sizeof(SYMBOL_INFO),
            .MaxNameLen = 256
        };
        HANDLE process = GetCurrentProcess();
        verify(SymFromAddr(process, pc, 0, si));
        IMAGEHLP_LINE64 line = {
            .SizeOfStruct = sizeof(IMAGEHLP_LINE64)
        };
        DWORD displacement;
        verify(SymGetLineFromAddr64(process, pc, &displacement, &line) || GetLastError() == ERROR_INVALID_ADDRESS ); 
        fprintf(stderr, "%u: %s - %u\n    %s\n",i, si->Name, (u32)line.LineNumber, line.FileName);
        free(si);
    }
}

typedef struct {
    UP allocationSize;
    s32 occupied;
    void* allocationPtr;
    STACKTRACE* stackTrace;
} mem_call_info;

#define MAX_TRACES 100000
UP _traceCount = 0;
mem_call_info _callTraces[MAX_TRACES];

void MDB_stdcall MDB_InitDebug(void) {
    char path[MAX_PATH];
    HANDLE process = GetCurrentProcess();
    verify(GetModuleFileNameExA(process, NULL, path, MAX_PATH));
    verify(SymInitialize(process, path, TRUE));
}

void MDB_CaptureAllocTrace(UP size, void* ptr) {
    assert(ptr);
    STACKTRACE* st = malloc(offsetof(STACKTRACE, Frames)+80);
    CaptureStackTrace(NULL, st, 10);
    _callTraces[_traceCount++] = (mem_call_info){
        .allocationSize = size,
        .occupied = 1,
        .allocationPtr = ptr,
        .stackTrace = st
    };
    if (_traceCount + 1 >= MAX_TRACES) {
        //fail();
        _traceCount = 0; //shhh
    }
}
// size only changed if occupied is true
void MDB_CheckAndUpdateTrace(void* ptr, s32 occupied, UP size, UP* oldSize) {
    *oldSize = 0;
    for (UP i = 0; i < _traceCount; i++) {
        mem_call_info* mci = &_callTraces[i];
        if (mci->allocationPtr == ptr) {
            *oldSize = mci->allocationSize;
            if (mci->occupied) {
                mci->occupied = occupied;
                if (occupied) mci->allocationSize = size;
                CaptureStackTrace(NULL, mci->stackTrace, 10);
                return;
            } else {
                fprintf(stderr, "========\nERROR\nAttempting to free block that has already been free'd.\n");
                fprintf(stderr, "Block location: %p\nBlockSize: %"PRIuPTR"\n\nStackTrace:\n", ptr, mci->allocationSize);
                PrintStackTrace(mci->stackTrace);
                DebugBreak();
                return;
            }
        } else if (mci->allocationPtr < ptr && (u8*)ptr < (u8*)mci->allocationPtr + mci->allocationSize ) {
            fprintf(stderr, "=========\nERROR\nAttempting to free block part-way through.\n");
            fprintf(stderr, "\nBlock location: %p\nBlockSize: %"PRIuPTR"\nOffset: %"PRIuPTR"\nOccupied: %d\n\nStackTrace:\n", mci->allocationPtr, mci->allocationSize, (UP)((u8*)ptr-(u8*)mci->allocationPtr), mci->occupied);
            PrintStackTrace(mci->stackTrace);
            DebugBreak();
            return;
        }
    }
    fprintf(stderr, "========\nERROR\nAttempting to free an invalid ptr.\n");
    fprintf(stderr, "Ptr: %p\nBlockSize: %"PRIuPTR"\n", ptr, size);
    DebugBreak();
}

#endif

void* MDB_stdcall MDB_CAlloc(UP count, UP size) {
#if MDB_DEBUG_MEM
    void* block = calloc(count, size);
    if (block) {
        _allocations++;
        MDB_CaptureAllocTrace(count*size, block);
    }
    return block;
#else
    return calloc(count,size);
#endif
}
void* MDB_stdcall MDB_Alloc(UP size) {
#if MDB_DEBUG_MEM
    void* block = malloc(size);
    if (block) {
        _allocations++;
        MDB_CaptureAllocTrace(size, block);
    }
    return block;
#else
    return malloc(size);
#endif
}
void* MDB_stdcall MDB_Realloc(void* block, UP size) {
#if MDB_DEBUG_MEM
    assert(size != 0);
    UP oldSize=0;
    if (!block) _allocations++;
    else MDB_CheckAndUpdateTrace(block, 0,0, &oldSize);
    void* newblock = malloc(size);
    if (!newblock) _allocations--;
    else MDB_CaptureAllocTrace(size, newblock),memcpy(newblock,block,oldSize);
    return newblock;
#else
    return realloc(block, size);
#endif
}
void MDB_stdcall MDB_Free(void* block) {
#if MDB_DEBUG_MEM
    UP oldSize=0;
    if (block)
        _allocations--,

        MDB_CheckAndUpdateTrace(block,0,0,&oldSize);
#else
    free(block);
#endif
}

UP MDB_stdcall MDB_GetAllocatedBlockCount(void) {
    return _allocations;
}
