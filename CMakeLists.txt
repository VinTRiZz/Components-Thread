COMPONENTS_CONFIGURE_COMPONENT(Thread "1.0.0"
    boost_filesystem
    boost_system
    boost_thread
    pthread
)
COMPONENTS_CCR_CONAN_IS_CONFIGURED(COMPONENTS_CONAN_CONFIGURED)
if (COMPONENTS_CONAN_CONFIGURED)
    COMPONENTS_CCR_CONAN_REQUIRE(Boost.Filesystem)
    COMPONENTS_CCR_CONAN_REQUIRE(Boost.System)
    COMPONENTS_CCR_CONAN_REQUIRE(Boost.Thread)
    COMPONENTS_CCR_CONAN_REQUIRE(PThread)
endif()
COMPONENTS_LINK_COMPONENT(Components-Thread Core)
COMPONENTS_LINK_COMPONENT(Components-Thread ExtraClasses)
COMPONENTS_LINK_COMPONENT(Components-Thread Logger)
if (COMPONENTS_IS_ENABLED_QT)
    target_link_libraries(Components-Thread PRIVATE Qt5::Core)
endif()
