#include "nano_basic.h"

static void (*print_ch_ptr)(NB_I8) = NULL;
static NB_BOOL (*fopen_ptr)(const NB_I8*, NB_BOOL) = NULL;
static void (*fclose_ptr)(void) = NULL;
static NB_BOOL (*fread_ptr)(NB_I8**, NB_SIZE*) = NULL;
static NB_BOOL (*fwrite_ptr)(NB_LINE_NUM, NB_I8*, NB_SIZE) = NULL;

void set_platform_print_ch(void (*func)(NB_I8)) {
    print_ch_ptr = func;
}

void set_platform_fopen(NB_BOOL (*func)(const NB_I8*, NB_BOOL)) {
    fopen_ptr = func;
}

void set_platform_fclose(void (*func)(void)) {
    fclose_ptr = func;
}

void set_platform_fread(NB_BOOL (*func)(NB_I8**, NB_SIZE*)) {
    fread_ptr = func;
}

void set_platform_fwrite(NB_BOOL (*func)(NB_LINE_NUM, NB_I8*, NB_SIZE)) {
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
