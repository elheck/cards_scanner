# Dev Container for Card Scanner
# Based on Ubuntu 24.04 with C++ development tools

FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    # Build tools
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    # Clang tools for linting
    clang-tidy \
    clang-format \
    # Python for Conan
    python3 \
    python3-pip \
    python3-venv \
    # OpenCV dependencies (for Conan to build OpenCV)
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libv4l-dev \
    libxvidcore-dev \
    libx264-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libatlas-base-dev \
    gfortran \
    # Tesseract dependencies
    libtesseract-dev \
    libleptonica-dev \
    tesseract-ocr \
    tesseract-ocr-eng \
    # Additional utilities
    git \
    curl \
    wget \
    unzip \
    # pipx for installing Python applications
    pipx \
    && rm -rf /var/lib/apt/lists/*

# Install Conan 2.x using pipx (recommended for PEP 668 compliance)
RUN pipx install conan && pipx ensurepath

# Make sure pipx binaries are in PATH for all users
ENV PATH="/root/.local/bin:$PATH"

# Switch to vscode user for Conan profile setup
USER vscode

# Ensure vscode user also has pipx path
ENV PATH="/home/vscode/.local/bin:$PATH"

# Install Conan for vscode user as well
RUN pipx install conan && pipx ensurepath

# Configure Conan profile
RUN conan profile detect --force

# Set up Conan remotes
RUN conan remote list

# Back to root for any final setup
USER root

# Set working directory
WORKDIR /workspace

# Reset to non-interactive
ENV DEBIAN_FRONTEND=dialog
