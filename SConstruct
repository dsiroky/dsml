env = Environment()

env["CXX"] = "g++-5"

env.AppendUnique(
        CPPDEFINES=[
            ],
        CPPPATH=[
            "#",
            "#googletest/googletest",
            "#googletest/googletest/include",
            ],
        CCFLAGS=[
            "-pedantic",
            "-fdiagnostics-color",
            "-Wall",
            "-Wextra",
            "-Wconversion",
            "-Wmissing-braces",
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

env.Program("run", Glob("*.cpp") + ["googletest/googletest/src/gtest-all.cc"])