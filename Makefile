CXX=clang++
CFLAGS=-g -O3 `llvm-config --cxxflags --ldflags --system-libs --libs all` \
-Wno-unused-function -Wno-unknown-warning-option -ferror-limit=200 -o mccomp

mccomp: mccomp.cpp ast/ast.cpp parser/parser.cpp exceptions/exceptions.cpp
	$(CXX) mccomp.cpp ast/ast.cpp parser/parser.cpp exceptions/exceptions.cpp $(CFLAGS)

clean:
	rm -rf mccomp 
