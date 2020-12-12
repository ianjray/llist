CC       = clang
SANITIZE = -g -O0 -fsanitize=address -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error
COVERAGE = -fprofile-instr-generate -fcoverage-mapping
OPTS     = $(SANITIZE) $(COVERAGE) -Weverything -Wno-padded -Wno-poison-system-directories

llist : llist.c
	$(CC) $(OPTS) -DUNITTEST_LLIST $^ -o $@

.PHONY : clean
clean :
	rm -rf llist llist.dSYM llist.profdata llist.profraw

.PHONY : coverage
coverage : llist.profdata
	xcrun llvm-cov show ./llist -instr-profile=$<

%.profdata : %.profraw
	xcrun llvm-profdata merge -sparse $< -o $@

%.profraw : %
	LLVM_PROFILE_FILE=$@ ./$<
