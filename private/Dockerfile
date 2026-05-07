FROM debian

# Update & upgrade APT.

RUN apt-get update && apt-get upgrade -y

COPY ./ ./src

EXPOSE 8081/tcp
EXPOSE 8081/udp

