# Quarto Interativo em OpenGL 3D

Aplicação desenvolvida em **C++17 e OpenGL 3.3**, com janela e entrada de dados pela API nativa do Windows (Win32/WGL), sem motores gráficos ou frameworks de jogos. O projeto apresenta um quarto 3D que pode ser explorado em primeira pessoa com teclado e mouse.

## Funcionalidades

- Modelagem própria de móveis e objetos, incluindo cama, guarda-roupas, mesa, cadeira, computador, porta, janela, luminária, quadro e bola.
- Abertura e fechamento de porta e janela, além de acionamento da luminária.
- Alternância entre manhã, tarde e noite, com alterações de iluminação e céu.
- Navegação com detecção de colisões e bola que se movimenta ao receber um impulso pela aproximação da câmera.
- Materiais distintos, sombras e mapeamento de texturas.

## Requisitos para compilar

- **Windows** com placa de vídeo e driver compatíveis com **OpenGL 3.3**.
- **CMake 3.16** ou superior.
- **MinGW-w64** via [MSYS2](https://www.msys2.org/).

No terminal **MSYS2 MINGW64**, instale o compilador:

```bash
pacman -S mingw-w64-x86_64-toolchain
```

## Compilação e execução

Abra o **PowerShell** na raiz do repositório e execute:

```powershell
$env:Path = "C:\msys64\mingw64\bin;$env:Path"
cmake -S . -B build-msys2 -G "MinGW Makefiles" `
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe `
  -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe `
  -DBUILD_TESTING=OFF
cmake --build build-msys2 -j 1
.\build-msys2\quarto.exe
```

Se o MSYS2 estiver instalado em outro diretório, ajuste os caminhos dos comandos. O CMake copia `assets/quadro.jpg` para a pasta do executável.

## Controles

| Comando | Ação |
|---|---|
| **W / A / S / D** | Movimentar a câmera |
| **Mouse** | Olhar ao redor |
| **E** | Interagir com o objeto apontado e ao alcance (porta, janela, luminária ou interruptor) |
| **1** | Selecionar manhã |
| **2** | Selecionar tarde |
| **3** | Selecionar noite |
| **Esc** | Fechar a aplicação |

**Observação:** aproxime-se de um objeto interativo e aponte a mira para ele antes de pressionar **E**. A bola reage à aproximação da câmera.

## Documentação

As técnicas de modelagem, transformações, navegação, interações, iluminação e texturas estão descritas no [Relatório](RELATORIO.md).
