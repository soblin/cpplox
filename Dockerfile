# syntax=docker/dockerfile:1
FROM ubuntu:24.04

ENV CC=clang
ENV CXX=clang++

ARG CLANG_VERSION=18
ARG LIB_STDCPP_VERSION=13

RUN apt-get update && apt-get install -y git curl wget lsb-release software-properties-common

RUN apt-get update && apt-get install -y \
    clang-${CLANG_VERSION} \
    clang++-${CLANG_VERSION} \
    clang-tidy-${CLANG_VERSION} \
    clangd-${CLANG_VERSION} \
    clang-format-${CLANG_VERSION} \
    libomp-${CLANG_VERSION}-dev \
    libstdc++-${LIB_STDCPP_VERSION}-dev

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} ${CLANG_VERSION}
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${CLANG_VERSION} ${CLANG_VERSION}
RUN update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-${CLANG_VERSION} ${CLANG_VERSION}
RUN update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-${CLANG_VERSION} ${CLANG_VERSION}
RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-${CLANG_VERSION} ${CLANG_VERSION}

RUN apt-get update && apt-get install -y cmake libreadline-dev libgtest-dev libboost-all-dev libmagicenum-dev lcov llvm
