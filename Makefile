CC=cc
WFLAGS=-Wall -Wextra
CFLAGS=-g -Isrc -rdynamic -DNDEBUG $(WFLAGS) $(OPTFLAGS)
LIBS=-ldl $(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/**/*.c src/**/*.c src/*.c)
OBJECTS=$(patsubst %c, %o, $(SOURCES))

TEST_SRC=$(wildcard test/*_test.c)
TESTS=$(patsubst %.c, %, $(TEST_SRC))

TARGET=build/libnaive.a
SO_TARGET=$(patsubst %.a, %.so, $(TARGET))

# The Target Build
all: $(TARGET) $(SO_TARGET) test

dev: CFLAGS=-g -Wall -Isrc -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@
$(SO_TARGET): $(TARGET) $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: test
test: CFLAGS += $(TARGET)
test: $(TESTS)
	sh ./test/run_test.sh

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -f test/process.log
	find . -name "*.gc*" -exec -r, {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

check:
	@echo Files with potentially dangerous functions.
	@egrep '[^.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(SOURCES) || true
# -ftrapv
# -ftrapv function will cause the program to abort on signed integer overflow (formally "undefined behaviour" in C).
