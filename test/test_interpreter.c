#include "nano_basic_local.h"
#include "test.h"

#define CODE_SIZE (256)
#define VALUE_SIZE (72)
#define STACK_SIZE (32)
static NB_I8 memory[CODE_SIZE + VALUE_SIZE + STACK_SIZE];

/* プラットフォーム関数のモック */
void platform_print_ch(NB_I8 ch)
{
}

NB_VALUE platform_import(NB_VALUE num)
{
    return num * 2;
}

NB_BOOL platform_fopen(const NB_I8 *name, NB_BOOL write_mode)
{
    return NB_FALSE;
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

static void test_nano_basic_init()
{
    TEST_BEGIN("nano_basic_init test");

    {
        nano_basic_init(memory, CODE_SIZE, VALUE_SIZE, STACK_SIZE);
        /* 初期化が成功すればOK */
        TEST(1);
    }

    TEST_END();
}

static void test_nano_basic_add_command()
{
    TEST_BEGIN("nano_basic_add_command test");

    NB_RESULT dummy_command(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
    {
        return NB_RESULT_SUCCESS;
    }

    {
        nano_basic_init(memory, CODE_SIZE, VALUE_SIZE, STACK_SIZE);
        TEST(nano_basic_add_command("TEST", dummy_command) == NB_TRUE);
    }

    TEST_END();
}

static void test_nano_basic_set_code_with_line_number()
{
    TEST_BEGIN("nano_basic_set_code with line number test");

    {
        NB_I8 buf[] = "10 PRINT \"HELLO\"";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        
        memory_code_clear();
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(num == 10);
        TEST(pos > 0);
        
        /* コードが格納されているか確認 */
        NB_I8 *stored_code = NULL;
        NB_SIZE stored_size = 0;
        TEST(IS_SUCCESS(memory_code_get(10, &stored_code, &stored_size)));
        LOG("stored_code: [%s]\r\n", stored_code);
    }
    {
        NB_I8 buf[] = "20 LET A 123";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(num == 20);
    }
    {
        NB_I8 buf[] = "30 IF A>100 THEN PRINT A";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(num == 30);
    }

    TEST_END();
}

static void test_nano_basic_set_code_uppercase_conversion()
{
    TEST_BEGIN("nano_basic_set_code uppercase conversion test");

    {
        NB_I8 buf[] = "10 print \"hello\"";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        NB_I8 *stored_code = NULL;
        NB_SIZE stored_size = 0;
        
        memory_code_clear();
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(IS_SUCCESS(memory_code_get(10, &stored_code, &stored_size)));
        
        /* コマンド部分が大文字に変換されているか確認 */
        LOG("stored_code: [%s]\r\n", stored_code);
        TEST(stored_code[0] == 'P');
        TEST(stored_code[1] == 'R');
        TEST(stored_code[2] == 'I');
        TEST(stored_code[3] == 'N');
        TEST(stored_code[4] == 'T');
        
        /* 文字列内は小文字のまま */
        NB_SIZE i = 0;
        for(i = 0; i < stored_size; i++) {
            if(stored_code[i] == 'h') {
                TEST(stored_code[i+1] == 'e');
                TEST(stored_code[i+2] == 'l');
                TEST(stored_code[i+3] == 'l');
                TEST(stored_code[i+4] == 'o');
                break;
            }
        }
    }
    {
        NB_I8 buf[] = "20 let a 100";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        NB_I8 *stored_code = NULL;
        NB_SIZE stored_size = 0;
        
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(IS_SUCCESS(memory_code_get(20, &stored_code, &stored_size)));
        
        LOG("stored_code: [%s]\r\n", stored_code);
        TEST(stored_code[0] == 'L');
        TEST(stored_code[1] == 'E');
        TEST(stored_code[2] == 'T');
        /* 変数名も大文字に */
        TEST(stored_code[4] == 'A');
    }

    TEST_END();
}

static void test_nano_basic_set_code_space_normalization()
{
    TEST_BEGIN("nano_basic_set_code space normalization test");

    {
        NB_I8 buf[] = "10   PRINT    \"TEST\"";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        NB_I8 *stored_code = NULL;
        NB_SIZE stored_size = 0;
        
        memory_code_clear();
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(IS_SUCCESS(memory_code_get(10, &stored_code, &stored_size)));
        
        LOG("stored_code: [%s]\r\n", stored_code);
        
        /* 複数の空白が1つに正規化されているか確認 */
        NB_SIZE space_count = 0;
        NB_SIZE i = 0;
        for(i = 0; i < stored_size - 1; i++) {
            if(stored_code[i] == ' ' && stored_code[i+1] == ' ') {
                space_count++;
            }
        }
        TEST(space_count == 0);  /* 連続した空白がないこと */
    }

    TEST_END();
}

static void test_nano_basic_set_code_without_line_number()
{
    TEST_BEGIN("nano_basic_set_code without line number test");

    {
        NB_I8 buf[] = "LIST";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        
        memory_code_clear();
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(num == 0);  /* 行番号なしは0行目として扱われる */
        
        /* 0行目に格納されているか確認 */
        NB_I8 *stored_code = NULL;
        NB_SIZE stored_size = 0;
        TEST(IS_SUCCESS(memory_code_get(0, &stored_code, &stored_size)));
        LOG("stored_code: [%s]\r\n", stored_code);
    }
    {
        NB_I8 buf[] = "RUN";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        
        TEST(IS_SUCCESS(nano_basic_set_code(buf, sizeof(buf), &num, &pos)));
        TEST(num == 0);
    }

    TEST_END();
}

static void test_nano_basic_proc_command()
{
    TEST_BEGIN("nano_basic_proc_command test");

    {
        NB_I8 code[] = "PRINT \"TEST\"";
        NB_LINE_NUM num = 10;
        NB_SIZE pos = 0;
        NB_STATE state = NB_STATE_RUN_MODE;
        
        memory_code_clear();
        memory_code_set(10, code, sizeof(code) - 1);
        memory_code_set(20, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(nano_basic_proc_command(&num, code, sizeof(code), &pos, &state)));
        TEST(num == 20);  /* 次の行に進む */
    }
    {
        NB_I8 code[] = "LET A 456";
        NB_LINE_NUM num = 20;
        NB_SIZE pos = 0;
        NB_STATE state = NB_STATE_RUN_MODE;
        NB_VALUE value = 0;
        
        memory_code_set(20, code, sizeof(code) - 1);
        memory_code_set(30, code, sizeof(code) - 1);
        
        TEST(IS_SUCCESS(nano_basic_proc_command(&num, code, sizeof(code), &pos, &state)));
        TEST(IS_SUCCESS(memory_variable_get(0, &value)));
        TEST(value == 456);
        TEST(num == 30);
    }
    {
        NB_I8 code[] = "UNKNOWN_COMMAND";
        NB_LINE_NUM num = 30;
        NB_SIZE pos = 0;
        NB_STATE state = NB_STATE_RUN_MODE;
        
        /* 未知のコマンドはエラーになる */
        TEST(IS_ERROR(nano_basic_proc_command(&num, code, sizeof(code), &pos, &state)));
    }

    TEST_END();
}

static void test_nano_basic_proc_repl_mode()
{
    TEST_BEGIN("nano_basic_proc REPL mode test");

    {
        NB_I8 buf[] = "10 PRINT \"HELLO\"";
        NB_STATE state = NB_STATE_REPL;
        NB_I8 *stored_code = NULL;
        NB_SIZE stored_size = 0;
        
        memory_code_clear();
        state = nano_basic_proc(state, buf, sizeof(buf));
        
        /* 行番号付きのコードはメモリに格納されるだけ */
        TEST(state == NB_STATE_REPL);
        TEST(IS_SUCCESS(memory_code_get(10, &stored_code, &stored_size)));
    }
    {
        NB_I8 buf[] = "20 LET A 789";
        NB_STATE state = NB_STATE_REPL;
        
        state = nano_basic_proc(state, buf, sizeof(buf));
        TEST(state == NB_STATE_REPL);
    }

    TEST_END();
}

static void test_nano_basic_set_input_value()
{
    TEST_BEGIN("nano_basic_set_input_value test");

    {
        nano_basic_set_input_value(999);
        /* エラーが発生しなければOK */
        TEST(1);
    }

    TEST_END();
}

static void test_code_update_and_delete()
{
    TEST_BEGIN("code update and delete test");

    {
        NB_I8 buf1[] = "10 PRINT \"FIRST\"";
        NB_I8 buf2[] = "10 PRINT \"SECOND\"";
        NB_I8 buf3[] = "10";
        NB_LINE_NUM num = 0;
        NB_SIZE pos = 0;
        NB_I8 *stored_code = NULL;
        NB_SIZE stored_size = 0;
        
        memory_code_clear();
        
        /* 最初の行を設定 */
        TEST(IS_SUCCESS(nano_basic_set_code(buf1, sizeof(buf1), &num, &pos)));
        TEST(IS_SUCCESS(memory_code_get(10, &stored_code, &stored_size)));
        LOG("first: [%s]\r\n", stored_code);
        
        /* 同じ行番号で更新 */
        TEST(IS_SUCCESS(nano_basic_set_code(buf2, sizeof(buf2), &num, &pos)));
        TEST(IS_SUCCESS(memory_code_get(10, &stored_code, &stored_size)));
        LOG("second: [%s]\r\n", stored_code);
        /* "SECOND"が含まれているはず */
        
        /* 行番号のみで削除 */
        TEST(IS_SUCCESS(nano_basic_set_code(buf3, sizeof(buf3), &num, &pos)));
        TEST(IS_ERROR(memory_code_get(10, &stored_code, &stored_size)));
        LOG("after delete, get should fail\r\n");
    }

    TEST_END();
}

int main(int argc, char *argv[])
{
    nano_basic_init(memory, CODE_SIZE, VALUE_SIZE, STACK_SIZE);
    
    test_nano_basic_init();
    test_nano_basic_add_command();
    test_nano_basic_set_code_with_line_number();
    test_nano_basic_set_code_uppercase_conversion();
    test_nano_basic_set_code_space_normalization();
    test_nano_basic_set_code_without_line_number();
    test_nano_basic_proc_command();
    test_nano_basic_proc_repl_mode();
    test_nano_basic_set_input_value();
    test_code_update_and_delete();
    
    return 0;
}
