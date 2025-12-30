#include <stdio.h>
#include "nano_basic.h"
#include "stdlib.h"

#define CODE_SIZE (128)
#define VALUE_SIZE (24 * 3)
#define STACK_SIZE (16)
#define BUF_SIZE (1024)
static NB_I8 memory[CODE_SIZE + VALUE_SIZE + STACK_SIZE] = {'\0'};
static NB_I8 buf[BUF_SIZE] = {'\0'};

void platform_print_ch(NB_I8 ch)
{
    fputc(ch, stderr);
}

static FILE *fp = NULL;

NB_BOOL platform_fopen(const NB_I8 *name, NB_BOOL write_mode)
{
    if(fp != NULL) {
        platform_fclose();
    }

    if(write_mode) {
        fp = fopen(name, "w");
    } else {
        fp = fopen(name, "r");
    }
    return (fp != NULL) ? NB_TRUE : NB_FALSE;
}

void platform_fclose()
{
    if(fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
}

static NB_I8 _buf[256] = "";
NB_BOOL platform_fread(NB_I8 **buf, NB_SIZE *size)
{
    NB_LINE_NUM num = 0;

    if(fp == NULL) {
        return NB_FALSE;
    }

    if(fgets(_buf, sizeof(_buf) - 1, fp) == NULL) {
        return NB_FALSE;
    }

    *buf = _buf;
    _buf[sizeof(_buf) - 1] = '\0';

    for( *size = 0 ; _buf[*size] != '\0' ; *size = *size + 1) {
        if(_buf[*size] == '\r' || _buf[*size] == '\n') {
            _buf[*size] = '\0';
            break;
        } else if('0' <= _buf[*size] && _buf[*size] <= '9') {
            num = num * 10 + (_buf[*size] - '0');
        }
    } 
    return NB_TRUE;
}

NB_BOOL platform_fwrite(NB_LINE_NUM num, NB_I8 *buf, NB_SIZE size)
{
    if(fp == NULL) {
        return NB_FALSE;
    }
    fprintf(fp, "%d %s\r\n", num, buf);
    return NB_TRUE;
}

static NB_SIZE console_strlen(const NB_I8 *buf, NB_SIZE size)
{
    NB_SIZE i = 0;
    for(i = 0 ; i < size ; i++) {
        if(buf[i] == '\0') {
            return i;
        }
    }
    return i;
}

NB_VALUE platform_import(NB_VALUE num)
{
    fprintf(stderr, "import num:%d, value:%d\r\n", num, num * 2);
    return num * 2;
}

NB_RESULT command_import(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_VALUE inport_num = 0;
    NB_SIZE index = 0;
    NB_VALUE value = 0;

    IF_ERROR_EXIT(calc(code, size, pos, &inport_num));
    IF_FALSE_EXIT(code[*pos] == NB_CODE_STR_SEPALATE_PARAMETER);
    *pos = *pos + 1;
    calc_get_variable_pos(code, size, pos, &index);
    value = platform_import(inport_num);
    IF_ERROR_EXIT(memory_variable_set(index, value));

    return memory_code_next(num);
}

void platform_export(NB_VALUE num, NB_VALUE value)
{
    fprintf(stderr, "export num:%d, value:%d\r\n", num, value);
}

NB_RESULT command_export(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_VALUE export_num = 0;
    NB_VALUE value = 0;

    IF_ERROR_EXIT(calc(code, size, pos, &export_num));
    LOG("%d[%c]\r\n", code[*pos], code[*pos]);
    IF_FALSE_EXIT(code[*pos] == NB_CODE_STR_SEPALATE_PARAMETER);
    *pos = *pos + 1;
    IF_ERROR_EXIT(calc(code, size, pos, &value));
    platform_export(export_num, value);
    
    return memory_code_next(num);
}

int main(int argc, char *argv[])
{
    NB_STATE state = NB_STATE_REPL;
    NB_SIZE size = 0;
    NB_VALUE value = 0;

    nano_basic_init(memory, CODE_SIZE, VALUE_SIZE, STACK_SIZE);
    nano_basic_set_platform_print_ch(platform_print_ch);
    nano_basic_set_platform_fopen(platform_fopen);
    nano_basic_set_platform_fclose(platform_fclose);
    nano_basic_set_platform_fread(platform_fread);
    nano_basic_set_platform_fwrite(platform_fwrite);
    nano_basic_add_command("IMP", command_import);
    nano_basic_add_command("EXP", command_export);

    while(state != NB_STATE_END) {
        if(state == NB_STATE_REPL) {
            platform_print_ch('>');
            platform_print_ch(' ');
            fgets(buf, sizeof(buf), stdin);
            size = console_strlen(buf, sizeof(buf));
            if(size > 0 && buf[size - 1] == '\n') {
                buf[size - 1] = '\0';
            }
        }
        state = nano_basic_proc(state, buf, size);
        if(state == NB_STATE_INPUT_NUMBER) {
            platform_print_ch('?');
            platform_print_ch(' ');
            fgets(buf, sizeof(buf), stdin);
            value = atoi(buf);
            nano_basic_set_input_value(value);
        }
    }

    return 0;
}
