FROM archlinux

ARG UID
ARG GROUPS
ARG USER

RUN pacman -Syu --noconfirm
# toolchain for compilation
RUN pacman -S --noconfirm gcc make pkg-config
# necessary libs
RUN pacman -S --noconfirm opencv fltk imlib2
# tools for actually running/debugging (FLTK will fail with "unable to find fonts" error if no fonts are installed
RUN pacman -S --noconfirm gdb xorg-fonts-75dpi

COPY docker-entrypoint.sh /docker-entrypoint.sh


RUN groupadd -g $GROUPS $USER
RUN useradd -u $UID $USER -g $GROUPS
USER $USER


WORKDIR comictranslator
ENTRYPOINT /docker-entrypoint.sh
