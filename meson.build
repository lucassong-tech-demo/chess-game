project(
  'chess-game',
  'cpp',
  version: '0.1.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
  ],
)

core_includes = include_directories(
  'model/inc',
  'utils/include',
)

core_sources = files(
  'model/src/Bishop.cpp',
  'model/src/BoardPosition.cpp',
  'model/src/ChessBoard.cpp',
  'model/src/GameFacade.cpp',
  'model/src/King.cpp',
  'model/src/Knight.cpp',
  'model/src/Pawn.cpp',
  'model/src/Piece.cpp',
  'model/src/PieceHistory.cpp',
  'model/src/Queen.cpp',
  'model/src/Rook.cpp',
  'model/src/Xml.cpp',
)

chess_core = static_library(
  'chess_core',
  core_sources,
  include_directories: core_includes,
  install: false,
)

chess_tests = executable(
  'chess_tests',
  'test/test.cpp',
  include_directories: core_includes,
  link_with: chess_core,
  install: false,
)

test(
  'chess_tests',
  chess_tests,
  protocol: 'exitcode',
)
