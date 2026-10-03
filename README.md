O que o arquivo faz
Compila src/sala.cpp como uma biblioteca usada pelo terminal (sistema_salas) e pela GUI (SistemaAlocacaoGUI).
A GUI é opcional. Se o Qt 6 não for encontrado, mostra um aviso e compila só o terminal. Para desligar de propósito, use -DBUILD_GUI=OFF.
Cria dois atalhos de execução, run e run_gui, que rodam a partir da raiz do projeto. Isso importa porque o terminal lê salas.csv e reservas.csv da pasta atual.
1. Instalar dependências

Windows (recomendado: MSYS2): baixe em msys2.org e abra o terminal MSYS2 UCRT64:

bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-qt6-base

Ubuntu/Debian (comando testado):

bash
sudo apt install build-essential cmake qt6-base-dev

macOS (com Homebrew):

bash
brew install cmake qt

Se só quiser o terminal, basta o compilador e o CMake. O Qt é só para a GUI.

2. Compilar

Na pasta do projeto (onde está o CMakeLists.txt):

bash
cmake -S . -B build
cmake --build build

No Windows com MSYS2, use cmake -S . -B build -G Ninja na primeira linha. No macOS, se o Qt não for achado, use cmake -S . -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt).

3. Rodar
bash
cmake --build build --target run        # versão de terminal
cmake --build build --target run_gui    # interface gráfica (se o Qt foi encontrado)

Também dá para rodar o executável direto, mas aí você precisa estar na raiz do projeto, senão ele não acha os CSVs:

bash
./build/sistema_salas        # no Windows: build\sistema_salas.exe