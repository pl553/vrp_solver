project('vrp_solver', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++2a'])

tinyxml2 = dependency('tinyxml2')
threads = dependency('threads')
boost = dependency('boost')

exe = executable('vrp_solver', 'core/cvrp_loader.cpp', 'core/core_model.cpp', 'core/cvrp_solver_savings.cpp', 'core/cvrp_solver_tabu.cpp', 'core/cvrp_solution_verifier.cpp',
  'main.cpp',
  dependencies : [tinyxml2, threads, boost], native : true)

test('basic', exe)
