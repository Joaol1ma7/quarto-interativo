# Quarto interativo

## Compilar e executar

Para apenas jogar, baixe [a versão pronta para Windows](release/QuartoInterativo-Windows.zip), extraia **todo** o ZIP e abra `quarto.exe`. Não é preciso instalar compilador nem CMake. É necessário um computador Windows com placa de vídeo compatível com OpenGL 3.3 e driver atualizado.

Para compilar o código, instale:

- **Windows:** [download oficial do Windows 11](https://www.microsoft.com/pt-br/software-download/windows11).
- **Driver de vídeo:** escolha o da sua placa em [Intel](https://www.intel.com/content/www/us/en/download-center/home.html), [AMD](https://www.amd.com/en/support.html) ou [NVIDIA](https://www.nvidia.com/en-us/drivers/).
- **CMake 3.16 ou superior:** [download oficial](https://cmake.org/download/).
- **MinGW-w64:** [instale o MSYS2](https://www.msys2.org/) e, no terminal **MSYS2 MINGW64**, execute `pacman -S mingw-w64-x86_64-toolchain` ([pacote oficial](https://packages.msys2.org/groups/mingw-w64-x86_64-toolchain)).

No PowerShell, na raiz do projeto, execute:

```powershell
$env:Path = "C:\msys64\mingw64\bin;$env:Path"
cmake -S . -B build-msys2 -G "MinGW Makefiles" `
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe `
  -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe `
  -DBUILD_TESTING=OFF
cmake --build build-msys2 -j 1
.\build-msys2\quarto.exe
```
Execute \build-msys2\quarto.exe e divirta-se
Se o MSYS2 foi instalado fora de `C:\msys64`, ajuste esse caminho nos comandos. A pasta `build-msys2` é nova para não reutilizar a configuração de um compilador antigo. O CMake copia `assets/quadro.jpg` para a pasta do executável.

## Comandos

WASD movimentar-se;  
E interagir com objetos;  
1 2 3 alterar hora do dia;  
ESC fechar experiência.
