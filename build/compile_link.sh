PROJECT_DIR=/Users/vapostolop/Desktop/pyRoPE
include=$PROJECT_DIR/include

include=/usr/local/Cellar/python@3.9/3.9.18/Frameworks/Python.framework/Versions/3.9/include/python3.9
numpy_inc=/usr/local/lib/python3.9/site-packages/numpy/core/include/
lib=/usr/local/Cellar/python\@3.9/3.9.18/Frameworks/Python.framework/Versions/3.9/lib
link=-lpython3.9
source_rope=$PROJECT_DIR/rope
lib_rope=/Users/vapostolop/Desktop/ROPE/rope/build
static_files_1=/Users/vapostolop/Desktop/ROPE/rope/build/diagnose/vagabond/core/libvag.a
static_files_2=/Users/vapostolop/Desktop/ROPE/rope/build/diagnose/vagabond/utils/libvagutils.a
static_files_3=/Users/vapostolop/Desktop/ROPE/rope/build/diagnose/vagabond/c4x/libcluster4x.a
static_gemmi=/Users/vapostolop/Desktop/ROPE/gemmi/libgemmi_cpp.a
static_fttw=/usr/local/lib/libfftw3f.a

g++ -c -std=c++14 -fpic $PROJECT_DIR/src/module.cpp -I $include -I $numpy_inc -I $lib_rope -I $source_rope -o $PROJECT_DIR/build/module.o
#g++ -shared -undefined dynamic_lookup module.o -o module.so -L $lib $link

clang -bundle -undefined dynamic_lookup -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX13.sdk $static_fttw $static_files_1 $static_files_2 $static_files_3 $static_gemmi $PROJECT_DIR/build/module.o -o $PROJECT_DIR/build/module.so