HTTP Server
-----------

A simple HTTP server in C++ 11.

Build
=====

* Install ``SCons`` and ``clang``.

* Write a ``SConstruct`` file to build the library

.. code:: python

    env = Environment()
    env["CXX"] = "clang++"

    SConscript(['src/SConscript'])

    env.Program('test', 'test.cpp',
        LIBS=['srv', 'c++abi', 'c++'],
        LIBPATH='src',
        CPPFLAGS='-std=c++11 -stdlib=libc++ -g',
        CPPPATH=['src'])

* Run ``scons``.

Note: GCC currently doesn't support C++11's ``regex``. We have to use ``clang`` and ``libc++``.

Usage
=====

.. code:: cpp

    #include "server.h"
    #include "ioloop.h"
    #include "handler.h"

    using namespace httpserver;

    class HelloWorldHandler : public HttpHandler {
        public:
            void get_handler(const HttpRequest& req, HttpResponse& resp, 
                const std::vector<std::string>& args) {
                
                resp.body = "Hello World!!";
                resp.headers["Content-Type"] = "text/plain";

            }
    };

    int main(int argc, char **argv) {
        
        EPollIOLoop ioloop;

        HttpServer server(argc, argc, ioloop);
        server.register_handler("/", new HelloWorldHandler());
        server.register_handler("/([^/]*)/", new HelloWorldHandler());

        return ioloop.start();
    }
