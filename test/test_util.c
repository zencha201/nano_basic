#include "nano_basic_local.h"
#include "test.h"

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

static void test_util_mem()
{
        NB_I8 buf1[32];
        NB_I8 buf2[32];
        TEST_BEGIN("util mem test");

    {
        util_memset(buf1, 0x00, sizeof(buf1));
        util_memset(buf2, 0xff, sizeof(buf2));
        TEST(IS_SUCCESS(util_memcpy(buf1, buf2, sizeof(buf1), sizeof(buf2))));
        SHOW_BUF(buf1, sizeof(buf1));
    }
    {
        util_memset(buf1, 0x00, sizeof(buf1));
        util_memset(buf2, 0xff, sizeof(buf2));
        TEST(IS_ERROR(util_memcpy(buf1, buf2, sizeof(buf1), sizeof(buf2) - 1)));
        SHOW_BUF(buf1, sizeof(buf1));
    }
    {
        util_memset(buf1, 0x00, sizeof(buf1));
        util_memset(buf2, 0xff, sizeof(buf2));
        TEST(IS_SUCCESS(util_memcpy(buf1, buf2, sizeof(buf1) - 1, sizeof(buf2))));
        SHOW_BUF(buf1, sizeof(buf1));
    }
    {
        util_memset(buf1, 0x00, sizeof(buf1));
        util_memset(buf2, 0xff, sizeof(buf2));
        TEST(IS_SUCCESS(util_memcpy(buf1 + 1, buf2, sizeof(buf1) - 1, sizeof(buf2) - 1)));
        SHOW_BUF(buf1, sizeof(buf1));
    }
    {
        util_memset(buf1, 0x00, sizeof(buf1));
        buf1[0] = 0x01;
        buf1[1] = 0x02;
        buf1[2] = 0x03;
        buf1[3] = 0x04;
        buf1[4] = 0x05;
        TEST(IS_SUCCESS(util_memmove(buf1, buf1 + 2, 3)));
        TEST(buf1[0] == 0x03);
        TEST(buf1[1] == 0x04);
        TEST(buf1[2] == 0x05);
        TEST(buf1[3] == 0x04);
        TEST(buf1[4] == 0x05);
        SHOW_BUF(buf1, sizeof(buf1));
    }
    {
        util_memset(buf1, 0x00, sizeof(buf1));
        buf1[0] = 0x01;
        buf1[1] = 0x02;
        buf1[2] = 0x03;
        buf1[3] = 0x04;
        buf1[4] = 0x05;
        TEST(IS_SUCCESS(util_memmove(buf1 + 2, buf1, 3)));
        TEST(buf1[0] == 0x01);
        TEST(buf1[1] == 0x02);
        TEST(buf1[2] == 0x01);
        TEST(buf1[3] == 0x02);
        TEST(buf1[4] == 0x03);
        SHOW_BUF(buf1, sizeof(buf1));
    }
    {
        util_memset(buf1, 0x00, sizeof(buf1));
        util_memset(buf2, 0xff, sizeof(buf2));
        TEST(IS_ERROR(util_memcpy(buf1 + 1, buf2, sizeof(buf1) - 1, sizeof(buf2) - 2)));
        SHOW_BUF( buf1, sizeof(buf1));
    }

    TEST_END();

}

static void test_util_str()
{
    TEST_BEGIN("util str test");
    {
        NB_I8 str[] = "123";
        NB_SIZE pos = 0;
        NB_I8 ch = ' ';
        TEST(util_strskipuntil(str, &pos, ch, sizeof(str)) == NB_TRUE);
        TEST(str[pos] == '1');
        TEST(pos == 0);
    }
    {
        NB_I8 str[] = "  3";
        NB_SIZE pos = 0;
        NB_I8 ch = ' ';
        TEST(util_strskipuntil(str, &pos, ch, sizeof(str)) == NB_TRUE);
        TEST(str[pos] == '3');
        TEST(pos == 2);
    }
    {
        NB_I8 str[] = "   ";
        NB_SIZE pos = 0;
        NB_I8 ch = ' ';
        TEST(util_strskipuntil(str, &pos, ch, sizeof(str)) == NB_FALSE);
        TEST(str[pos] == '\0');
        TEST(pos == 3);
    }
    {
        NB_I8 str1[] = "abc";
        NB_I8 str2[] = "abc";
        TEST(util_strcmp(str1, str2, util_strlen(str1)) == NB_TRUE);
    }
    {
        NB_I8 str1[] = "abc";
        NB_I8 str2[] = "axc";
        TEST(util_strcmp(str1, str2, util_strlen(str1)) == NB_FALSE);
    }
    {
        NB_I8 str1[] = "abcd";
        NB_I8 str2[] = "abc";
        TEST(util_strcmp(str1, str2, util_strlen(str1)) == NB_FALSE);
    }
    {
        NB_I8 str1[] = "abc";
        NB_I8 str2[] = "abcd";
        TEST(util_strcmp(str1, str2, util_strlen(str1)) == NB_TRUE);
    }
    {
        NB_I8 str[] = "123";
        NB_SIZE pos = 0;
        NB_I16 value;
        TEST(IS_SUCCESS(util_atoi(str, &pos, sizeof(str), &value)));
        LOG("pos:%d, value:%d\r\n", pos, value);
        TEST(value == 123);
        TEST(pos == 3);
    }
    {
        NB_I8 str[] = "234 end";
        NB_SIZE pos = 0;
        NB_I16 value;
        TEST(IS_SUCCESS(util_atoi(str, &pos, sizeof(str), &value)));
        LOG("pos:%d, value:%d\r\n", pos, value);
        TEST(value == 234);
        TEST(pos == 3);
    }
    {
        NB_I8 str[] = "-234 end";
        NB_SIZE pos = 0;
        NB_I16 value;
        TEST(IS_SUCCESS(util_atoi(str, &pos, sizeof(str), &value)));
        LOG("pos:%d, value:%d\r\n", pos, value);
        TEST(value == -234);
        TEST(pos == 4);
    }
    {
        NB_I8 str[] = "abcdef";
        NB_SIZE size = 0;
        TEST((size = util_strlen(str)) == sizeof(str) - 1);
    }
    TEST_END();
}

int main(int argc, char *argv[])
{
    test_util_mem();
    test_util_str();
    return 0;
}
