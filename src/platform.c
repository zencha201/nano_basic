#include "nano_basic.h"

static PrintCh print_ch_ptr = NULL;
static FOpen fopen_ptr = NULL;
static FClose fclose_ptr = NULL;
static FRead fread_ptr = NULL;
static FWrite fwrite_ptr = NULL;

void nano_basic_set_platform_print_ch(PrintCh func) {
    print_ch_ptr = func;
}

void nano_basic_set_platform_fopen(FOpen func) {
    fopen_ptr = func;
}

void nano_basic_set_platform_fclose(FClose func) {
    fclose_ptr = func;
}

void nano_basic_set_platform_fread(FRead func) {
    fread_ptr = func;
}

void nano_basic_set_platform_fwrite(FWrite func) {
    fwrite_ptr = func;
}

void platform_print_ch(NB_I8 ch)
{
    if (print_ch_ptr != NULL) {
        print_ch_ptr(ch);
    }
}

NB_BOOL platform_fopen(const NB_I8 *name, NB_BOOL write_mode)
{
    if (fopen_ptr != NULL) {
        return fopen_ptr(name, write_mode);
    }
    return NB_FALSE;
}

void platform_fclose()
{
    if (fclose_ptr != NULL) {
        fclose_ptr();
    }
}

NB_BOOL platform_fread(NB_I8 **buf, NB_SIZE *size)
{
    if (fread_ptr != NULL) {
        return fread_ptr(buf, size);
    }
    return NB_FALSE;
}

NB_BOOL platform_fwrite(NB_LINE_NUM num, NB_I8 *buf, NB_SIZE size)
{
    if (fwrite_ptr != NULL) {
        return fwrite_ptr(num, buf, size);
    }
    return NB_FALSE;
}
