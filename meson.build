project('vrp_solver', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++2a'])

tinyxml2 = dependency('tinyxml2')

exe = executable('vrp_solver', 'core/cvrp_loader.cpp', 'core/cvrp_solver_savings.cpp', 'main.cpp',
  dependencies : [tinyxml2], native : true)

test('basic', exe)
