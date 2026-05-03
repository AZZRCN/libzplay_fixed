////////////////////////////////////////////////////////////////////////////////
///
/// x64 version of the CPU detect routine.
///
/// x64 CPUs always support SSE and SSE2, so this is a simplified implementation.
///
////////////////////////////////////////////////////////////////////////////////

#include "cpu_detect.h"
#include "STTypes.h"

static uint _dwDisabledISA = 0x00;

void disableExtensions(uint dwDisableMask)
{
    _dwDisabledISA = dwDisableMask;
}

uint detectCPUextensions(void)
{
    uint extensions = SUPPORT_SSE | SUPPORT_SSE2;
    return extensions & ~_dwDisabledISA;
}
