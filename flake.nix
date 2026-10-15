{
  description = "Meteour Running - projeto de Computacao Grafica da UFOP";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forEachSystem = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in {
      packages = forEachSystem (pkgs: {
        default = pkgs.stdenv.mkDerivation {
          pname = "meteour-running";
          version = "1.0.0";
          src = pkgs.lib.cleanSourceWith {
            src = self;
            filter = path: type:
              let name = baseNameOf path;
              in !(builtins.elem name [ "build" ".git" ".vscode" ".agents" ".aws" ".codex" ]);
          };
          nativeBuildInputs = [ pkgs.cmake pkgs.pkg-config ];
          buildInputs = [ pkgs.freeglut pkgs.libGL pkgs.libGLU ];
          # OpenGL needs the host GPU driver at runtime.
          postFixup = ''
            patchelf --add-rpath /run/opengl-driver/lib $out/bin/MeteourRunning
          '';
          meta = {
            description = "Jogo 3D de sobrevivencia a meteoros em OpenGL";
            platforms = pkgs.lib.platforms.linux;
            mainProgram = "MeteourRunning";
          };
        };
      });
      apps = forEachSystem (pkgs: {
        default = {
          type = "app";
          program = "${self.packages.${pkgs.stdenv.hostPlatform.system}.default}/bin/MeteourRunning";
          meta.description = "Executa o Meteour Running";
        };
      });
      devShells = forEachSystem (pkgs: {
        default = pkgs.mkShell {
          packages = [ pkgs.cmake pkgs.pkg-config pkgs.gdb pkgs.freeglut pkgs.libGL pkgs.libGLU ];
        };
      });
    };
}
