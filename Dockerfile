FROM archlinux:latest
WORKDIR /app
COPY . .
RUN pacman -Syyu base-devel --noconfirm
RUN pacman -Syyu arm-none-eabi-gcc --noconfirm
RUN pacman -Syyu arm-none-eabi-newlib --noconfirm
RUN pacman -Syyu git --noconfirm

RUN git submodule update --init --recursive
RUN make && cp firmware* compiled-firmware/
