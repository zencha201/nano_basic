#ifndef __NANO_BASIC_H__
#define __NANO_BASIC_H__

typedef char NB_I8;
typedef short NB_I16;
typedef int NB_I32;
typedef unsigned char NB_U8;
typedef unsigned short NB_U16;
typedef unsigned int NB_U32;
typedef NB_I16 NB_VALUE;
typedef short NB_LINE_NUM;
typedef short NB_SIZE;
typedef char NB_BOOL;

#ifndef NULL
#define NULL 0
#endif /* NULL */

#define NB_TRUE ((NB_BOOL)1)
#define NB_FALSE ((NB_BOOL)0)

#define NB_CODE_HEADER_SIZE (sizeof(NB_LINE_NUM) + sizeof(NB_SIZE)) /* ヘッダのサイズを取得 */
#define NB_CODE_SIZE(SIZE) (NB_CODE_HEADER_SIZE + SIZE) /* ヘッダ含むサイズを取得 */

#define NB_CODE_STR_TERMINATE ('\0') /* 終端 */
#define NB_CODE_STR_SEPALATE_PARAMETER (' ') /* 文字列上のパラメータ区切り */
#define NB_CODE_SEPALATE_PARAMETER (' ') /* メモリ内のコード上のパラメータ区切り */
#define NB_CODE_STRING_SIGN ('\"') /* 文字列記号 */

/* #define __DEBUG__ */
#ifdef __DEBUG__
#include <stdio.h>
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define TOSTRING(STR) #STR
#else /* __DEBUG__ */
#define LOG(...)
#endif /* __DEBUG__ */

#define IS_SUCCESS(EXP) ((EXP) == NB_RESULT_SUCCESS)
#define IS_ERROR(EXP) ((EXP) != NB_RESULT_SUCCESS)
#define IF_ERROR_EXIT(EXP) { \
    NB_RESULT __result = (EXP); \
    if IS_ERROR(__result) { \
        LOG("%s:%d: %s is %d\r\n", __FILE__, __LINE__, TOSTRING(EXP), __result); \
        return __result; \
    } \
}
#define IF_FALSE_EXIT(EXP) { \
    NB_RESULT __result = (EXP); \
    if IS_ERROR(__result == NB_FALSE) { \
        LOG("%s:%d: %s is false\r\n", __FILE__, __LINE__, TOSTRING(EXP)); \
        return NB_RESULT_ERROR_RUNTIME; \
    } \
}

typedef enum{
    NB_STATE_RUN_MODE, /* 入力待ちをせずにループ */
    NB_STATE_REPL, /* コード入力待ち */
    NB_STATE_INPUT_NUMBER, /* インプットコマンドによる数値入力待ち */
    NB_STATE_END,
} NB_STATE;

typedef enum{
    NB_RESULT_SUCCESS,
    NB_RESULT_FALSE,
    NB_RESULT_ERROR_MEMORY, 
    NB_RESULT_ERROR_SYNTAX, 
    NB_RESULT_ERROR_RUNTIME, 
} NB_RESULT;

typedef NB_RESULT (*CommandFunc)(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state);

/* インタプリタを動作させるための関数 */
void nano_basic_init(NB_I8 *memory, NB_SIZE code_size, NB_SIZE value_size, NB_SIZE stack_size);
NB_BOOL nano_basic_add_command(const NB_I8 *name, CommandFunc func);
NB_STATE nano_basic_proc(NB_STATE state, NB_I8 *buf, NB_SIZE size);
void nano_basic_set_input_value(NB_VALUE value);

/* コマンドを実装する場合に使用する関数 */
NB_RESULT calc_get_variable_pos(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_SIZE *index);
NB_RESULT calc(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value);
NB_RESULT memory_variable_get(NB_SIZE pos, NB_VALUE *value);
NB_RESULT memory_variable_set(NB_SIZE pos, NB_VALUE value);

/* プラットフォーム側で実装が必要な関数 */
typedef void (*PrintCh)(NB_I8);
typedef NB_BOOL (*FOpen)(const NB_I8*, NB_BOOL);
typedef void (*FClose)(void);
typedef NB_BOOL (*FRead)(NB_I8**, NB_SIZE*);
typedef NB_BOOL (*FWrite)(NB_LINE_NUM, NB_I8*, NB_SIZE);

void set_platform_print_ch(PrintCh func);
void set_platform_fopen(FOpen func);
void set_platform_fclose(FClose func);
void set_platform_fread(FRead func);
void set_platform_fwrite(FWrite func);

#endif /* __NANO_BASIC_H__ */
