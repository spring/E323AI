### Generic native Skirmish AI config
#
set(mySourceDirRel         "") # Common values are "" or "src"
set(additionalSources      "")
set(additionalCompileFlags "")
set(additionalLibraries    ${LegacyCpp_AIWRAPPER_TARGET} CUtils ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})

configure_native_skirmish_ai(mySourceDirRel additionalSources additionalCompileFlags additionalLibraries)
