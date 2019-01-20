project(
    'unet',
    'cpp',
    default_options : ['cpp_std=c++14'],
    license : 'MIT',
    version : '0.0.1',
)

boost = dependency('boost', required : true)
gtest = dependency('gtest', required : false)
gmock = dependency('gmock', required : false)
benchmark = dependency('benchmark', required : false)
threads = dependency('threads')

incdirs = include_directories('include')

install_subdir('include/unet', install_dir : get_option('includedir'))

lib = library(
    'unet',
    [
        'src/detail/frame.cpp',
        'src/detail/queue.cpp',
        'src/detail/raw_socket.cpp',
        'src/detail/socket.cpp',
        'src/detail/socket_set.cpp',
        'src/dev/tap.cpp',
        'src/event.cpp',
        'src/exception.cpp',
        'src/raw_socket.cpp',
        'src/stack.cpp',
        'src/timer.cpp',
        'src/wire/ethernet.cpp',
        'src/wire/ipv4.cpp',
    ],
    dependencies : [boost],
    include_directories : incdirs,
    install : true,
)

if gtest.found() and gmock.found()
    test = executable(
        'unet-test',
        [
            'test/main.cpp',
            'test/detail/frame.cpp',
            'test/detail/list.cpp',
            'test/detail/queue.cpp',
            'test/detail/raw_socket.cpp',
            'test/detail/socket.cpp',
            'test/event.cpp',
            'test/stack.cpp',
            'test/timer.cpp',
            'test/wire/ethernet.cpp',
            'test/wire/ipv4.cpp',
            'test/wire/wire.cpp',
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
            'bench/detail/socket.cpp',
        ],
        dependencies : [benchmark, threads],
        include_directories : incdirs,
        link_with : lib,
    )

    benchmark('bench', bench)
endif

example_defs = {
    'arping' : 'examples/arping.cpp',
    'tap'    : 'examples/tap.cpp',
}

example_exes = []

foreach name, cpp : example_defs
    example_exes += executable(
        name,
        [cpp],
        include_directories : incdirs,
        link_with : lib,
    )
endforeach

run_target('smoke', command : ['scripts/smoke.py', '-v'], depends : example_exes)
run_target('format', command : 'scripts/clang-format.sh')
