#!/usr/bin/env bash
set -e

# Colores para la salida del script
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

MODE="debug"
DO_CLEAN=0
DO_INSTALL=0

show_help() {
    echo "Uso: ./build.sh [OPCIONES]"
    echo ""
    echo "Opciones disponibles:"
    echo "  --debug     Compila con trazas detalladas en consola (por defecto)"
    echo "  --release   Compila optimizado sin trazas de depuración"
    echo "  --clean     Elimina compilaciones previas antes de compilar"
    echo "  --install   Instala la aplicación en /usr/local/bin (requiere root)"
    echo "  --help      Muestra esta ayuda"
    echo ""
    echo "Ejemplos:"
    echo "  ./build.sh --debug"
    echo "  ./build.sh --release"
    echo "  ./build.sh --clean --release --install"
}

# Parseo de parámetros
for arg in "$@"; do
    case "$arg" in
        --debug)
            MODE="debug"
            ;;
        --release)
            MODE="release"
            ;;
        --clean)
            DO_CLEAN=1
            ;;
        --install)
            DO_INSTALL=1
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}[ERROR] Parámetro desconocido: $arg${NC}"
            show_help
            exit 1
            ;;
    esac
done

# Detectar qmake de Qt5 en Slackware 15
QMAKE_BIN=""
if command -v qmake-qt5 >/dev/null 2>&1; then
    QMAKE_BIN="qmake-qt5"
elif command -v qmake >/dev/null 2>&1; then
    QMAKE_BIN="qmake"
else
    echo -e "${RED}[ERROR] No se encontró qmake ni qmake-qt5 en el sistema.${NC}"
    exit 1
fi

echo -e "${CYAN}=== Slackware Wizard (slackwiz) - Build System ===${NC}"
echo -e "${CYAN}[INFO] Modo seleccionado:${NC} ${MODE^^}"
echo -e "${CYAN}[INFO] Usando qmake:${NC} $QMAKE_BIN"

# Limpieza si se solicitó --clean
if [ "$DO_CLEAN" -eq 1 ]; then
    echo -e "${YELLOW}[CLEAN] Limpiando archivos temporales y compilaciones previas...${NC}"
    if [ -f Makefile ]; then
        make distclean >/dev/null 2>&1 || true
    fi
    rm -rf build Makefile .qmake.stash
fi

# Generar Makefile con qmake
echo -e "${CYAN}[QMAKE] Generando Makefile en modo $MODE...${NC}"
$QMAKE_BIN slackwiz.pro "CONFIG+=$MODE"

# Compilar utilizando todos los núcleos disponibles
CORES=$(nproc 2>/dev/null || echo 1)
echo -e "${CYAN}[MAKE] Compilando con $CORES hilos...${NC}"
make -j"$CORES"

echo -e "${GREEN}[OK] Compilación finalizada correctamente: ./build/slackwiz${NC}"

# Instalación o ejecución directa
if [ "$DO_INSTALL" -eq 1 ]; then
    if [ "$EUID" -ne 0 ]; then
        echo -e "${RED}[ERROR] La opción --install requiere privilegios de root (ejecuta con su -c o sudo).${NC}"
        exit 1
    fi
    echo -e "${CYAN}[INSTALL] Instalando slackwiz en el sistema...${NC}"
    make install
    echo -e "${GREEN}[OK] Instalación completada en /usr/local/bin/slackwiz${NC}"
else
    echo -e "${CYAN}[RUN] Ejecutando ./build/slackwiz...${NC}"
    echo "------------------------------------------------------------"
    ./build/slackwiz
fi
