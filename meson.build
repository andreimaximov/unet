project(
    'unet',
    'cpp',
    default_options : ['cpp_std=c++14'],
    license : 'MIT',
    version : '0.0.1',
)

gtest = dependency('gtest', required : false)
gmock = dependency('gmock', required : false)
benchmark = dependency('benchmark', required : false)
threads = dependency('threads')

incdirs = include_directories('include')

install_subdir('include/unet', install_dir : get_option('includedir'))

lib = library(
    'unet',
    [
        'src/stack.cpp',
    ],
    include_directories : incdirs,
    install : true,
)

if gtest.found() and gmock.found()
    test = executable(
        'unet-test',
        [
            'test/main.cpp',
            'test/stack.cpp',
        ],
        dependencies : [gtest, gmock],
        include_directories : incdirs,
        link_with : lib,
    )

    test('test', test)
endif

if benchmark.found()
    bench = executable(
        'unet-bench',
        [
            'bench/main.cpp',
            'bench/stack.cpp',
        ],
        dependencies : [benchmark, threads],
        include_directories : incdirs,
        link_with : lib,
    )

    benchmark('bench', bench)
endif

run_target('format', command : 'scripts/clang-format.sh')
