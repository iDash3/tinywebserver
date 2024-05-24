FROM gcc:latest

RUN apt-get update && apt-get install -y \
    libcmark-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY server.c .
COPY Makefile .
COPY README.md .

RUN make

EXPOSE 8000

CMD ["./server"]

