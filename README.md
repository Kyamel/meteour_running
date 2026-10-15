# Meteour Running

Jogo 3D de sobrevivência feito originalmente para a disciplina de Computação Gráfica da Universidade Federal de Ouro Preto (UFOP), por Lucas dos Anjos Camelo. Dirija pela arena e desvie de meteoros até a vida chegar a zero.

## Como jogar

| Tecla | Ação |
| --- | --- |
| W / S | Acelerar / dar ré |
| A / D | Virar |
| R | Reiniciar a partida |
| Q ou Esc | Sair |

Cada colisão reduz 20 pontos de vida. Há um segundo de invulnerabilidade após o dano. A barra de vida, o tempo de sobrevivência e os impactos aparecem na tela. Ao perder toda a vida, pressione **R** para jogar novamente.

## Executar com Nix (Linux)

É necessário um ambiente gráfico com suporte a OpenGL e Flakes habilitados no Nix.

```sh
nix run .
```

Para desenvolver:

```sh
nix develop
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/MeteourRunning
```

## Compilar sem Nix

Instale CMake 3.16+, um compilador C++17, OpenGL/GLU e FreeGLUT. No Windows, instale essas bibliotecas pelo MSYS2/MinGW e use seu terminal MinGW.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/MeteourRunning
```

No Windows, o executável pode estar em `build/Release/MeteourRunning.exe`. O CMake copia os recursos para a pasta do executável.

## Implementação

O projeto usa o pipeline clássico de OpenGL/FreeGLUT para fins didáticos. O carro vem de `LowPolyCars.obj`, lido por `ObjLoader` e compilado em uma display list para evitar reconstruir suas faces a cada quadro. Os meteoros usam uma textura procedural; o chão repete `assets/textura_piso.jpg`. A atualização usa tempo decorrido em segundos, e a colisão compara a esfera do meteoro com uma caixa orientada junto ao carro. A interface e os efeitos de impacto são desenhados após a cena 3D.

## Recursos e licença

`src/stb_image.h` é uma biblioteca de terceiros (stb_image, v2.30; consulte o cabeçalho para as condições). A origem e as permissões de redistribuição de `LowPolyCars.obj` e das imagens em `assets/` ainda precisam ser confirmadas. **Nenhuma licença para o projeto ou esses recursos está declarada neste repositório.** Confirme a autoria/licença dos recursos antes de tornar o histórico público ou distribuir o jogo.

O arquivo `bonde.obj`, `assets/grama.bmp`, `assets/Car Texture 2.png` e arquivos antigos de `build/` pertencem ao histórico do projeto e não são usados na versão atual.
