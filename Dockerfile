# ==============================================================================
# Geant4 & ROOT Simulation Environment with Advanced IDEs (Neovim/Emacs/Nano)
# Base OS: Rocky Linux 9 (RHEL 9 Compatible, 100% Bug-for-Bug compatible)
# ==============================================================================
FROM rockylinux:9

# ==============================================================================
# 1. System Update & Repository Setup
# - epel-release: Extra Packages for Enterprise Linux (for htop, tmux, etc.)
# - crb (CodeReady Builder): Required for development packages (formerly PowerTools)
# ==============================================================================
RUN dnf update -y && \
    dnf install -y epel-release && \
    dnf config-manager --set-enabled crb

# ==============================================================================
# 2. Core Dependencies (C++17, CMake, GUI & Qt6 for Geant4)
# - --allowerasing: Prevents dependency conflicts (e.g., curl vs minimal-curl)
# - python3-dnf-plugin-versionlock: Prevents unexpected kernel/package updates
# - qt6, libX11, libglvnd: Required for Geant4 OpenGL and Qt GUI visualization
# ==============================================================================
RUN dnf install -y --allowerasing \
    gcc gcc-c++ cmake cmake-gui make git wget tar curl unzip \
    python3-dnf-plugin-versionlock \
    expat-devel xerces-c-devel \
    libX11-devel libXext-devel libXmu-devel libXi-devel \
    libglvnd-opengl libglvnd-glx libglvnd-devel mesa-dri-drivers \
    qt6-qtbase-devel qt6-qt3d-devel && \
    dnf clean all

# ==============================================================================
# 3. Download Geant4 Source Code (v11.2.1)
# ==============================================================================
WORKDIR /opt
RUN wget https://gitlab.cern.ch/geant4/geant4/-/archive/v11.2.1/geant4-v11.2.1.tar.gz && \
    tar -xzf geant4-v11.2.1.tar.gz && \
    rm geant4-v11.2.1.tar.gz

# ==============================================================================
# 4. Compile and Install Geant4
# - DGEANT4_INSTALL_DATA=ON: Automatically downloads physics datasets 
#   (Radioactive decay, Neutron cross-sections, etc.)
# - CMAKE_CXX_STANDARD=17: Strict C++17 match required for ROOT 6 compatibility
# - BUILD_MULTITHREADED=ON: Enables event-level multi-threading (G4MTRunManager)
# ==============================================================================
WORKDIR /opt/geant4-v11.2.1/build
RUN cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr/local/geant4 \
    -DGEANT4_INSTALL_DATA=ON \
    -DGEANT4_USE_OPENGL_X11=ON \
    -DGEANT4_USE_QT=ON \
    -DGEANT4_USE_QT_QT6=ON \
    -DGEANT4_BUILD_MULTITHREADED=ON \
    -DCMAKE_CXX_STANDARD=17 && \
    make -j$(nproc) && \
    make install

# 5. Clean up Geant4 build artifacts to reduce Docker image size
WORKDIR /opt
RUN rm -rf geant4-v11.2.1

# 6. Register Geant4 environment variables to bashrc
RUN echo "source /usr/local/geant4/bin/geant4.sh" >> /root/.bashrc

# ==============================================================================
# 7. CERN ROOT & Data Analysis Dependencies
# - pcre-devel, gsl-devel, fftw-devel: Core math/stat libraries for ROOT Minuit
# - xrootd: CERN's distributed file system client
# - R-devel: For statistical analysis integration
# ==============================================================================
RUN dnf install -y \
    gedit \
    python3-pip python3-devel python3-numpy \
    pcre-devel pcre2-devel \
    libXpm-devel libXft-devel libXext-devel \
    gsl-devel openssl-devel patch binutils \
    gcc-gfortran mesa-libGL-devel mesa-libGLU-devel glew-devel ftgl-devel \
    mariadb-devel fftw-devel cfitsio-devel graphviz-devel libuuid-devel \
    avahi-compat-libdns_sd-devel openldap-devel libxml2-devel readline-devel \
    xrootd-client-devel xrootd-libs-devel \
    R-devel R-Rcpp-devel R-RInside-devel && \
    dnf clean all

# ==============================================================================
# 8. HEP Python Ecosystem (Scikit-HEP)
# - uproot, awkward: Native python libraries to read/write ROOT files
# - boost-histogram, mplhep: High-performance binning and plotting
# ==============================================================================
RUN pip3 install --no-cache-dir \
    uproot awkward mplhep scikit-hep \
    vector particle boost-histogram \
    numpy pandas matplotlib scipy jupyterlab

# ==============================================================================
# 9. Download and Compile CERN ROOT (v6.30.04)
# - Minuit2: Advanced minimization library for spectra fitting
# - PyROOT=OFF: Disabled to prevent python environment conflicts (using uproot instead)
# ==============================================================================
WORKDIR /opt
RUN wget https://root.cern/download/root_v6.30.04.source.tar.gz && \
    tar -xzf root_v6.30.04.source.tar.gz && \
    rm root_v6.30.04.source.tar.gz

WORKDIR /opt/root-6.30.04/build
RUN cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr/local/root \
    -DCMAKE_CXX_STANDARD=17 \
    -Dgdml=ON \
    -Dmathmore=ON \
    -Dminuit2=ON \
    -Dpyroot=OFF && \
    make -j$(nproc) && \
    make install

# 10. Clean up ROOT build artifacts
WORKDIR /opt
RUN rm -rf root-6.30.04

# 11. Register ROOT environment variables and Terminal Aliases
RUN echo "source /usr/local/root/bin/thisroot.sh" >> /root/.bashrc && \
    echo "alias ls='ls --color=auto'" >> /root/.bashrc && \
    echo "alias ll='ls -l --color=auto'" >> /root/.bashrc && \
    echo "alias grep='grep --color=auto'" >> /root/.bashrc && \
    echo "export PS1='\[\e[1;32m\][\u@\h \W]\$\[\e[0m\] '" >> /root/.bashrc

# ==============================================================================
# 12. Workspace Utilities & IDE Setup (Neovim, Emacs, Nano)
# - nodejs, java, ripgrep, fd-find: Required for Neovim LSPs and Fuzzy finders
# - libstdc++-static, clang: Prevents linking ('linc') errors during Tree-sitter builds
# - emacs-nox: Headless Emacs (No X-window dependency)
# ==============================================================================
RUN dnf module enable -y nodejs:20 && \
    dnf install -y --allowerasing nodejs java-17-openjdk-devel ripgrep fd-find \
    vim-enhanced tmux fzf bat htop tree jq libstdc++-static clang \
    emacs-nox nano && \
    dnf clean all

# Install Rust & Cargo (Required for building Tree-sitter CLI)
# -4 flag forces IPv4 to prevent GitHub API IPv6 routing timeouts in Docker
RUN curl -4 --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# Force C/C++ compiler for Tree-sitter to avoid linking failures
ENV CC=gcc
ENV CXX=g++

# Install tree-sitter-cli globally via Cargo
RUN cargo install tree-sitter-cli

# Download latest Neovim Nightly (v0.12+) for LazyVim compatibility
RUN curl -LO https://github.com/neovim/neovim/releases/download/nightly/nvim-linux-x86_64.tar.gz && \
    tar -C /opt -xzf nvim-linux-x86_64.tar.gz && \
    ln -sf /opt/nvim-linux-x86_64/bin/nvim /usr/local/bin/nvim && \
    rm -f nvim-linux-x86_64.tar.gz

# Clone LazyVim Starter Template
RUN git clone https://github.com/LazyVim/starter /root/.config/nvim && \
    rm -rf /root/.config/nvim/.git

# Pre-configure Tree-sitter to automatically install specific language parsers
# (C, C++, Python, Java, JSON, Fortran, CMake, Bash)
RUN mkdir -p /root/.config/nvim/lua/plugins && \
    echo 'return { { "nvim-treesitter/nvim-treesitter", opts = function(_, opts) if type(opts.ensure_installed) == "table" then vim.list_extend(opts.ensure_installed, { "c", "cpp", "python", "java", "json", "fortran", "cmake", "bash" }) end end } }' > /root/.config/nvim/lua/plugins/treesitter.lua

# Run Neovim headlessly to install Lazy.nvim plugins and parsers during docker build
RUN nvim --headless "+Lazy! sync" +qa

# ==============================================================================
# 13. Nano and Emacs Optimization
# ==============================================================================
# Nano: Apply syntax highlighting and sane coding defaults
RUN git clone https://github.com/scopatz/nanorc.git /root/.nano && \
    cat /root/.nano/nanorc > /root/.nanorc && \
    echo "set linenumbers" >> /root/.nanorc && \
    echo "set tabsize 4" >> /root/.nanorc && \
    echo "set autoindent" >> /root/.nanorc

# Emacs: Initialize MELPA package manager and disable startup splash screen
RUN echo "(require 'package)" >> /root/.emacs && \
    echo "(add-to-list 'package-archives '(\"melpa\" . \"https://melpa.org/packages/\") t)" >> /root/.emacs && \
    echo "(package-initialize)" >> /root/.emacs && \
    echo "(custom-set-variables '(inhibit-startup-screen t))" >> /root/.emacs

# ==============================================================================
# 14. Final Initialization
# ==============================================================================
WORKDIR /work
CMD ["/bin/bash"]
