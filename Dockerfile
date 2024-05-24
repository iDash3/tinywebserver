FROM gcc:latest

RUN apt-get update && apt-get install -y \
    libcmark-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

COPY server.c .
COPY Makefile .

RUN make

EXPOSE 8080

CMD ["./server"]

