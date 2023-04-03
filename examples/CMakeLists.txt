build_lib_example(
    NAME qd-channel-model-example
    SOURCE_FILES qd-channel-model-example.cc
    LIBRARIES_TO_LINK
      ${libqd-channel}
      ${libspectrum}
      ${liblte}
)

