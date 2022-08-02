# Copyright 2022 PingCAP, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
FROM ubuntu:jammy-20220531 as builder

USER root
WORKDIR /root/

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    apt update && apt install -y gnupg wget curl && \
    curl https://apt.llvm.org/llvm-snapshot.gpg.key|apt-key add && \
    apt install -y clang-13 lldb-13 lld-13 clang-tools-13 clang-13-doc \
    libclang-common-13-dev libclang-13-dev libclang1-13 clang-format-13 clangd-13 \
    clang-tidy-13 libc++-13-dev libc++abi-13-dev libomp-13-dev llvm-13-dev \
    libfuzzer-13-dev && apt install -y lcov cmake ninja-build libssl-dev zlib1g-dev \
    libcurl4-openssl-dev git && curl https://sh.rustup.rs -sSf | sh -s -- -y --profile \
    minimal --default-toolchain nightly

ENV HOME=/root \
    TZ=Asia/Shanghai \
    PATH="/opt/cmake/bin:/usr/local/bin/:${PATH}:/usr/local/go/bin:/root/.cargo/bin" \
    CC=clang-13 \
    CXX=clang++-13

ADD . /root/tiflash

RUN cd /root/tiflash && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-Wno-deprecated-declarations -stdlib=libc++' . && make tiflash -j16

FROM ubuntu:jammy-20220531
ENV HOME=/root \
    TZ=Asia/Shanghai
    LD_LIBRARY_PATH=/tiflash

COPY --from=builder /root/tiflash/dbms/src/Server/tiflash /tiflash/tiflash
COPY --from=builder /root/tiflash/contrib/tiflash-proxy/target/release/libtiflash_proxy.so /tiflash/
COPY --from=builder /lib/llvm-13/lib/libc++.so.1.0 /tiflash/libc++.so.1
COPY --from=builder /lib/llvm-13/lib/libc++abi.so.1.0 /tiflash/libc++abi.so.1
COPY --from=builder /lib/llvm-13/lib/libunwind.so.1.0 /tiflash/libunwind.so.1

RUN apt update && apt install -y curl mysql-client tzdata && apt-get clean && \
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && chmod +x /tiflash/*.so.1

ENTRYPOINT ["/tiflash/tiflash", "server"]
