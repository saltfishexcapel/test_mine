test_mine_sources = [
  'main.c',
  'test_mine-window.c',
  'test_mine-application.c',
  'test_mine-field.c',
  'object-hash.c',
  'mine_engine_v2.c'
]

test_mine_deps = [
  dependency('gtk4'),
]

gnome = import('gnome')

test_mine_sources += gnome.compile_resources('test_mine-resources',
  'test_mine.gresource.xml',
  c_name: 'test_mine'
)

executable('test_mine', test_mine_sources,
  dependencies: test_mine_deps,
  install: true,
)
