ifeq ($(ROOT),)
ROOT:=.
endif

CC := gcc

CFLAGS:=-O3 -std=c99 -fpermissive -Wno-error=incompatible-pointer-types -Wall -Wextra

SRC      := $(ROOT)/src
INCLUDE  := -I$(ROOT)/benchmark

PROJ ?= hqc-1
# PROJ ?= hqc-3
# PROJ ?= hqc-5

TARGET ?= m1_hqcport

TARG := m1_hqcport

CFLAGS+= -std=c99 -mcpu=apple-m1 -mtune=apple-m1 -D_APPLE_SILICON_ -pedantic
INCLUDE += -I$(SRC)/armv8 -I$(SRC)/common

ifeq ($(PROJ),hqc-1)
INCLUDE      += -I$(SRC)/armv8/hqc-1 -I$(SRC)/common/hqc-1
else ifeq ($(PROJ),hqc-3)
INCLUDE      += -I$(SRC)/armv8/hqc-3 -I$(SRC)/common/hqc-3
else ifeq ($(PROJ),hqc-5)
INCLUDE      += -I$(SRC)/armv8/hqc-5 -I$(SRC)/common/hqc-5
endif


HQC_OBJS:= code.o crypto_memset.o fft.o kem.o symmetric.o keccak_permutation.o fips202.o gf.o gf2x.o hqc.o parsing.o reed_muller.o reed_solomon.o vector.o 
#HQC_OBJS_VERBOSE:=vector.o reed_muller.o reed_solomon-verbose.o fft.o gf.o gf2x.o code-verbose.o parsing.o hqc-verbose.o kem-verbose.o shake_ds.o shake_prng.o api_cachekey.o

TEST_DIR:=$(ROOT)/tests
bench_INCLUDE:=
OS := $(shell uname -s)
ARCH := $(shell uname -m)
ifeq  ($(OS), Darwin)
ifeq  ($(ARCH), arm64)
 	LIB_OBJS    +=  bench.o bench_macos.o
	bench_INCLUDE:=-I $(ROOT)/benchmark
endif
endif

BIN:=bin
BUILD:=bin/build

all: code-test kem2 hashBench hqc-test code-bench gf2x-test pkeBench
folders:
	@echo -e "\n### Creating folders\n"
	mkdir -p $(BUILD)

%.o: $(SRC)/common/%.c | folders
	@/bin/echo -e "\n### Compiling $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(LIB) -o $(BUILD)/$@

%.o: $(SRC)/armv8/%.c | folders
	@/bin/echo -e "\n### Compiling $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(LIB) -o $(BUILD)/$@

%.o : $(SRC)/armv8/$(PROJ)/%.c | folders
	@/bin/echo -e "\n### Compiling $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(LIB) -o $(BUILD)/$@

%.o: $(ROOT)/benchmark/%.c | folders
	@/bin/echo -e "\n### Compiling $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(LIB) -o $(BUILD)/$@

%.o: $(TEST_DIR)/%.c | folders
	@/bin/echo -e "\n### Compiling $@"
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(LIB) -o $(BUILD)/$@

%-verbose.o: $(SRC)/%.c | folders
	@echo -e "\n### Compiling $@ (verbose mode)\n"
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(LIB) -D VERBOSE -o $(BUILD)/$@

hqc-128: $(HQC_OBJS) $(LIB_OBJS) | folders
	@echo -e "\n### Compiling hqc-256\n"
	$(CC) $(CFLAGS) $(MAIN_HQC) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@

hqc-128-kat: $(HQC_OBJS) $(LIB_OBJS) | folders
	@echo -e "\n### Compiling hqc-256 KAT\n"
	$(CC) $(CFLAGS) $(MAIN_KAT) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@

hqc-128-verbose: $(HQC_OBJS_VERBOSE) $(LIB_OBJS) | folders
	@echo -e "\n### Compiling hqc-256 (verbose mode)\n"
	$(CC) $(CFLAGS) $(MAIN_HQC) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -D VERBOSE -o $(BIN)/$@

gf2x-test: gf2x-test.o $(HQC_OBJS) $(LIB_OBJS) | folders
	@/bin/echo -e "\n### Compiling gf2x-test.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@

hqc-test: hqc-test.o $(HQC_OBJS) $(filter-out bench.o,$(LIB_OBJS)) | folders
	@/bin/echo -e "\n### Compiling hqc-test.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@

code-bench: code-bench.o $(HQC_OBJS) $(LIB_OBJS) | folders
	@/bin/echo -e "\n### Compiling code-bench.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@

code-test: code-test.o $(HQC_OBJS) $(filter-out bench.o,$(LIB_OBJS)) | folders
	@/bin/echo -e "\n### Compiling code-test.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@


kem2: kem2.o bench.o $(HQC_OBJS) $(LIB_OBJS) | folders
	@/bin/echo -e "\n### Compiling kem2.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@

hashBench: hashBench.o bench.o $(HQC_OBJS) $(LIB_OBJS) | folders
	@/bin/echo -e "\n### Compiling hashBench.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@

pkeBench: pkeBench.o bench.o $(HQC_OBJS) $(LIB_OBJS) | folders
	@/bin/echo -e "\n### Compiling pkeBench.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@


%-test: %-test.o $(HQC_OBJS) $(LIB_OBJS) | folders
	@/bin/echo -e "\n### Compiling hqc-test.c"
	$(CC) $(CFLAGS) $(addprefix $(BUILD)/, $^) $(INCLUDE) $(LIB) -o $(BIN)/$@


clean:
	rm -f PQCkemKAT_*
	rm -f vgcore.*
	rm -rf ./bin
