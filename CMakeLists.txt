if (COMPONENTS_IS_ENABLED_QT)
    COMPONENTS_CONFIGURE_COMPONENT(Thread
        Qt5::Core
        pthread
        boost_system
        boost_thread
        boost_filesystem
    )
# FS --> для процессов
else()
    COMPONENTS_CONFIGURE_COMPONENT(Thread
        pthread
        boost_system
        boost_thread
        boost_filesystem
    )
endif()

COMPONENTS_LINK_COMPONENT(Thread Logger)
COMPONENTS_LINK_COMPONENT(Thread ExtraClasses)
