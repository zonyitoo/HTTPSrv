env = Environment()
env["CXX"] = "clang++"

SConscript(['src/SConscript'])

env.Program('test', ['test.cpp'], 
    LIBS=['srv', 'c++abi', 'c++', 'pthread'], 
    LIBPATH='src',
    CCFLAGS='-std=c++11 -g -stdlib=libc++',
    CPPPATH=['src'])

#Program('test', ['test.cpp'], 
#    LIBS=['srv'], 
#    LIBPATH='src',
#    CCFLAGS='-std=c++11 -g',
#    CPPPATH=['src'])

env.Program('test_client', ['test_client.cpp'],
    LIBS=['c++abi', 'c++', 'pthread'],
    CPPFLAGS='-std=c++11 -g -stdlib=libc++')
