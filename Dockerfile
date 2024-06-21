FROM ubuntu:22.04

# ARG GO_VERSION=1.19.4
# ENV GO_VERSION=${GO_VERSION}
ARG DEBIAN_FRONTEND=noninteractive

# # Install linux dependencies
RUN apt-get update \
 && apt-get install -y libssl-dev software-properties-common git sqlite3 zip curl rsync sagemath wget git gcc

# Install TopologyUpdater dependencies 
RUN apt-get update \
 && apt-get install -y pkg-config libssl-dev libncursesw5 libtinfo-dev systemd libsystemd-dev libsodium-dev tmux git jq libtool bc gnupg aptitude libtool secure-delete iproute2 tcptraceroute sqlite3 bsdmainutils libusb-1.0-0-dev libudev-dev unzip llvm clang libnuma-dev libpq-dev build-essential libffi-dev libgmp-dev zlib1g-dev make g++ autoconf automake liblmdb-dev procps

RUN apt-get install -y libboost-all-dev

RUN apt-get install -y locales postgresql-client

RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && \
    dpkg-reconfigure --frontend=noninteractive locales && \
    update-locale LANG=en_US.UTF-8

ENV LANG en_US.UTF-8 

COPY ./requirements.txt requirements.txt
RUN pip install -r requirements.txt


