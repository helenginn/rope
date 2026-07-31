{
  description = "Flake: helenginn/rope";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let

        pkgs = import nixpkgs { inherit system; };
        inherit (pkgs) lib;

        version = "dev";

        buildInputs = with pkgs; [
          boost
          fftwFloat
          glew
          glm
          SDL2
          SDL2_image
          nlohmann_json
          gemmi
          libpng
          libjpeg
          zlib
          openssl
          curl
        ];

        nativeBuildInputs = with pkgs; [
          meson
          ninja
          pkg-config
          python3
          cmake
        ];

        meta = with lib; {
          description = "conformational space of proteins + slow-burn vagabond rewrite";
          longDescription = ''
          conformational space of proteins + slow-burn vagabond rewrite
          '';
          homepage = "https://github.com/helenginn/rope";
          license = licenses.gpl3Plus;
          platforms = platforms.linux ++ platforms.darwin;
          mainProgram = "rope.gui";
        };

        rope = pkgs.stdenv.mkDerivation {
          pname = "rope";
          inherit version meta nativeBuildInputs buildInputs;
          src = lib.fileset.toSource {
            root = ./.;
            fileset = lib.fileset.unions [
              ./vagabond
              ./meson.build
              ./meson_options.txt
              ./config
              ./assets
            ];
          };
          mesonFlags = [
            "-Dwarning_level=1"
            "-Denable_tests=false"
          ];
        };

        rope-debug = rope.overrideAttrs (_old: {
          pname = "rope-debug";
          mesonFlags = [
            "-Dwarning_level=1"
            "-Denable_tests=false"
          ];
          mesonBuildType = "debug";
          dontStrip = true;
          });

      in
      {
        packages = {
          default = rope;
          inherit rope rope-debug;
        };
  
        apps = {
          default = {
            type= "app";
            program = lib.getExe rope;
          };
          rope-cli = {
            type= "app";
            program = "${rope}/bin/rope";
          };
          rope-debug = {
            type= "app";
            program = lib.getExe rope-debug;
          };
          rope-cli-debug = {
            type= "app";
            program = "${rope-debug}/bin/rope";
          };
        };

        devShells.default = pkgs.mkShell {
          inherit nativeBuildInputs buildInputs;

          shellHook = ''
            echo ""
            echo " RoPE dev shell"
            echo ""
          '';
        };
      }
    );
}
