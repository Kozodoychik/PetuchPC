import assembler
import sys


def main():
    with open(sys.argv[1], "r") as file:
        l = assembler.PetuchPCAsm(file.readlines())
        l.assemble_instructions()

if __name__ == "__main__":
    main()