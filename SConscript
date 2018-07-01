import os

env = Environment()

env["CXX"] = os.environ.get("CXX") or env["CXX"]

env.AppendUnique(
    CPPDEFINES=[
        ],
    CPPPATH=[
        "#",
        "#googletest/googletest",
        "#googletest/googletest/include",
        ],
    CCFLAGS=[
        "-fdiagnostics-color",
        ],
    CXXFLAGS=[
        "-std=c++17",
        "-fno-rtti",
        ],
    LINKFLAGS=[
        "-fno-rtti",
        "-pthread",
        ],
    )

if "clang" in env["CXX"]:
    env.AppendUnique(
        CCFLAGS=[
            "-ferror-limit=10",
            ],
        CXXFLAGS=[
            #"-stdlib=libc++",
            ],
        LINKFLAGS=[
            #"-stdlib=libc++",
            ],
        )
else:
    env.AppendUnique(
        CCFLAGS=[
            "-fmax-errors=10",
            ],
        )

env_warnings = env.Clone()
env_warnings.AppendUnique(
    CCFLAGS=[
        "-pedantic",
        "-Wall",
        "-Wextra",
        "-Wconversion",
        "-Wdeprecated",
        "-Wfloat-conversion",
        "-Wfloat-equal",
        "-Wmissing-braces",
        "-Wstrict-aliasing",
        "-Wswitch",
        "-Wswitch-default",
        "-Wswitch-enum",
        "-Wuninitialized",
        "-Wunreachable-code",
        "-Wunused-function",
        "-Wunused-parameter",
        "-Wunused-result",
        "-Wunused-value",
        "-Wunused-variable",
        ],
    )

obj = env_warnings.Object(Glob("*.cpp"))

env.Program("test", obj + ["googletest/googletest/src/gtest-all.cc"])

