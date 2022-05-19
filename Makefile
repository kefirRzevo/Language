CXX    = gcc
TARGET = app
#CXXFLAGS = -D NDEBUG -g -std=c++14 -fmax-errors=100 -Wall 				\
	   -Wextra -Weffc++  												\
	   -Waggressive-loop-optimizations -Wc++0x-compat -Wc++11-compat    \
	   -Wc++14-compat -Wcast-align -Wcast-qual -Wchar-subscripts        \
	   -Wconditionally-supported -Wconversion -Wctor-dtor-privacy       \
	   -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security \
	   -Wformat-signedness -Wformat=2 -Winline -Wlogical-op             \
	   -Wmissing-declarations -Wnon-virtual-dtor -Wopenmp-simd          \
	   -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls  \
	   -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=8192       \
	   -Wstrict-null-sentinel -Wstrict-overflow=2                       \
	   -Wsuggest-attribute=noreturn -Wsuggest-final-methods             \
	   -Wsuggest-final-types -Wsuggest-override -Wswitch-default        \
	   -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code             \
	   -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix    \
	   -Wno-missing-field-initializers -Wno-narrowing                   \
	   -Wno-old-style-cast -Wno-varargs  -fcheck-new                    \
	   -fsized-deallocation -fstack-check -fstack-protector             \
	   -fstrict-overflow                          						\
	   -fno-omit-frame-pointer -fPIE  -fsanitize=address                \
	   -fsanitize=bool -fsanitize=bounds  -fsanitize=enum               \
	   -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero   \
	   -fsanitize=integer-divide-by-zero               					\
	   -fsanitize=nonnull-attribute  -fsanitize=null  -fsanitize=return \
	   -fsanitize=returns-nonnull-attribute                             \
	   -fsanitize=signed-integer-overflow -fsanitize=unreachable        \
	   -fsanitize=vla-bound -fsanitize=vptr -lm -pie 	            	\
	   -Wno-format-nonliteral 

# $(CXX) -g $(CXXFLAGS) -c  main.cpp src/akinator.cpp src/dump.cpp src/parser.cpp src/tree.cpp
#"${workspaceFolder}\\/main.cpp", "${workspaceFolder}\\/lib/hash.cpp", "${workspaceFolder}\\/lib/operators.cpp", "${workspaceFolder}/\\src/lib/tree_viz_dump.cpp", "${workspaceFolder}/\\lib/tree.cpp", "${workspaceFolder}/\\src/differentiator.cpp", "${workspaceFolder}/\\src/optimization.cpp", "${workspaceFolder}/\\src/parser.cpp"
	
make: 
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/main.o 				-c main.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/array.o 				-c src/array.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/symtable.o 		    -c src/symtable.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/tree_dump.o 			-c src/tree_dump.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/tree.o 				-c src/tree.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/lexer.o 	    		-c frontend/lexer.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/recursive_descent.o 	-c frontend/recursive_descent.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/compiler.o 			-c backend/compiler.cpp
	$(CXX) -o $(TARGET) obj/main.o obj/array.o obj/symtable.o obj/tree_dump.o obj/tree.o obj/lexer.o obj/recursive_descent.o obj/compiler.o

front:
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/lexer.o 				-c frontend/lexer.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/recursive_descent.o 	-c frontend/recursive_descent.cpp
	$(CXX) -o $(TARGET) obj/main.o obj/array.o obj/symtable.o obj/tree_dump.o obj/tree.o obj/lexer.o obj/recursive_descent.o obj/compiler.o

lex:
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/lexer.o 				-c frontend/lexer.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/main.o 				-c main.cpp
	$(CXX) -o $(TARGET) obj/main.o obj/array.o obj/symtable.o obj/tree_dump.o obj/tree.o obj/lexer.o obj/recursive_descent.o obj/compiler.o

back:
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/compiler.o 			-c backend/compiler.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/symtable.o 		    -c src/symtable.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/main.o 				-c main.cpp
	$(CXX) -o $(TARGET) obj/main.o obj/array.o obj/symtable.o obj/tree_dump.o obj/tree.o obj/lexer.o obj/recursive_descent.o obj/compiler.o
	./$(TARGET)

sources: main.cpp src/array.cpp src/symtable.cpp src/tree_dump.cpp src/tree.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/main.o 				-c main.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/array.o 				-c src/array.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/symtable.o 			-c src/symtable.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/tree_dump.o 			-c src/tree_dump.cpp
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/tree.o 				-c src/tree.cpp
	$(CXX) -o $(TARGET) obj/main.o obj/array.o obj/symtable.o obj/tree_dump.o obj/tree.o obj/lexer.o obj/recursive_descent.o obj/compiler.o

x:
	$(CXX) -g -pipe $(CXXFLAGS) -o obj/tree_dump.o 			-c src/tree_dump.cpp
	$(CXX) -o $(TARGET) obj/main.o obj/array.o obj/tree_dump.o obj/tree.o obj/lexer.o obj/recursive_descent.o

$(TARGET):	obj/main.o obj/array.o  obj/tree.o obj/lexer.o obj/recursive_descent.o
	$(CXX) -o $(TARGET) obj/main.o obj/array.o obj/tree_dump.o obj/tree.o obj/lexer.o obj/recursive_descent.o

clean:
	rm -rf obj/*.o $(TARGET)

kill:
	rm -rf logfiles/*.txt
	