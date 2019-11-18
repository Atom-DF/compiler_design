CXX=clang++
CFLAGS=-g -O3 `llvm-config --cxxflags --ldflags --system-libs --libs all` \
-Wno-unused-function -Wno-unknown-warning-option -ferror-limit=200 -frtti -o mccomp

mccomp: mccomp.cpp ast/ast.cpp ast/codegen.cpp parser/parser.cpp
	$(CXX) mccomp.cpp ast/ast.cpp parser/parser.cpp ast/codegen.cpp $(CFLAGS)

clean:
	rm -rf mccomp 

