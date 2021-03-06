/*
 * $Id$
 *
 * DEBUG: section 54    Interprocess Communication
 *
 */

#include "config.h"
#include "base/TextException.h"
#include "CpuAffinitySet.h"
#include "Debug.h"
#include "util.h"

#if HAVE_ERRNO_H
#include <errno.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif


CpuAffinitySet::CpuAffinitySet()
{
    CPU_ZERO(&theCpuSet);
    CPU_ZERO(&theOrigCpuSet);
}

void
CpuAffinitySet::apply()
{
    Must(CPU_COUNT(&theCpuSet) > 0); // CPU affinity mask set
    Must(!applied());

    bool success = false;
    if (sched_getaffinity(0, sizeof(theOrigCpuSet), &theOrigCpuSet)) {
        debugs(54, DBG_IMPORTANT, "ERROR: failed to get CPU affinity for "
               "process PID " << getpid() << ", ignoring CPU affinity for "
               "this process: " << xstrerror());
    } else {
        cpu_set_t cpuSet;
        memcpy(&cpuSet, &theCpuSet, sizeof(cpuSet));
        CPU_AND(&cpuSet, &cpuSet, &theOrigCpuSet);
        if (CPU_COUNT(&cpuSet) <= 0) {
            debugs(54, DBG_IMPORTANT, "ERROR: invalid CPU affinity for process "
                   "PID " << getpid() << ", may be caused by an invalid core in "
                   "'cpu_affinity_map' or by external affinity restrictions");
        } else if (sched_setaffinity(0, sizeof(cpuSet), &cpuSet)) {
            debugs(54, DBG_IMPORTANT, "ERROR: failed to set CPU affinity for "
                   "process PID " << getpid() << ": " << xstrerror());
        } else
            success = true;
    }
    if (!success)
        CPU_ZERO(&theOrigCpuSet);
}

void
CpuAffinitySet::undo()
{
    if (applied()) {
        if (sched_setaffinity(0, sizeof(theOrigCpuSet), &theOrigCpuSet)) {
            debugs(54, DBG_IMPORTANT, "ERROR: failed to restore original CPU "
                   "affinity for process PID " << getpid() << ": " <<
                   xstrerror());
        }
        CPU_ZERO(&theOrigCpuSet);
    }
}

bool
CpuAffinitySet::applied() const
{
    return (CPU_COUNT(&theOrigCpuSet) > 0);
}

void
CpuAffinitySet::set(const cpu_set_t &aCpuSet)
{
    memcpy(&theCpuSet, &aCpuSet, sizeof(theCpuSet));
}
