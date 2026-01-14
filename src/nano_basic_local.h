#ifndef __NANO_BASIC_LOCAL_H__
#define __NANO_BASIC_LOCAL_H__

#include "nano_basic.h"

typedef struct {
    NB_LINE_NUM num;
    NB_SIZE size;
    NB_I8 buf[];
} NB_CODE;

typedef struct {
    const NB_I8 *name;
    CommandFunc func;
} CommandFuncList;

NB_RESULT nano_basic_set_code(NB_I8 *buf, NB_SIZE size, NB_LINE_NUM *num, NB_SIZE *pos);

void command_init();
void command_set_input_value(NB_VALUE value);
#ifdef __ENABLE_COMMAND_HELP__
NB_RESULT command_help(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
#endif /* __ENABLE_COMMAND_HELP__ */
NB_RESULT command_rem(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_let(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_print(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_input(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_if(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_then(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_goto(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_gosub(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_return(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_end(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_import(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_export(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_run(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_new(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_load(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_save(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_renum(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_list(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);
NB_RESULT command_del(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);

#if 0 /* nano_nasic.hに移動 */
NB_RESULT calc_get_variable_pos(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_SIZE *index);
NB_RESULT calc(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value);
#endif

void memory_init(NB_I8 *memory, NB_SIZE code_size, NB_SIZE value_size, NB_SIZE stack_size);

void memory_code_clear();
NB_RESULT memory_code_delete(NB_LINE_NUM num);
NB_RESULT memory_code_get(NB_LINE_NUM num, NB_I8 **buf, NB_SIZE *size);
NB_RESULT memory_code_set(NB_LINE_NUM num, NB_I8 *buf, NB_SIZE size);
#if 0 /* nano_nasic.hに移動 */
NB_RESULT memory_code_next(NB_LINE_NUM *next_num);
#endif
NB_RESULT memory_code_set_line_number(NB_LINE_NUM num, NB_LINE_NUM step);

void memory_value_clear();
#if 0 /* nano_nasic.hに移動 */
NB_RESULT memory_variable_get(NB_SIZE pos, NB_VALUE *value);
NB_RESULT memory_variable_set(NB_SIZE pos, NB_VALUE value);
#endif

void memory_stack_clear();
NB_RESULT memory_stack_push(NB_LINE_NUM value);
NB_RESULT memory_stack_pop(NB_LINE_NUM *value);

void platform_print_ch(NB_I8 ch);
NB_BOOL platform_fopen(const NB_I8 *name, NB_BOOL write_mode);
void platform_fclose();
NB_BOOL platform_fread(NB_I8 **buf, NB_SIZE *size);
NB_BOOL platform_fwrite(NB_LINE_NUM num, NB_I8 *buf, NB_SIZE size);

NB_BOOL util_strskipuntil(const NB_I8 *str, NB_SIZE *pos, NB_I8 ch, NB_SIZE size);
NB_BOOL util_strcmp(const NB_I8 *str1, const NB_I8 *str2, NB_SIZE size);
NB_SIZE util_strlen(const NB_I8 *str);
NB_RESULT util_atoi(const NB_I8 *str, NB_SIZE *pos, NB_SIZE size, NB_I16 *value);
NB_RESULT util_memset(NB_I8 *dest, NB_I8 value, NB_SIZE size);
NB_RESULT util_memcpy(NB_I8 *dest, NB_I8 *src, NB_SIZE size, NB_SIZE dest_size);
NB_RESULT util_memmove(NB_I8 *dest, NB_I8 *src, NB_SIZE size);
void util_print_value(NB_VALUE value);

#ifdef __DEBUG__
void show_buf(const NB_I8 *buf, NB_SIZE size);
#define SHOW_BUF(BUF, SIZE) show_buf(BUF, SIZE)
#else /* __DEBUG__ */
#define SHOW_BUF(BUF, SIZE)
#endif /* __DEBUG__ */

#endif /* __NANO_BASIC_LOCAL_H__ */
