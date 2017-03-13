project(virtest)
cmake_minimum_required(VERSION 2.6)
install(DIRECTORY vir DESTINATION include/vir)

enable_testing()
add_subdirectory(tests)
