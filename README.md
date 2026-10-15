# Meteour Running

Jogo 3D de sobrevivência feito originalmente para a disciplina de Computação Gráfica da Universidade Federal de Ouro Preto (UFOP). Dirija pela arena e desvie de meteoros até a vida chegar a zero.

## Como jogar

| Tecla | Ação |
| --- | --- |
| W / S | Acelerar / dar ré |
| A / D | Virar |
| R | Reiniciar a partida |
| M | Ativar/desativar o som |
| Q ou Esc | Sair |

Cada colisão reduz 20 pontos de vida. Há um segundo de invulnerabilidade após o dano. A barra de vida, o tempo de sobrevivência e os impactos aparecem na tela. Ao perder toda a vida, pressione **R** para jogar novamente.

## Compilar

Instale CMake 3.16+, um compilador C++17, OpenGL/GLU, FreeGLUT, SDL2, SDL2_mixer e pkg-config. No Windows, instale essas bibliotecas pelo MSYS2/MinGW e use seu terminal MinGW.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/MeteourRunning
```

## Implementação

O projeto usa o pipeline clássico de OpenGL/FreeGLUT para fins didáticos. O carro vem de `LowPolyCars.obj`, lido por `ObjLoader` e compilado em uma display list para evitar reconstruir suas faces a cada quadro. Os meteoros usam uma textura procedural; o chão repete `assets/textura_piso.jpg`. A atualização usa tempo decorrido em segundos, e a colisão compara a esfera do meteoro com uma caixa orientada junto ao carro. A interface e os efeitos de impacto são desenhados após a cena 3D.

O som usa SDL2_mixer. Os créditos e as licenças dos efeitos estão em [`assets/audio/README.md`](assets/audio/README.md).

## Recursos e licença

`src/stb_image.h` é uma biblioteca de terceiros (stb_image, v2.30; consulte o cabeçalho para as condições).
