add_executable("ui-test"
    main.cpp
    )

target_link_libraries("ui-test" PUBLIC
    project::options
    manga-manager::ui
    )

install(TARGETS "ui-test" DESTINATION bin)
