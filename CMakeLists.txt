check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        test/qd-channel-test-suite.cc
        )
endif()

build_lib(
    LIBNAME qd-channel
    SOURCE_FILES
      model/qd-channel-model.cc
      model/qd-channel-utils.cc
    HEADER_FILES
      model/qd-channel-model.h
      model/qd-channel-utils.h
    LIBRARIES_TO_LINK
      ${libcore}
      ${libspectrum}
    TEST_SOURCES test/qd-channel-test-suite.cc
                 ${examples_as_tests_sources}
)

