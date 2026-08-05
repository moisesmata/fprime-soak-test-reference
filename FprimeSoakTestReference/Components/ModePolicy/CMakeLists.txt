####
# F Prime CMakeLists.txt:
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
####
set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/ModePolicy.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/ModePolicy.cpp"
)

register_fprime_module()

# Uncomment when unit tests are added:
# add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test/ut/")
