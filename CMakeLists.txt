cmake_minimum_required(VERSION 3.12)

set(CPPM_VERSION 0.0.12)
include(cmake/cppm_loader.cmake)
cppm_project()

project(zmqpp VERSION 4.2.0 LANGUAGES C CXX)
cppm_setting()
cppm_cxx_standard(17)
cppm_compiler_option(DEFAULT)

find_cppkg(ZeroMQ 4.3.3  MODULE libzmq-static TYPE lib)


cppm_target_define(zmq STATIC
SOURCES
    src/zmqpp/loop.cpp
    src/zmqpp/poller.cpp
    src/zmqpp/zmqpp.hpp
    src/zmqpp/reactor.hpp
    src/zmqpp/inet.hpp
    src/zmqpp/frame.cpp
    src/zmqpp/proxy_steerable.cpp
    src/zmqpp/curve.cpp
    src/zmqpp/zap_request.cpp
    src/zmqpp/message.cpp
    src/zmqpp/auth.hpp
    src/zmqpp/socket.cpp
    src/zmqpp/context.cpp
    src/zmqpp/actor.cpp
    src/zmqpp/signal.hpp
    src/zmqpp/z85.hpp
    src/zmqpp/proxy.cpp
    src/zmqpp/socket.hpp
    src/zmqpp/socket_mechanisms.hpp
    src/zmqpp/context.hpp
    src/zmqpp/actor.hpp
    src/zmqpp/signal.cpp
    src/zmqpp/z85.cpp
    src/zmqpp/socket_types.hpp
    src/zmqpp/proxy.hpp
    src/zmqpp/context_options.hpp
    src/zmqpp/loop.hpp
    src/zmqpp/byte_ordering.hpp
    src/zmqpp/poller.hpp
    src/zmqpp/zmqpp.cpp
    src/zmqpp/reactor.cpp
    src/zmqpp/frame.hpp
    src/zmqpp/proxy_steerable.hpp
    src/zmqpp/zap_request.hpp
    src/zmqpp/socket_options.hpp
    src/zmqpp/curve.hpp
    src/zmqpp/message.hpp
    src/zmqpp/compatibility.hpp
    src/zmqpp/exception.hpp
    src/zmqpp/auth.cpp
)


set(global_deps PUBLIC ZeroMQ)
cppm_target_dependencies(zmq
   ${global_deps})


cppm_target_install(zmq)

