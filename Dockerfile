FROM ubuntu:16.04
RUN apt-get update -y
RUN apt-get install -y --no-install-recommends wget ca-certificates
RUN wget -qO - http://www.dotdeb.org/dotdeb.gpg | apt-key add -
RUN apt-get install -y apt-utils
RUN apt-get install -y --no-install-recommends software-properties-common
RUN add-apt-repository -y ppa:allegro/5.2
RUN apt-get update
RUN apt-get install -y gcc
RUN apt-get install -y g++
RUN apt-get install -y gdb
RUN apt-get install -y libsdl2-dev
RUN apt-get install -y liballegro4-dev
RUN apt-get install -y --no-install-recommends vim
RUN apt-get install -y --no-install-recommends make
RUN apt-get install -y --no-install-recommends python
RUN apt-get install -y --no-install-recommends libncurses5
RUN apt-get install -y --no-install-recommends libncurses5-dev
RUN apt-get install -y --no-install-recommends libgtk2.0-0
RUN apt-get install -y --no-install-recommends libgtk2.0-common
RUN apt-get install -y --no-install-recommends libwxgtk3.0-dev
RUN apt-get install -y --no-install-recommends git
#RUN git clone https://github.com/virtualagc/virtualagc.git
RUN git clone https://github.com/astrosec/virtualagc.git
RUN cd virtualagc && make
#Entry point runs at docker container startup
ADD doit.sh /
RUN chmod 777 doit.sh
ENTRYPOINT ["/doit.sh"]
#CMD args are passed to ENTRYPOINT command and is overwritten when you pass your own args to docker
#CMD ["-c"]
EXPOSE 19698
