CC = gcc
RM = rm

# ビルドモード設定
BUILD = release
#BUILD = debug

# ビルドターゲット
TARGET_TYPE = console
#TARGET_TYPE = memory_test
#TARGET_TYPE = util_test

# nano nasicライブラリ
LIB_PATH = ./lib/
LIB = libnanobasic.so

LDLAGS = 
CFLAGS = -I./include

# デバッグモード設定
ifeq ($(BUILD), debug)
CFLAGS += -g0
CFLAGS += -ggdb
CFLAGS += -D__DEBUG__
endif

# ソース
TARGET = nano_basic
SRCS =  src/calc.c
SRCS += src/command.c
SRCS += src/debug.c
SRCS += src/interpreter.c
SRCS += src/memory.c
SRCS += src/util.c

TARGET_SRCS = 

# ビルドモード、オプションによる設定
ifeq ($(TARGET_TYPE), memory_test)
CFLAGS += -I./src
CFLAGS += -I./test
TARGET_SRCS += test/test_memory.c
TARGET = memory_test
else ifeq ($(TARGET_TYPE), util_test)
CFLAGS += -I./src
CFLAGS += -I./test
TARGET_SRCS += test/test_util.c
TARGET = util_test
else ifeq ($(TARGET_TYPE), calc_test)
CFLAGS += -I./src
CFLAGS += -I./test
TARGET_SRCS += test/test_calc.c
TARGET = calc_test
else ifeq ($(TARGET_TYPE), command_test)
CFLAGS += -I./src
CFLAGS += -I./test
TARGET_SRCS += test/test_command.c
TARGET = command_test
else ifeq ($(TARGET_TYPE), interpreter_test)
CFLAGS += -I./src
CFLAGS += -I./test
TARGET_SRCS += test/test_interpreter.c
TARGET = interpreter_test
else
TARGET_SRCS += test/console.c
endif

# ソースからオブジェクトのリストを生成
OBJS = $(SRCS:%.c=%.o)
TARGET_OBJS = $(TARGET_SRCS:%.c=%.o)

all: $(TARGET)

run: all
	LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./$(TARGET)

$(TARGET) : $(LIB) $(TARGET_OBJS)
	$(CC) -o $@ $(LDLAGS) $(TARGET_OBJS) -L$(LIB_PATH) -l$(LIB:lib%.so=%)

$(LIB) : $(OBJS)
	mkdir -p $(LIB_PATH)
	$(CC) -shared -fPIC -o $(LIB_PATH)/$@ $^

.c.o:
	$(CC) -fPIC -c $(CFLAGS) $< -o $(<:%.c=%.o)

clean:
	$(RM) -rf $(OBJS)
	$(RM) -rf $(TARGET_OBJS)
	$(RM) -rf $(LIB_PATH)/$(LIB)
	$(RM) -rf $(TARGET)
	$(RM) -rf memory_test util_test calc_test command_test interpreter_test
	$(RM) -rf test/test_memory.o test/test_util.o test/test_calc.o test/test_command.o test/test_interpreter.o

# 静的解析ターゲット
CPPCHECK = cppcheck
CPPCHECK_FLAGS = --enable=all --std=c99 --platform=unix64 --suppress=missingIncludeSystem
CPPCHECK_FLAGS += -I./include -I./src
CPPCHECK_REPORT = static_analysis_report.txt

.PHONY: sa

sa:
	@which $(CPPCHECK) > /dev/null 2>&1 || \
		(sudo apt-get update && \
		sudo apt-get install -y cppcheck)
	@$(CPPCHECK) $(CPPCHECK_FLAGS) $(SRCS) $(TARGET_SRCS) 2>&1 | tee $(CPPCHECK_REPORT)
	@grep -E "(error|warning|style|performance|portability)" $(CPPCHECK_REPORT) | wc -l || echo "0"
	@echo "issues detected"

# テスト実行ターゲット
.PHONY: test test_all

test_all: test_memory test_util test_calc test_command test_interpreter

test_memory:
	@echo "=== Running Memory Tests ==="
	@$(MAKE) TARGET_TYPE=memory_test clean all
	@LD_LIBRARY_PATH=./lib:$$LD_LIBRARY_PATH ./memory_test

test_util:
	@echo "=== Running Util Tests ==="
	@$(MAKE) TARGET_TYPE=util_test clean all
	@LD_LIBRARY_PATH=./lib:$$LD_LIBRARY_PATH ./util_test

test_calc:
	@echo "=== Running Calc Tests ==="
	@$(MAKE) TARGET_TYPE=calc_test clean all
	@LD_LIBRARY_PATH=./lib:$$LD_LIBRARY_PATH ./calc_test

test_command:
	@echo "=== Running Command Tests ==="
	@$(MAKE) TARGET_TYPE=command_test clean all
	@LD_LIBRARY_PATH=./lib:$$LD_LIBRARY_PATH ./command_test

test_interpreter:
	@echo "=== Running Interpreter Tests ==="
	@$(MAKE) TARGET_TYPE=interpreter_test clean all
	@LD_LIBRARY_PATH=./lib:$$LD_LIBRARY_PATH ./interpreter_test
