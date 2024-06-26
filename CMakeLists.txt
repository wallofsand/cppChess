cmake_minimum_required(VERSION 3.0.0)
project(cppChess VERSION 0.1.0)

include_directories(cppChess ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include/fmt)

set(DCMAKE_CXX_FLAGS -pg)
set(DCMAKE_EXE_LINKER_FLAGS -pg)
set(DCMAKE_SHARED_LINKER_FLAGS -pg)

add_executable(cppChess
    main.cpp Compass.cpp
    Bitboard.cpp Chess.cpp
    Move.cpp MoveGenerator.cpp
    Player.cpp SearchLogger.cpp
    PieceLocationTables.cpp
    TTable.cpp)

add_subdirectory(fmt EXCLUDE_FROM_ALL)
target_link_libraries(cppChess PRIVATE fmt::fmt)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
