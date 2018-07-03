import os

EnsurePythonVersion(2, 7)
EnsureSConsVersion(2, 5)
Decider("MD5-timestamp")

env = Environment()

env["CXX"] = os.environ.get("CXX") or env["CXX"]
for var in (
        "MSVC_VERSION", "TARGET_ARCH" # for MSVC scons toolkit
        ):
    if var in os.environ:
        env[var] = os.environ[var]

env["USE_MSVC"] = "msvc" in env["TOOLS"]

env.AppendUnique(
    CPPDEFINES=[
        ],
    CPPPATH=[
        "#",
        "#googletest/googletest",
        "#googletest/googletest/include",
        ],
    )

if env["USE_MSVC"]:
    env.AppendUnique(
        CCFLAGS=[
            "/std:c++17",
            "/Zm2000",
            "/wd4503", # "decorated name length exceeded, name was truncated"
            "/wd4800", # "forcing value to bool, performance warning"
            ],
        CXXFLAGS=["/EHa"],
        CPPDEFINES=[
            "WIN32",
            "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS",
            "_CRT_NONSTDC_NO_DEPRECATE",
            "_WINSOCK_DEPRECATED_NO_WARNINGS",
            "_ALLOW_KEYWORD_MACROS",
            "NOMINMAX",
            "WIN32_LEAN_AND_MEAN", # exclude rarely used MSW stuff
            "_WIN32_WINNT=0x0601", # Win 7

            # gtest can't be built with MSVC2017 in the default setting.
            "GTEST_HAS_TR1_TUPLE=0",
            "GTEST_HAS_STD_TUPLE=1",
            "GTEST_LANG_CXX11=1",
            ],
        LINKFLAGS=[
            "/ignore:4204", # missing debugging information for referencing module
            "/MACHINE:X64",
            ],
        )
    env_warnings = env.Clone()
    env_warnings.AppendUnique(
        CCFLAGS=[
            "/W4",
            ],
        )
else:
    env.AppendUnique(
        CCFLAGS=[
            "-fdiagnostics-color",
            ],
        CXXFLAGS=[
            "-std=c++14",
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
            #"-Werror",
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

