#if defined(__APPLE__)

#include <libkern/OSCacheControl.h>
#include <pthread.h>
#include <stddef.h>
#include <string.h>

/*
 * Copy into memory the kernel mapped with MAP_JIT, where mprotect returns
 * EACCES and mach_vm_write returns KERN_INVALID_ADDRESS.
 *
 * pthread_jit_write_protect_np is the only way in, and a managed runtime cannot
 * call it itself: while the region is writable it is not executable, so a
 * JIT-compiled caller faults on its next instruction. The toggle and the copy
 * have to share one native frame. Callers resolve this with dlsym.
 */
void doorstop_jit_memcpy(void *dst, const void *src, size_t n) {
#if defined(__aarch64__)
    int toggle = pthread_jit_write_protect_supported_np();

    if (toggle)
        pthread_jit_write_protect_np(0);

    memcpy(dst, src, n);

    if (toggle)
        pthread_jit_write_protect_np(1);
#else
    memcpy(dst, src, n);
#endif

    sys_icache_invalidate(dst, n);
}

#endif
