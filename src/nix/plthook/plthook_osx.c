/* -*- indent-tabs-mode: nil -*-
 *
 * plthook_osx.c -- implementation of plthook for OS X
 *
 * URL: https://github.com/kubo/plthook
 *
 * ------------------------------------------------------
 *
 * Copyright 2014-2019 Kubo Takehiro <kubo@jiubao.org>
 *                2020 Geoffrey Horsington <neigh@coder.horse>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the authors.
 *
 */
#if defined(__APPLE__)
#include "plthook.h"
#include <dlfcn.h>
#include <inttypes.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <mach-o/nlist.h>
#include <mach-o/swap.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PLTHOOK_DEBUG_CMD 1
#define PLTHOOK_DEBUG_BIND 1

#ifdef PLTHOOK_DEBUG_CMD
#define DEBUG_CMD(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_CMD(...)
#endif

#ifdef PLTHOOK_DEBUG_BIND
#define DEBUG_BIND(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_BIND(...)
#endif

#ifdef __LP64__
#define segment_command_ segment_command_64
#define section_ section_64
#define nlist_ nlist_64
#else
#define segment_command_ segment_command
#define section_ section
#define nlist_ nlist
#endif

typedef struct {
    const char *name;
    void **addr;
} bind_address_t;

struct plthook {
    unsigned int num_entries;
    char *strings;
    bind_address_t entries[1];
};

#define MAX_SEGMENTS 8

typedef struct {
    plthook_t *plthook;
    intptr_t slide;
    int num_segments;
    int linkedit_segment_idx;
    int data_segment_idx;
    void **lazy_symbols;
    size_t lazy_symbols_size;
    uint32_t lazy_symbols_isym_offset;
    struct segment_command_ *segments[MAX_SEGMENTS];
} data_t;

static int plthook_open_real(plthook_t **plthook_out, uint32_t image_idx,
                             const struct mach_header *mh,
                             const char *image_name);

static void set_errmsg(const char *fmt, ...)
    __attribute__((__format__(__printf__, 1, 2)));

static char errmsg[512];

void *plthook_handle_by_name(const char *name) {
    void *mono_handle = NULL;
    uint32_t cnt = _dyld_image_count();
    for (uint32_t idx = 0; idx < cnt; idx++) {
        const char *image_name = idx ? _dyld_get_image_name(idx) : NULL;
        if (image_name && strstr(image_name, name)) {
            mono_handle = dlopen(image_name, RTLD_LAZY | RTLD_NOLOAD);
            return mono_handle;
        }
    }
    return NULL;
}

int plthook_open(plthook_t **plthook_out, const char *filename) {
    size_t namelen;
    uint32_t cnt;
    uint32_t idx;

    if (filename == NULL) {
        return plthook_open_real(plthook_out, 0, NULL, NULL);
    }
    cnt = _dyld_image_count();
    namelen = strlen(filename);
    namelen = strlen(filename);
    cnt = _dyld_image_count();

    for (idx = 0; idx < cnt; idx++) {
        const char *image_name = _dyld_get_image_name(idx);
        size_t offset = 0;

        if (image_name == NULL) {
            *plthook_out = NULL;
            set_errmsg("Cannot find file at image index %u", idx);
            return PLTHOOK_INTERNAL_ERROR;
        }
        if (*filename != '/') {
            size_t image_name_len = strlen(image_name);
            if (image_name_len > namelen) {
                offset = image_name_len - namelen;
            }
        }
        if (strcmp(image_name + offset, filename) == 0) {
            return plthook_open_real(plthook_out, idx, NULL, image_name);
        }
    }
    *plthook_out = NULL;
    set_errmsg("Cannot find file: %s", filename);
    return PLTHOOK_FILE_NOT_FOUND;
}

int plthook_open_by_handle(plthook_t **plthook_out, void *hndl) {
    int flags[] = {
        RTLD_LAZY | RTLD_NOLOAD,
        RTLD_LAZY | RTLD_NOLOAD | RTLD_FIRST,
    };
    int flag_idx;
    uint32_t cnt = _dyld_image_count();
#define NUM_FLAGS (sizeof(flags) / sizeof(flags[0]))

    if (hndl == NULL) {
        set_errmsg("NULL handle");
        return PLTHOOK_FILE_NOT_FOUND;
    }
    for (flag_idx = 0; flag_idx < NUM_FLAGS; flag_idx++) {
        uint32_t idx;

        for (idx = 0; idx < cnt; idx++) {
            const char *image_name = idx ? _dyld_get_image_name(idx) : NULL;
            void *handle = dlopen(image_name, flags[flag_idx]);
            if (handle != NULL) {
                dlclose(handle);
                if (handle == hndl) {
                    return plthook_open_real(plthook_out, idx, NULL,
                                             image_name);
                }
            }
        }
    }
    set_errmsg("Cannot find the image correspond to handle %p", hndl);
    return PLTHOOK_FILE_NOT_FOUND;
}

int plthook_open_by_address(plthook_t **plthook_out, void *address) {
    Dl_info dlinfo;
    uint32_t idx = 0;
    uint32_t cnt = _dyld_image_count();

    if (!dladdr(address, &dlinfo)) {
        *plthook_out = NULL;
        set_errmsg("Cannot find address: %p", address);
        return PLTHOOK_FILE_NOT_FOUND;
    }
    for (idx = 0; idx < cnt; idx++) {
        if (dlinfo.dli_fbase == _dyld_get_image_header(idx)) {
            return plthook_open_real(plthook_out, idx, dlinfo.dli_fbase,
                                     dlinfo.dli_fname);
        }
    }
    set_errmsg("Cannot find the image index for base address: %p",
               dlinfo.dli_fbase);
    return PLTHOOK_FILE_NOT_FOUND;
}

static int load_la_symbols(data_t *data) {
    struct segment_command_ *data_segment =
        data->segments[data->data_segment_idx];
    struct section_ *data_sections =
        (struct section_ *)((size_t)data_segment +
                            sizeof(struct segment_command_));
    struct section_ *section;
    uint32_t seg_i = 0;

    for (seg_i = 0; seg_i < data_segment->nsects; seg_i++) {
        section = &data_sections[seg_i];
        DEBUG_CMD("__DATA section %s\n", section->sectname);
        if ((section->flags & S_LAZY_SYMBOL_POINTERS) ==
            S_LAZY_SYMBOL_POINTERS) {
            DEBUG_CMD("Found lazy symbol pointers\n");
            data->lazy_symbols = (void **)(section->addr + data->slide);
            data->lazy_symbols_size = section->size;
            data->lazy_symbols_isym_offset = section->reserved1;
            return 0;
        }
    }

    return PLTHOOK_INTERNAL_ERROR;
}

static int get_base_offset(FILE *obj_file) {
    uint32_t magic;
    struct fat_header header;
    struct fat_arch arch;
    int is_fat, needs_swap;
    size_t header_size = sizeof(struct fat_header);
    size_t arch_size = sizeof(struct fat_arch);
    off_t arch_offset = (off_t)header_size;
    uint32_t arch_i = 0;

    fseek(obj_file, 0, SEEK_SET);
    fread(&magic, sizeof(uint32_t), 1, obj_file);

    is_fat = magic == FAT_CIGAM || magic == FAT_MAGIC;
    needs_swap = magic == FAT_CIGAM;
    if (!is_fat)
        return 0; // No FAT => use offsets as-is

    DEBUG_CMD("Got FAT binary\n");

    fseek(obj_file, 0, SEEK_SET);
    fread(&header, sizeof(struct fat_header), 1, obj_file);
    if (needs_swap)
        swap_fat_header(&header, 0);

    for (arch_i = 0; arch_i < header.nfat_arch; arch_i++) {
        fseek(obj_file, arch_offset + arch_i * arch_size, SEEK_SET);
        fread(&arch, arch_size, 1, obj_file);
        if (needs_swap)
            swap_fat_arch(&arch, 1, 0);
        switch (arch.cputype) {
        case CPU_TYPE_I386:
            DEBUG_CMD("Has arch: I386, offset: %8x\n", arch.offset);
#ifndef __LP64__
            return arch.offset;
#endif
            break;
        case CPU_TYPE_X86_64:
            DEBUG_CMD("Has arch: X86_64, offset: %8x\n", arch.offset);
#if __LP64__
            return arch.offset;
#endif
            break;
        default:
            break;
        }
    }
    return -1;
}

static int plthook_open_real(plthook_t **plthook_out, uint32_t image_idx,
                             const struct mach_header *mh,
                             const char *image_name) {
    struct load_command *cmd;
    struct dysymtab_command *dysymtab_command;
    struct symtab_command *symtab_command;
    uint32_t lazy_bind_off = 0;
    uint32_t lazy_bind_size = 0;
    unsigned int nbind;
    data_t data = {
        NULL,
    };
    size_t size;
    int i;
    uint32_t symbol_index;
    size_t num_funcs;
    uint32_t *indirect_symbols;
    struct nlist_ *symbols;
    char *symbol_strings;
    FILE *dfile;
    int base_offset;

    data.linkedit_segment_idx = -1;
    data.data_segment_idx = -1;
    data.slide = _dyld_get_image_vmaddr_slide(image_idx);
    DEBUG_CMD("slide=%" PRIxPTR "\n", data.slide);
    if (mh == NULL) {
        mh = _dyld_get_image_header(image_idx);
    }
    if (image_name == NULL) {
        image_name = _dyld_get_image_name(image_idx);
    }

    DEBUG_CMD("Image name: %s\n", image_name);

#ifdef __LP64__
    cmd = (struct load_command *)((size_t)mh + sizeof(struct mach_header_64));
#else
    cmd = (struct load_command *)((size_t)mh + sizeof(struct mach_header));
#endif
    for (i = 0; i < mh->ncmds; i++) {
        struct dyld_info_command *dyld_info;
        struct segment_command *segment;
        struct segment_command_64 *segment64;

        switch (cmd->cmd) {
        case LC_SEGMENT: /* 0x1 */
            segment = (struct segment_command *)cmd;
            DEBUG_CMD("LC_SEGMENT\n"
                      "  segname   %s\n"
                      "  vmaddr    %8x  vmsize     %8x\n"
                      "  fileoff   %8x  filesize   %8x\n"
                      "  maxprot   %8x  initprot   %8x\n"
                      "  nsects    %8d  flags      %8x\n",
                      segment->segname, segment->vmaddr, segment->vmsize,
                      segment->fileoff, segment->filesize, segment->maxprot,
                      segment->initprot, segment->nsects, segment->flags);
#ifndef __LP64__
            if (strcmp(segment->segname, "__LINKEDIT") == 0) {
                data.linkedit_segment_idx = data.num_segments;
            }
            if (strcmp(segment->segname, "__DATA") == 0) {
                data.data_segment_idx = data.num_segments;
            }
            if (data.num_segments == MAX_SEGMENTS) {
                set_errmsg("Too many segments:  %s", image_name);
                return PLTHOOK_INTERNAL_ERROR;
            }
            data.segments[data.num_segments++] = segment;
#endif
            break;
        case LC_SEGMENT_64: /* 0x19 */
            segment64 = (struct segment_command_64 *)cmd;
            DEBUG_CMD("LC_SEGMENT_64\n"
                      "  segname   %s\n"
                      "  vmaddr    %8llx  vmsize     %8llx\n"
                      "  fileoff   %8llx  filesize   %8llx\n"
                      "  maxprot   %8x  initprot   %8x\n"
                      "  nsects    %8d  flags      %8x\n",
                      segment64->segname, segment64->vmaddr, segment64->vmsize,
                      segment64->fileoff, segment64->filesize,
                      segment64->maxprot, segment64->initprot,
                      segment64->nsects, segment64->flags);
#ifdef __LP64__
            if (strcmp(segment64->segname, "__LINKEDIT") == 0) {
                data.linkedit_segment_idx = data.num_segments;
            }
            if (strcmp(segment64->segname, "__DATA") == 0) {
                data.data_segment_idx = data.num_segments;
            }
            if (data.num_segments == MAX_SEGMENTS) {
                set_errmsg("Too many segments: %s", image_name);
                return PLTHOOK_INTERNAL_ERROR;
            }
            data.segments[data.num_segments++] = segment64;
#endif
            break;
        case LC_DYLD_INFO_ONLY: /* (0x22|LC_REQ_DYLD) */
            dyld_info = (struct dyld_info_command *)cmd;
            lazy_bind_off = dyld_info->lazy_bind_off;
            lazy_bind_size = dyld_info->lazy_bind_size;
            DEBUG_CMD("LC_DYLD_INFO_ONLY\n"
                      "                 offset     size\n"
                      "  rebase       %8x %8x\n"
                      "  bind         %8x %8x\n"
                      "  weak_bind    %8x %8x\n"
                      "  lazy_bind    %8x %8x\n"
                      "  export_bind  %8x %8x\n",
                      dyld_info->rebase_off, dyld_info->rebase_size,
                      dyld_info->bind_off, dyld_info->bind_size,
                      dyld_info->weak_bind_off, dyld_info->weak_bind_size,
                      dyld_info->lazy_bind_off, dyld_info->lazy_bind_size,
                      dyld_info->export_off, dyld_info->export_size);
            break;
        case LC_SYMTAB: /* 0x2 */
            symtab_command = (struct symtab_command *)cmd;
            DEBUG_CMD("LC_SYMTAB\n"
                      "symbol count: %8x\n"
                      "symbol offset: %8x\n"
                      "stf offset: %8x\n"
                      "str size: %8x\n",
                      symtab_command->nsyms, symtab_command->symoff,
                      symtab_command->stroff, symtab_command->strsize);
            break;
        case LC_DYSYMTAB: /* 0xb */
            dysymtab_command = (struct dysymtab_command *)cmd;
            DEBUG_CMD("LC_DYSYMTAB\n"
                      "itable offset: %8x\n"
                      "itable count: %8x\n",
                      dysymtab_command->indirectsymoff,
                      dysymtab_command->nindirectsyms);
            break;
        case LC_LOAD_DYLIB: /* 0xc */
            DEBUG_CMD("LC_LOAD_DYLIB\n");
            break;
        case LC_ID_DYLIB: /* 0xd */
            DEBUG_CMD("LC_ID_DYLIB\n");
            break;
        case LC_LOAD_DYLINKER: /* 0xe */
            DEBUG_CMD("LC_LOAD_DYLINKER\n");
            break;
        case LC_ROUTINES_64: /* 0x1a */
            DEBUG_CMD("LC_ROUTINES_64\n");
            break;
        case LC_UUID: /* 0x1b */
            DEBUG_CMD("LC_UUID\n");
            break;
        case LC_VERSION_MIN_MACOSX: /* 0x24 */
            DEBUG_CMD("LC_VERSION_MIN_MACOSX\n");
            break;
        case LC_FUNCTION_STARTS: /* 0x26 */
            DEBUG_CMD("LC_FUNCTION_STARTS\n");
            break;
        case LC_MAIN: /* 0x28|LC_REQ_DYLD */
            DEBUG_CMD("LC_MAIN\n");
            break;
        case LC_DATA_IN_CODE: /* 0x29 */
            DEBUG_CMD("LC_DATA_IN_CODE\n");
            break;
        case LC_SOURCE_VERSION: /* 0x2A */
            DEBUG_CMD("LC_SOURCE_VERSION\n");
            break;
        case LC_DYLIB_CODE_SIGN_DRS: /* 0x2B */
            DEBUG_CMD("LC_DYLIB_CODE_SIGN_DRS\n");
            break;
        default:
            DEBUG_CMD("LC_? (0x%x)\n", cmd->cmd);
        }
        cmd = (struct load_command *)((size_t)cmd + cmd->cmdsize);
    }

    if (data.linkedit_segment_idx == -1) {
        set_errmsg("Cannot find the linkedit segment: %s", image_name);
        return PLTHOOK_INVALID_FILE_FORMAT;
    }
    if (data.data_segment_idx == -1) {
        set_errmsg("Cannot find the data segment: %s", image_name);
        return PLTHOOK_INVALID_FILE_FORMAT;
    }
    if (load_la_symbols(&data) != 0) {
        set_errmsg("Cannot locate lazily loaded symbols: %s", image_name);
        return PLTHOOK_INVALID_FILE_FORMAT;
    }

    num_funcs = data.lazy_symbols_size / sizeof(void *);

    size = offsetof(plthook_t, entries) + sizeof(bind_address_t) * num_funcs;
    data.plthook = (plthook_t *)malloc(size);
    if (data.plthook == NULL) {
        set_errmsg("failed to allocate memory: %" PRIuPTR " bytes", size);
        return PLTHOOK_OUT_OF_MEMORY;
    }

    dfile = fopen(image_name, "r");
    if (dfile == NULL) {
        set_errmsg("Failed to open image (no permissions or in use?): %s",
                   image_name);
        free(data.plthook);
        return PLTHOOK_INVALID_FILE_FORMAT;
    }

    base_offset = get_base_offset(dfile);

    if (base_offset < 0) {
        set_errmsg("The binary is a FAT binary but has no binary for the "
                   "current arch.");
        free(data.plthook);
        fclose(dfile);
        return PLTHOOK_INVALID_FILE_FORMAT;
    }

    DEBUG_CMD("Got base offset: %x\n", base_offset);

    indirect_symbols =
        malloc(sizeof(uint32_t) * dysymtab_command->nindirectsyms);
    symbols = malloc(sizeof(struct nlist_) * symtab_command->nsyms);
    symbol_strings = malloc(sizeof(char) * symtab_command->strsize);

    fseek(dfile, base_offset + dysymtab_command->indirectsymoff, SEEK_SET);
    fread(indirect_symbols, sizeof(uint32_t), dysymtab_command->nindirectsyms,
          dfile);

    fseek(dfile, base_offset + symtab_command->symoff, SEEK_SET);
    fread(symbols, sizeof(struct nlist_), symtab_command->nsyms, dfile);

    fseek(dfile, base_offset + symtab_command->stroff, SEEK_SET);
    fread(symbol_strings, sizeof(char), symtab_command->strsize, dfile);

    fclose(dfile);

    data.plthook->num_entries = num_funcs;
    data.plthook->strings = symbol_strings;

    for (i = 0; i < num_funcs; i++) {
        symbol_index = indirect_symbols[i + data.lazy_symbols_isym_offset];
        DEBUG_CMD("Symbol (%p) %d -> %s\n", data.lazy_symbols[i], symbol_index,
                  symbol_strings + symbols[symbol_index].n_un.n_strx);

        data.plthook->entries[i].name =
            symbol_strings + symbols[symbol_index].n_un.n_strx;
        data.plthook->entries[i].addr = &data.lazy_symbols[i];
    }

    free(indirect_symbols);
    free(symbols);
    *plthook_out = data.plthook;
    return 0;
}

int plthook_enum(plthook_t *plthook, unsigned int *pos, const char **name_out,
                 void ***addr_out) {
    if (*pos >= plthook->num_entries) {
        *name_out = NULL;
        *addr_out = NULL;
        return EOF;
    }
    *name_out = plthook->entries[*pos].name;
    *addr_out = plthook->entries[*pos].addr;
    (*pos)++;
    return 0;
}

int plthook_replace(plthook_t *plthook, const char *funcname, void *funcaddr,
                    void **oldfunc) {
    size_t funcnamelen = strlen(funcname);
    unsigned int pos = 0;
    const char *name;
    void **addr;
    int rv;

    if (plthook == NULL) {
        set_errmsg("invalid argument: The first argument is null.");
        return PLTHOOK_INVALID_ARGUMENT;
    }
    while ((rv = plthook_enum(plthook, &pos, &name, &addr)) == 0) {
        if (strncmp(name, funcname, funcnamelen) == 0) {
            if (name[funcnamelen] == '\0' || name[funcnamelen] == '$') {
                goto matched;
            }
        }
        if (name[0] == '@') {
            /* Oracle libclntsh.dylib imports 'read' as '@_read'. */
            name++;
            if (strncmp(name, funcname, funcnamelen) == 0) {
                if (name[funcnamelen] == '\0' || name[funcnamelen] == '$') {
                    goto matched;
                }
            }
        }
        if (name[0] == '_') {
            name++;
            if (strncmp(name, funcname, funcnamelen) == 0) {
                if (name[funcnamelen] == '\0' || name[funcnamelen] == '$') {
                    goto matched;
                }
            }
        }
        continue;
    matched:
        if (oldfunc) {
            *oldfunc = *addr;
        }
        *addr = funcaddr;
        return 0;
    }
    if (rv == EOF) {
        set_errmsg("no such function: %s", funcname);
        rv = PLTHOOK_FUNCTION_NOT_FOUND;
    }
    return rv;
}

void plthook_close(plthook_t *plthook) {
    if (plthook != NULL) {
        free(plthook->strings);
        free(plthook);
    }
    return;
}

const char *plthook_error(void) { return errmsg; }

static void set_errmsg(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(errmsg, sizeof(errmsg) - 1, fmt, ap);
    va_end(ap);
}
#endif