project(
  'chess-game',
  'cpp',
  'c',
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
  cpp_args: '-DCHESS_SOURCE_DIR="@0@"'.format(meson.project_source_root()),
  include_directories: core_includes,
  link_with: chess_core,
  install: false,
)

test(
  'chess_tests',
  chess_tests,
  protocol: 'exitcode',
)

session_includes = include_directories(
  'gui/inc',
)

chess_session = static_library(
  'chess_session',
  'gui/src/ChessSession.cpp',
  include_directories: [
    core_includes,
    session_includes,
  ],
  link_with: chess_core,
  install: false,
)

session_tests = executable(
  'chess_session_tests',
  'gui/test/session_test.cpp',
  include_directories: [
    core_includes,
    session_includes,
  ],
  link_with: [
    chess_session,
    chess_core,
  ],
  install: false,
)

test(
  'chess_session_tests',
  session_tests,
  protocol: 'exitcode',
)

gui_option = get_option('gui')
gtkmm = dependency(
  'gtkmm-4.0',
  version: '>=4.10',
  required: gui_option,
)

if gtkmm.found()
  gnome = import('gnome')
  chess_resources = gnome.compile_resources(
    'chess-resources',
    'gui/resources/chess.gresource.xml',
    source_dir: 'gui/resources',
    c_name: 'chess_resources',
  )

  executable(
    'chess-game',
    [
      'gui/src/ChessBoardView.cpp',
      'gui/src/ChessWindow.cpp',
      'gui/src/main.cpp',
      chess_resources,
    ],
    dependencies: gtkmm,
    # glib-compile-resources emits one standards-limit-exceeding C string for
    # the embedded PNG bundle. Keep strict warnings for all handwritten C++.
    c_args: '-Wno-overlength-strings',
    include_directories: [
      core_includes,
      session_includes,
    ],
    link_with: [
      chess_session,
      chess_core,
    ],
    install: false,
  )
endif

summary(
  {
    'Core tests': true,
    'GTKmm GUI': gtkmm.found(),
    'GTKmm version': gtkmm.found() ? gtkmm.version() : 'not found',
  },
  section: 'Build targets',
)
