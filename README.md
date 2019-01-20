# README

*unet* is a user space TCP/IP stack I'm writing to learn about networking. It is inspired by [picotcp](https://github.com/tass-belgium/picotcp) and [smoltcp](https://github.com/m-labs/smoltcp), two awesome well documented network stacks.

## Dependencies

- [boost](https://www.boost.org/) - Required
- [gflags](https://github.com/gflags/gflags) - Optional
- [googletest](https://github.com/google/googletest) - Optional
- [benchmark](https://github.com/google/benchmark) - Optional

## Building

You will need [Meson](http://mesonbuild.com/) and [Ninja](https://ninja-build.org/) to build. The basic steps are:

```
meson build && cd build && ninja
```

The following special targets are provided:

- **install:** Installs the library and headers on your system
- **test:** Runs unit tests, don't forget to `meson configure -Db_sanitize=address`
- **benchmark:** Runs benchmarks, don't forget to `meson configure -Dbuildtype=release`
- **smoke:** Runs [smoke](scripts/smoke.py) tests
- **format:** Runs `clang-format` on the source

A dev VM w/a build environment is provided. Just `vagrant up && vagrant ssh`.

## Resources

- [Stanford's CS 144 MOOC](https://lagunita.stanford.edu/courses/Engineering/Networking-SP/SelfPaced/courseware)
- [TUN/TAP Interface Tutorial](http://backreference.org/2010/03/26/tuntap-interface-tutorial/)
