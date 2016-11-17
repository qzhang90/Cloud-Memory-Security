#clang-3.9 -g -O0 -emit-llvm hello.c -c -o hello.bc
clang-3.9 -gdwarf-4 -g -O0 -emit-llvm -c hello.c -o hello.bc
#opt -load ../llvmbuild/lib/LLVMHello.so -hello < hello.bc > newhello.bc
llc -O0 -filetype=obj hello.bc -dwarf-version 4
gcc hello.o -o hello


#opt -dot-callgraph hello.bc 
