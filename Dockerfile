# Используем базовый образ с Qt6
FROM ubuntu:22.04 AS builder

# Устанавливаем переменные окружения
ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем системные зависимости и инструменты сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libsqlite3-dev \
    sqlite3 \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-base-dev-tools \
    libqt6sql6-sqlite \
    && rm -rf /var/lib/apt/lists/*

# Создаем рабочую директорию
WORKDIR /app

# Копируем исходный код
COPY . .

# Создаем директорию для сборки и компилируем
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# ==============================
# Финальный образ
# ==============================
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_DEBUG_PLUGINS=0

# Устанавливаем только runtime зависимости Qt6
RUN apt-get update && apt-get install -y \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6sql6-sqlite \
    libqt6network6 \
    libqt6printsupport6 \
    libsqlite3-0 \
    sqlite3 \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxkbcommon-x11-0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xfixes0 \
    libxcb-xinerama0 \
    libxcb-xinput0 \
    libxcb-xkb1 \
    xvfb \
    x11-utils \
    && rm -rf /var/lib/apt/lists/*

# Создаем пользователя без привилегий для безопасности
RUN useradd -m -u 1000 appuser

WORKDIR /app

# Копируем собранное приложение из стадии сборки
COPY --from=builder --chown=appuser:appuser /app/build/src/LedgerMini /app/ledgermini

# Создаем необходимые директории для данных
RUN mkdir -p /app/data/db && chown -R appuser:appuser /app/data

# Переключаемся на непривилегированного пользователя
USER appuser

# Создаем скрипт запуска
RUN echo '#!/bin/bash\n\
echo "=== LedgerMini ==="\n\
echo "Запуск с DISPLAY=$DISPLAY"\n\
if [ -n "$DISPLAY" ]; then\n\
    echo "Запуск с GUI..."\n\
    /app/ledgermini\n\
else\n\
    echo "DISPLAY не установлен. Запуск с xvfb (без GUI)..."\n\
    xvfb-run -a /app/ledgermini\n\
fi' > /app/entrypoint.sh && \
    chmod +x /app/entrypoint.sh

# Указываем точку входа
ENTRYPOINT ["/app/entrypoint.sh"]