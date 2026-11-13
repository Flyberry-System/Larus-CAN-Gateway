FROM ubuntu:22.04

LABEL maintainer="Larus Project"
LABEL description="CAN to NMEA Bridge for Maritime Applications"

# Update and install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    can-utils \
    git \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy source files
COPY Larus-bridge.cpp .
COPY simulator.cpp .
COPY CMakeLists.txt .
COPY nmea_config.json .

# Build
RUN mkdir build && cd build && \
    cmake .. && \
    cmake --build . && \
    cd .. && \
    mv build/larus-bridge . && \
    mv build/can-simulator .

# Create logging directory
RUN mkdir -p /var/log/larus

# Expose virtual CAN interface if needed
# VOLUME ["/dev/can0"]

# Default entry point
ENTRYPOINT ["./larus-bridge"]
CMD ["can0"]

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD test -f /var/log/larus/nmea_bridge.log || exit 1
