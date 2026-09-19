{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = [
    pkgs.bc
    pkgs.gnumake
    pkgs.qemu
    pkgs.pkgsCross.riscv64-embedded.buildPackages.gcc
    pkgs.pkgsCross.riscv64-embedded.buildPackages.gdb
  ];

  # xv6 looks for a compiler prefix. On NixOS, the binary is `riscv64-none-elf-gcc`
  shellHook = ''
    export TOOLPREFIX=riscv64-none-elf-
    echo "========================================"
    echo "  xv6 RISC-V Environment Ready!        "
    echo "  Compiler: $(riscv64-none-elf-gcc --version | head -n1)"
    echo "  QEMU:     $(qemu-system-riscv64 --version | head -n1)"
    echo "========================================"
  '';
}

