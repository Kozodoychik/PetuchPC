import assembler


def main():
    with open("demo/test.s", "r") as file:
        l = assembler.PetuchPCAsm(file.readlines())
        l.assemble_instructions()

if __name__ == "__main__":
    main()