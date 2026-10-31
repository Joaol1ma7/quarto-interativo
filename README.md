# Quarto interativo

## Compilar e executar

Requisitos para compilar e executar:

- **WINDOWS 11**;
- **Driver com OpenGL 3.3:** Driver da placa de vídeo atualizado.
- **CMake 3.16 ou superior:** [baixar o instalador para Windows](https://cmake.org/download/).
- **MinGW-w64:** [instalar o MSYS2](https://www.msys2.org/) e, no terminal **MSYS2 MINGW64**, executar `pacman -S mingw-w64-x86_64-toolchain` ([pacote oficial](https://packages.msys2.org/groups/mingw-w64-x86_64-toolchain)).

Para usar os comandos abaixo no PowerShell, `cmake`, `g++` e `mingw32-make` precisam estar no `PATH`. Na instalação padrão do MSYS2, adicione `C:\msys64\mingw64\bin` ao `PATH`.

No PowerShell, a partir da raiz do projeto:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j 4
.\build\quarto.exe
```

O CMake copia `assets/quadro.jpg` para a pasta do executável.

## Comandos 

WASD movimentar-se;
E interagir com objetos;
1 2 3 Alterar hora do dia;
ESC fechar experiência.

