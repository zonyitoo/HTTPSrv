env = Environment()
env["CXX"] = "clang++"

SConscript(['src/SConscript'])

env.Program('test', ['test.cpp'], 
    LIBS=['srv', 'c++abi', 'c++'], 
    LIBPATH='src',
    CCFLAGS='-std=c++11 -g -stdlib=libc++',
    CPPPATH=['src'])

#Program('test', ['test.cpp'], 
#    LIBS=['srv'], 
#    LIBPATH='src',
#    CCFLAGS='-std=c++11 -g',
#    CPPPATH=['src'])
