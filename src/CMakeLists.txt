cmake_minimum_required(VERSION 3.8)

enable_language(CXX)

file(GLOB files *.h *.cpp) 

add_library(TLSFAllocator SHARED ${files})

target_compile_features(TLSFAllocator PUBLIC cxx_std_17)
