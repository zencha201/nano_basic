#include "nano_basic_local.h"
#include "test.h"
#include <string.h>

#define CODE_SIZE (256)
#define VALUE_SIZE (72)
#define STACK_SIZE (32)
static NB_I8 memory[CODE_SIZE + VALUE_SIZE + STACK_SIZE];

/* プラットフォーム関数のモック */
static NB_I8 print_buffer[256];
static NB_SIZE print_pos = 0;

void platform_print_ch(NB_I8 ch)
{
    if(print_pos < sizeof(print_buffer) - 1) {
        print_buffer[print_pos++] = ch;
        print_buffer[print_pos] = '\0';
    }
}

static void clear_print_buffer()
{
    print_pos = 0;
    print_buffer[0] = '\0';
}

NB_VALUE platform_import(NB_VALUE num)
{
    return num * 2;
}

NB_BOOL platform_fopen(const NB_I8 *name, NB_BOOL write_mode)
{
    return NB_FALSE;  /* テストでは使用しない */
}

void platform_fclose()
{
}

NB_BOOL platform_fread(NB_I8 **buf, NB_SIZE *size)
{
    return NB_FALSE;
}

NB_BOOL platform_fwrite(NB_LINE_NUM num, NB_I8 *buf, NB_SIZE size)
{
    return NB_FALSE;
}

static void test_command_rem()
{
    TEST_BEGIN("command REM test");

    {
        NB_I8 code[] = "REM THIS IS COMMENT";
        NB_LINE_NUM num = 10;
        NB_SIZE pos = 4;  /* "REM " の後 */
        NB_STATE state = NB_STATE_RUN_MODE;
        NB_LINE_NUM prev_num = num;
        
        memory_code_set(10, code, sizeof(code) - 1);
        memory_code_set(20, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(command_rem(&num, code, sizeof(code), &pos, &state)));
        TEST(num == 20);  /* 次の行に進む */
        TEST(state == NB_STATE_RUN_MODE);
    }

    TEST_END();
}

static void test_command_let()
{
    TEST_BEGIN("command LET test");

    {
        NB_I8 code[] = "LET A 123";
        NB_LINE_NUM num = 10;
        NB_SIZE pos = 4;  /* "LET " の後 */
        NB_STATE state = NB_STATE_RUN_MODE;
        NB_VALUE value = 0;
        
        memory_code_set(10, code, sizeof(code) - 1);
        memory_code_set(20, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(command_let(&num, code, sizeof(code), &pos, &state)));
        TEST(IS_SUCCESS(memory_variable_get(0, &value)));  /* A = index 0 */
        TEST(value == 123);
        TEST(num == 20);  /* 次の行に進む */
    }
    {
        NB_I8 code[] = "LET B A+10";
        NB_LINE_NUM num = 20;
        NB_SIZE pos = 4;
        NB_STATE state = NB_STATE_RUN_MODE;
        NB_VALUE value = 0;
        
        memory_code_set(20, code, sizeof(code) - 1);
        memory_code_set(30, code, sizeof(code) - 1);
        memory_variable_set(0, 100);  /* A = 100 */
        
        TEST(IS_SUCCESS(command_let(&num, code, sizeof(code), &pos, &state)));
        TEST(IS_SUCCESS(memory_variable_get(1, &value)));  /* B = index 1 */
        TEST(value == 110);
        TEST(num == 30);
    }

    TEST_END();
}

static void test_command_print()
{
    TEST_BEGIN("command PRINT test");

    {
        NB_I8 code[] = "PRINT \"HELLO\"";
        NB_LINE_NUM num = 10;
        NB_SIZE pos = 6;  /* "PRINT " の後 */
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_set(10, code, sizeof(code) - 1);
        memory_code_set(20, code, sizeof(code) - 1);
        
        clear_print_buffer();
        TEST(IS_SUCCESS(command_print(&num, code, sizeof(code), &pos, &state)));
        LOG("print_buffer: [%s]\r\n", print_buffer);
        TEST(print_buffer[0] == 'H');
        TEST(print_buffer[1] == 'E');
        TEST(print_buffer[2] == 'L');
        TEST(print_buffer[3] == 'L');
        TEST(print_buffer[4] == 'O');
        TEST(num == 20);
    }
    {
        NB_I8 code[] = "PRINT 123";
        NB_LINE_NUM num = 20;
        NB_SIZE pos = 6;
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_set(20, code, sizeof(code) - 1);
        memory_code_set(30, code, sizeof(code) - 1);
        
        clear_print_buffer();
        TEST(IS_SUCCESS(command_print(&num, code, sizeof(code), &pos, &state)));
        LOG("print_buffer: [%s]\r\n", print_buffer);
        /* 数値が出力されているはず */
        TEST(num == 30);
    }

    TEST_END();
}

static void test_command_if()
{
    TEST_BEGIN("command IF test");

    {
        /* IF 1 THEN PRINT "TRUE" */
        NB_I8 code[] = "IF 1 PRINT \"TRUE\"";
        NB_LINE_NUM num = 10;
        NB_SIZE pos = 3;  /* "IF " の後 */
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_set(10, code, sizeof(code) - 1);
        memory_code_set(20, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(command_if(&num, code, sizeof(code), &pos, &state)));
        TEST(num == 10);  /* 条件が真なので同じ行で継続 */
        TEST(pos > 3);    /* 位置が進んでいる */
    }
    {
        /* IF 0 THEN PRINT "FALSE" */
        NB_I8 code[] = "IF 0 PRINT \"FALSE\"";
        NB_LINE_NUM num = 20;
        NB_SIZE pos = 3;
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_set(20, code, sizeof(code) - 1);
        memory_code_set(30, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(command_if(&num, code, sizeof(code), &pos, &state)));
        TEST(num == 30);  /* 条件が偽なので次の行に進む */
    }
    {
        /* IF 5>3 THEN ... */
        NB_I8 code[] = "IF 5>3 PRINT \"OK\"";
        NB_LINE_NUM num = 30;
        NB_SIZE pos = 3;
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_set(30, code, sizeof(code) - 1);
        memory_code_set(40, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(command_if(&num, code, sizeof(code), &pos, &state)));
        TEST(num == 30);  /* 条件が真なので同じ行で継続 */
    }

    TEST_END();
}

static void test_command_goto()
{
    TEST_BEGIN("command GOTO test");

    {
        NB_I8 code[] = "GOTO 100";
        NB_LINE_NUM num = 10;
        NB_SIZE pos = 5;  /* "GOTO " の後 */
        NB_STATE state = NB_STATE_RUN_MODE;
        
        TEST(IS_SUCCESS(command_goto(&num, code, sizeof(code), &pos, &state)));
        TEST(num == 100);  /* 100行目に移動 */
    }
    {
        NB_I8 code[] = "GOTO 50+50";
        NB_LINE_NUM num = 20;
        NB_SIZE pos = 5;
        NB_STATE state = NB_STATE_RUN_MODE;
        
        TEST(IS_SUCCESS(command_goto(&num, code, sizeof(code), &pos, &state)));
        TEST(num == 100);  /* 50+50=100行目に移動 */
    }

    TEST_END();
}

static void test_command_gosub_return()
{
    TEST_BEGIN("command GOSUB/RETURN test");

    {
        NB_I8 code1[] = "GOSUB 200";
        NB_I8 code2[] = "RETURN";
        NB_LINE_NUM num = 10;
        NB_SIZE pos = 6;  /* "GOSUB " の後 */
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_set(10, code1, sizeof(code1) - 1);
        memory_code_set(20, code1, sizeof(code1) - 1);
        memory_code_set(200, code2, sizeof(code2) - 1);
        memory_code_set(210, code2, sizeof(code2) - 1);
        
        /* GOSUB */
        TEST(IS_SUCCESS(command_gosub(&num, code1, sizeof(code1), &pos, &state)));
        TEST(num == 200);  /* 200行目に移動 */
        
        /* RETURN */
        pos = 6;
        TEST(IS_SUCCESS(command_return(&num, code2, sizeof(code2), &pos, &state)));
        TEST(num == 20);  /* 元の次の行(20行目)に戻る */
    }

    TEST_END();
}

static void test_command_end()
{
    TEST_BEGIN("command END test");

    {
        NB_I8 code[] = "END";
        NB_LINE_NUM num = 100;
        NB_SIZE pos = 3;  /* "END" の後 */
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_set(100, code, sizeof(code) - 1);
        memory_code_set(110, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(command_end(&num, code, sizeof(code), &pos, &state)));
        TEST(state == NB_STATE_REPL);  /* REPL状態に戻る */
        TEST(num == 110);  /* 次の行に進む */
    }

    TEST_END();
}

static void test_command_run()
{
    TEST_BEGIN("command RUN test");

    {
        NB_I8 code[] = "RUN";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 3;  /* "RUN" の後 */
        NB_STATE state = NB_STATE_REPL;
        
        memory_code_set(10, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(command_run(&num, code, sizeof(code), &pos, &state)));
        TEST(state == NB_STATE_RUN_MODE);  /* RUN_MODE状態に変更 */
        TEST(num == 10);  /* 最初の行に移動 */
    }

    TEST_END();
}

static void test_command_new()
{
    TEST_BEGIN("command NEW test");

    {
        NB_I8 code[] = "NEW";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 3;  /* "NEW" の後 */
        NB_STATE state = NB_STATE_REPL;
        NB_I8 *buf = NULL;
        NB_SIZE size = 0;
        
        /* コードを設定 */
        memory_code_set(10, code, sizeof(code) - 1);
        memory_code_set(20, code, sizeof(code) - 1);
        
        /* NEWコマンド実行 */
        TEST(IS_SUCCESS(command_new(&num, code, sizeof(code), &pos, &state)));
        
        /* コードがクリアされているか確認 */
        TEST(IS_ERROR(memory_code_get(10, &buf, &size)));
    }

    TEST_END();
}

static void test_command_list()
{
    TEST_BEGIN("command LIST test");

    {
        NB_I8 code1[] = "PRINT \"LINE10\"";
        NB_I8 code2[] = "PRINT \"LINE20\"";
        NB_I8 code3[] = "LIST";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 4;  /* "LIST" の後 */
        NB_STATE state = NB_STATE_REPL;
        
        memory_code_clear();
        memory_code_set(10, code1, sizeof(code1) - 1);
        memory_code_set(20, code2, sizeof(code2) - 1);
        
        clear_print_buffer();
        TEST(IS_SUCCESS(command_list(&num, code3, sizeof(code3), &pos, &state)));
        /* 出力が行われているはず */
        LOG("print_buffer: [%s]\r\n", print_buffer);
    }

    TEST_END();
}

static void test_command_renum()
{
    TEST_BEGIN("command RENUM test");

    {
        NB_I8 code1[] = "PRINT \"A\"";
        NB_I8 code2[] = "PRINT \"B\"";
        NB_I8 code3[] = "RENUM 100 20";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 6;  /* "RENUM " の後 */
        NB_STATE state = NB_STATE_REPL;
        NB_I8 *buf = NULL;
        NB_SIZE size = 0;
        
        memory_code_clear();
        memory_code_set(10, code1, sizeof(code1) - 1);
        memory_code_set(20, code2, sizeof(code2) - 1);
        
        TEST(IS_SUCCESS(command_renum(&num, code3, sizeof(code3), &pos, &state)));
        
        /* 行番号が振り直されているか確認 */
        TEST(IS_SUCCESS(memory_code_get(100, &buf, &size)));
        TEST(IS_SUCCESS(memory_code_get(120, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(10, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(20, &buf, &size)));
    }

    TEST_END();
}

static void test_command_del()
{
    TEST_BEGIN("command DEL test");

    {
        /* DEL 20 - 単一行削除 */
        NB_I8 code1[] = "PRINT \"LINE10\"";
        NB_I8 code2[] = "PRINT \"LINE20\"";
        NB_I8 code3[] = "PRINT \"LINE30\"";
        NB_I8 code_del[] = "DEL 20";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 4;  /* "DEL " の後 */
        NB_STATE state = NB_STATE_REPL;
        NB_I8 *buf = NULL;
        NB_SIZE size = 0;
        
        memory_code_clear();
        memory_code_set(10, code1, sizeof(code1) - 1);
        memory_code_set(20, code2, sizeof(code2) - 1);
        memory_code_set(30, code3, sizeof(code3) - 1);
        
        TEST(IS_SUCCESS(command_del(&num, code_del, sizeof(code_del), &pos, &state)));
        
        /* 20行目が削除され、10と30が残っている */
        TEST(IS_SUCCESS(memory_code_get(10, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(20, &buf, &size)));
        TEST(IS_SUCCESS(memory_code_get(30, &buf, &size)));
    }
    {
        /* DEL 10 20 - 範囲削除 */
        NB_I8 code1[] = "PRINT \"LINE10\"";
        NB_I8 code2[] = "PRINT \"LINE20\"";
        NB_I8 code3[] = "PRINT \"LINE30\"";
        NB_I8 code4[] = "PRINT \"LINE40\"";
        NB_I8 code_del[] = "DEL 20 30";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 4;  /* "DEL " の後 */
        NB_STATE state = NB_STATE_REPL;
        NB_I8 *buf = NULL;
        NB_SIZE size = 0;
        
        memory_code_clear();
        memory_code_set(10, code1, sizeof(code1) - 1);
        memory_code_set(20, code2, sizeof(code2) - 1);
        memory_code_set(30, code3, sizeof(code3) - 1);
        memory_code_set(40, code4, sizeof(code4) - 1);
        
        TEST(IS_SUCCESS(command_del(&num, code_del, sizeof(code_del), &pos, &state)));
        
        /* 20と30が削除され、10と40が残っている */
        TEST(IS_SUCCESS(memory_code_get(10, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(20, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(30, &buf, &size)));
        TEST(IS_SUCCESS(memory_code_get(40, &buf, &size)));
    }
    {
        /* DEL - パラメータなしで先頭行削除 */
        NB_I8 code1[] = "PRINT \"LINE10\"";
        NB_I8 code2[] = "PRINT \"LINE20\"";
        NB_I8 code_del[] = "DEL";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 3;  /* "DEL" の後 */
        NB_STATE state = NB_STATE_REPL;
        NB_I8 *buf = NULL;
        NB_SIZE size = 0;
        
        memory_code_clear();
        memory_code_set(10, code1, sizeof(code1) - 1);
        memory_code_set(20, code2, sizeof(code2) - 1);
        
        TEST(IS_SUCCESS(command_del(&num, code_del, sizeof(code_del), &pos, &state)));
        
        /* 先頭の10が削除され、20が残っている */
        TEST(IS_ERROR(memory_code_get(10, &buf, &size)));
        TEST(IS_SUCCESS(memory_code_get(20, &buf, &size)));
    }
    {
        /* DEL 10 - 単一行を範囲として指定（開始と終了が同じ） */
        NB_I8 code1[] = "PRINT \"LINE5\"";
        NB_I8 code2[] = "PRINT \"LINE10\"";
        NB_I8 code3[] = "PRINT \"LINE15\"";
        NB_I8 code_del[] = "DEL 10 10";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 4;  /* "DEL " の後 */
        NB_STATE state = NB_STATE_REPL;
        NB_I8 *buf = NULL;
        NB_SIZE size = 0;
        
        memory_code_clear();
        memory_code_set(5, code1, sizeof(code1) - 1);
        memory_code_set(10, code2, sizeof(code2) - 1);
        memory_code_set(15, code3, sizeof(code3) - 1);
        
        TEST(IS_SUCCESS(command_del(&num, code_del, sizeof(code_del), &pos, &state)));
        
        /* 10行目のみが削除 */
        TEST(IS_SUCCESS(memory_code_get(5, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(10, &buf, &size)));
        TEST(IS_SUCCESS(memory_code_get(15, &buf, &size)));
    }

    TEST_END();
}

int main(int argc, char *argv[])
{
    memory_init(memory, CODE_SIZE, VALUE_SIZE, STACK_SIZE);
    command_init();
    
    test_command_rem();
    test_command_let();
    test_command_print();
    test_command_if();
    test_command_goto();
    test_command_gosub_return();
    test_command_end();
    test_command_run();
    test_command_new();
    test_command_list();
    test_command_renum();
    test_command_del();
    
    return 0;
}
