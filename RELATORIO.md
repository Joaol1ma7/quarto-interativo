# Relatório — quarto interativo

## Modelagem

O quarto e seus móveis foram construídos diretamente no código com primitivas geométricas, como cubos, esferas, cilindros e troncos de cone. A composição dessas peças forma cama, guarda-roupas, mesa, cadeira, computador, porta, janela, luminária, quadro e bola. A paisagem vista pelas aberturas também é composta por geometria gerada no projeto.

## Transformação

Cada peça recebe matrizes de translação, rotação e escala, combinadas com a transformação do objeto a que pertence. Isso permite reutilizar as mesmas malhas em partes de tamanhos e posições diferentes. Porta e janela giram em torno das dobradiças; a bola combina deslocamento com uma rotação acumulada conforme a distância percorrida.

## Interação com os objetos da cena

A mira identifica o objeto visível mais próximo dentro do alcance de interação, e a tecla E abre ou fecha a porta e a janela, ou alterna a luz pela luminária e pelo interruptor.

A bola recebe um impulso quando a câmera se aproxima. Ela rola, perde velocidade gradualmente e rebate nos obstáculos.

## Iluminação
A iluminação combina componentes ambiente, difusa e especular. As teclas 1, 2 e 3 selecionam manhã, tarde e noite, alterando a cor e a intensidade da luz natural e do céu. A luminária fornece luz artificial em um cone voltado para baixo.

Dois mapas de profundidade produzem sombras para a luz natural e para a luminária. As transformações atuais dos objetos móveis são consideradas nesses mapas. A luz natural atravessa os vãos da janela e da porta, enquanto paredes e móveis opacos bloqueiam sua incidência direta. O vidro usa transparência e uma aproximação de Fresnel para destacar as bordas vistas em ângulo.

## Mapeamento de textura

Coordenadas UV associam as superfícies às imagens usadas como texturas OpenGL. As texturas do piso, da madeira, da bola e da tela do monitor são geradas por código; a fotografia do quadro é carregada de `assets/quadro.jpg`.
