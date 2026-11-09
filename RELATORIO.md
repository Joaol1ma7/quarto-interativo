# Relatório técnico - Quarto Interativo

## 1. Introdução

O Quarto Interativo é uma aplicação 3D desenvolvida em C++ com OpenGL. O objetivo foi criar um ambiente que o usuário pudesse explorar com teclado e mouse, além de permitir interações com objetos da cena. Este relatório apresenta as principais técnicas utilizadas no desenvolvimento do projeto.

## 2. Modelagem e transformações

O quarto e o cenário externo foram construídos diretamente no código, sem o uso de motores gráficos ou modelos 3D importados. Para isso, utilizamos formas geométricas como cubos, esferas, cilindros e troncos de cone. Foram modelados mais de oito objetos, entre eles cama, guarda-roupas, mesa, cadeira, computador, monitor, teclado, porta, janela, luminária, quadro e bola.

Para montar e posicionar os objetos, utilizamos translação, rotação e escala. Essas transformações também permitem movimentar partes da cena, como a porta e a janela, que giram ao abrir e fechar, e a bola, que se desloca e gira ao rolar.

## 3. Navegação e interações

O usuário explora o quarto com uma câmera em primeira pessoa, usando as teclas **W/A/S/D** para se movimentar e o **mouse** para olhar ao redor. Também foram implementadas colisões para impedir que a câmera atravesse paredes e móveis.

Para identificar o objeto apontado pelo usuário, utilizamos a técnica de seleção por raios (*ray casting*). Com a tecla **E**, é possível abrir e fechar a porta, abrir e fechar a janela e ligar ou desligar a luminária, inclusive pelo interruptor. A bola também reage à aproximação do usuário, rolando, colidindo com obstáculos e perdendo velocidade aos poucos.

## 4. Iluminação e materiais

A iluminação foi desenvolvida com *shaders* em GLSL, combinando luz ambiente, difusa e especular. As teclas **1, 2 e 3** permitem alternar entre manhã, tarde e noite, mudando a iluminação e a aparência do céu. A luminária funciona como uma fonte de luz artificial dentro do quarto.

As sombras foram criadas com a técnica de *shadow mapping*, usando dois mapas de profundidade: um para a luz natural e outro para a luminária. Elas são atualizadas conforme os objetos se movimentam. Também utilizamos materiais com diferentes cores, brilhos e transparências. No vidro, o reflexo varia de acordo com o ângulo de visão.

## 5. Mapeamento de texturas

As texturas foram aplicadas às superfícies dos objetos por meio de coordenadas **UV**. As texturas do piso, da madeira, da bola e da tela do monitor são geradas no próprio código, enquanto a imagem do quadro é carregada do arquivo `assets/quadro.jpg`.

## 6. Considerações finais

O projeto reúne modelagem, transformações, navegação, interações, iluminação, sombras e texturas em uma única cena. Com esses recursos, foi possível criar um quarto explorável e interativo, atendendo à proposta da atividade sem utilizar um motor gráfico.
