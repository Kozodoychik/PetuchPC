import re
import sys
from enum import Enum


class OperandTypes(Enum):
    REG = 0
    IMM = 1
    REG_PTR = 2
    IMM_PTR = 3
    LABEL = 4
    LABEL_PTR = 5

class TokenTypes(Enum):
    INSTRUCTION = 0
    LABEL = 1
    DIRECTIVE = 2
    
class PointerTypes(Enum):
    IMM = 0
    REG = 1
    
class Instruction:
    def __init__(self, opcode, type):
        self.opcode = opcode
        self.type = type

class InstructionTypes(Enum):
    TYPE0 = 0
    TYPE1 = 1
    TYPE2 = 2
    TYPE3 = 3
    TYPE4 = 4
    TYPE5 = 5

class Size(Enum):
    BYTE = 0
    WORD = 1
    DWORD = 2

HEX_DIGITS = "0123456789abcdefABCDEF"

registers = {
    "r0":0,
    "r1":1,
    "r2":2,
    "r3":3,
    "r4":4,
    "r5":5,
    "r6":6,
    "r7":7,
    "r8":8,
    "r9":9,
    "r10":10,
    "r11":11,
    "r12":12,
    "r13":13,
    "r14":14,
    "r15":15
}

instr = {
    "nop":[Instruction(0, InstructionTypes.TYPE5)],
    "add":[Instruction(1, InstructionTypes.TYPE0), Instruction(2, InstructionTypes.TYPE3)],
    "sub":[Instruction(3, InstructionTypes.TYPE0), Instruction(4, InstructionTypes.TYPE3)],
    "mul":[Instruction(5, InstructionTypes.TYPE0), Instruction(6, InstructionTypes.TYPE3)],
    "div":[Instruction(7, InstructionTypes.TYPE0), Instruction(8, InstructionTypes.TYPE3)],
    "cpy":[Instruction(9, InstructionTypes.TYPE0)],
    "xchg":[Instruction(10, InstructionTypes.TYPE0)],
    "and":[Instruction(11, InstructionTypes.TYPE0), Instruction(12, InstructionTypes.TYPE3)],
    "or":[Instruction(13, InstructionTypes.TYPE0), Instruction(14, InstructionTypes.TYPE3)],
    "not":[Instruction(15, InstructionTypes.TYPE4)],
    "xor":[Instruction(16, InstructionTypes.TYPE0), Instruction(17, InstructionTypes.TYPE3)],
    "inc":[Instruction(18, InstructionTypes.TYPE4)],
    "dec":[Instruction(19, InstructionTypes.TYPE4)],
    "push":[Instruction(20, InstructionTypes.TYPE4), Instruction(21, InstructionTypes.TYPE2)],
    "pop":[Instruction(22, InstructionTypes.TYPE4)],
    "jmp":[Instruction(23, InstructionTypes.TYPE2)],
    "call":[Instruction(24, InstructionTypes.TYPE2)],
    "int":[Instruction(25, InstructionTypes.TYPE2)],
    "ld":[Instruction(26, InstructionTypes.TYPE1), Instruction(27, InstructionTypes.TYPE3)],
    "st":[Instruction(28, InstructionTypes.TYPE1)],
    "cmp":[Instruction(29, InstructionTypes.TYPE0), Instruction(30, InstructionTypes.TYPE3)],
    "ret":[Instruction(31, InstructionTypes.TYPE5)],
    "iret":[Instruction(32, InstructionTypes.TYPE5)],
    "hlt":[Instruction(33, InstructionTypes.TYPE5)]
}

RE_OPERAND = r"[\[]?[a-zA_Z0-9#\-_.]+[\]]?"
RE_INSTR = rf"^(?:|\s+)(?P<opcode>[a-z]+)(?:.(?P<size>[bwd]))?(?: (?P<operand1>{RE_OPERAND})(?:, (?P<operand2>{RE_OPERAND}))?)?$"
RE_LABEL = r"^(?:|\s+)(?P<label>[a-zA-Z0-9_]+):$"
RE_OPERAND_LABEL = r"^[a-zA-Z0-9_]+$"
RE_OPERAND_LABEL_PTR = r"^\[[a-zA-Z0-9_]+\]$"
RE_HEX = r"[0-9a-fA-F]+"
RE_OPERAND_PTR = rf"^\[0x{RE_HEX}\]$"

class PetuchPCAsm:
    
    def __init__(self, code: str):
        self.code = code
        self.tokens = []
        self.labels = {}
        self.relocations = []

    def get_operand_type(self, operand: str):
        if operand in registers:
            return OperandTypes.REG
        elif operand.startswith("0x") and re.match(RE_HEX, operand[2:]):
            return OperandTypes.IMM
        elif operand.isdigit():
            return OperandTypes.IMM
        elif re.match(RE_OPERAND_PTR, operand):
            return OperandTypes.IMM_PTR
        elif re.match(RE_OPERAND_LABEL, operand):
            return OperandTypes.LABEL
        elif re.match(RE_OPERAND_LABEL_PTR, operand):
            return OperandTypes.LABEL_PTR
        
        print(f"Invalid operand: {operand}")
        sys.exit()

    def parse_label(self, line: str):
        m = re.match(RE_LABEL, line)

        token = {
            "type":TokenTypes.LABEL,
            "name":m.group("label")
        }

        self.tokens.append(token)

    def parse_instruction(self, line: str):
        m = re.match(RE_INSTR, line)

        if not m: return
        if not m.group("opcode") in instr:
            print(f"Invalid instruction: {m.group('opcode')}")
            sys.exit()

        token = {
            "type":TokenTypes.INSTRUCTION,
            "opcode":m.group("opcode"),
            "operands":list(
                filter(
                    lambda f: f is not None,
                    [m.group("operand1"), m.group("operand2")]
                )
            ),
            "size":m.group("size") or "d"
        }
        
        self.tokens.append(token)
        
    def make_tokens(self):
        for line in self.code:
            if re.match(RE_LABEL, line):
                self.parse_label(line)
            elif line.startswith("."):
                self.tokens.append({
                    "type":TokenTypes.DIRECTIVE,
                    "data":line
                })
            else: self.parse_instruction(line)

    def get_instruction_type(self, token):
        if len(token["operands"]) == 2:
            operand1_type = self.get_operand_type(token["operands"][0])
            operand2_type = self.get_operand_type(token["operands"][1])
            if operand1_type == OperandTypes.REG:
                match operand2_type:
                    case OperandTypes.REG:
                        return InstructionTypes.TYPE0
                    case OperandTypes.IMM_PTR:
                        return InstructionTypes.TYPE1
                    case OperandTypes.LABEL_PTR:
                        return InstructionTypes.TYPE1
                    case OperandTypes.IMM:
                        return InstructionTypes.TYPE3
                    case OperandTypes.LABEL:
                        return InstructionTypes.TYPE3
        elif len(token["operands"]) == 1:
            operand_type = self.get_operand_type(token["operands"][0])
            match operand_type:
                case OperandTypes.IMM:
                    return InstructionTypes.TYPE2
                case OperandTypes.LABEL:
                    return InstructionTypes.TYPE2
                case OperandTypes.REG:
                    return InstructionTypes.TYPE4
        else:
            return InstructionTypes.TYPE5
        sys.exit()

    def get_imm_value(self, imm):
        if (imm.isdigit()): return int(imm)
        return int(imm.lstrip("[").rstrip("]").rstrip("h"), 16)

    def get_imm_size(self, operand):
        imm = self.get_imm_value(operand)
        if imm <= 0xff:
            return Size.BYTE
        elif imm > 0xff and imm <= 0xffff:
            return Size.WORD
        else:
            if imm > 0xffffffff:
                print(f"Warning: overflow: {operand}")
            return Size.DWORD
        
    def get_size(self, token):
        match token["size"]:
            case "b": return Size.BYTE
            case "w": return Size.WORD
            case "d": return Size.DWORD
        return Size.DWORD

    def get_instruction(self, token):
        instructions = instr[token["opcode"]]
        instr_type = self.get_instruction_type(token)
        for i in instructions:
            if instr_type == i.type: return i
        print("Invalid operands")
        sys.exit()

    def reloc(self, data):
        for rel in self.relocations:
            if rel["name"] in self.labels:
                offset = self.labels[rel["name"]]
                data[rel["offset"]] = offset & 0xff
                data[rel["offset"]+1] = (offset & 0xff00) >> 8
                data[rel["offset"]+2] = (offset & 0xff0000) >> 16
                data[rel["offset"]+3] = (offset & 0xff000000) >> 24
            else: print(f"Undefined label: {rel['name']}")

    def assemble_instructions(self):
        self.make_tokens()
        offset = 0
        data = bytearray()

        for token in self.tokens:
            #print(token)
            if token["type"] == TokenTypes.INSTRUCTION:
                instruction = self.get_instruction(token)
                
                match instruction.type:
                    case InstructionTypes.TYPE0:
                        o = \
                            instruction.opcode << 10 \
                            | registers[token["operands"][0]] << 6 \
                            | registers[token["operands"][1]] << 2

                        data.append(o & 0xff)
                        data.append((o & 0xff00) >> 8)
                        
                        offset += 2
                    case InstructionTypes.TYPE1:
                        o = \
                            instruction.opcode << 10 \
                            | self.get_size(token).value << 8 \
                            | registers[token["operands"][0]] << 4
                        data.append(o & 0xff)
                        data.append((o & 0xff00) >> 8)

                        operand2 = 0
                        if self.get_operand_type(token["operands"][1]) == OperandTypes.LABEL_PTR:
                            self.relocations.append({
                                "offset":len(data),
                                "name":token["operands"][1].lstrip("[").rstrip("]")
                            })
                        else:
                            operand2 = self.get_imm_value(token["operands"][1])

                        data.append(operand2 & 0xff)
                        data.append((operand2 & 0xff00) >> 8)
                        data.append((operand2 & 0xff0000) >> 16)
                        data.append((operand2 & 0xff000000) >> 24)

                        offset += 6
                    case InstructionTypes.TYPE2:
                        operand = 0
                        if self.get_operand_type(token["operands"][0]) == OperandTypes.LABEL:
                            size = Size.DWORD
                            self.relocations.append({
                                "offset":len(data)+1,
                                "name":token["operands"][0].lstrip("[").rstrip("]")
                            })
                        else:
                            size = self.get_imm_size(token["operands"][0])
                            operand = self.get_imm_value(token["operands"][0])
                        o = \
                            instruction.opcode << 2 \
                            | size.value & 0b11
                        data.append((o & 0xff00) >> 8)
                        data.append(o & 0xff)
                        match size:
                            case Size.BYTE:
                                data.append(operand & 0xff)
                            case Size.WORD:
                                data.append(operand & 0xff)
                                data.append((operand & 0xff00) >> 8)
                            case Size.DWORD:
                                data.append(operand & 0xff)
                                data.append((operand & 0xff00) >> 8)
                                data.append((operand & 0xff0000) >> 16)
                                data.append((operand & 0xff000000) >> 24)

                        offset += 1 + (2**(size.value))
                    case InstructionTypes.TYPE3:
                        operand = 0
                        if self.get_operand_type(token["operands"][1]) == OperandTypes.LABEL:
                            size = Size.DWORD
                            self.relocations.append({
                                "offset":len(data)+2,
                                "name":token["operands"][1].lstrip("[").rstrip("]")
                            })
                        else:
                            size = self.get_imm_size(token["operands"][1])
                            operand = self.get_imm_value(token["operands"][1])
                        o = \
                            instruction.opcode << 10 \
                            | ((size.value & 0b11) << 8) \
                            | (registers[token["operands"][0]] << 4)
                        data.append(o & 0xff)
                        data.append((o & 0xff00) >> 8)
                        match size:
                            case Size.BYTE:
                                data.append(operand & 0xff)
                            case Size.WORD:
                                data.append(operand & 0xff)
                                data.append((operand & 0xff00) >> 8)
                            case Size.DWORD:
                                data.append(operand & 0xff)
                                data.append((operand & 0xff00) >> 8)
                                data.append((operand & 0xff0000) >> 16)
                                data.append((operand & 0xff000000) >> 24)

                        offset += 2 + (2**(size.value))
                    case InstructionTypes.TYPE4:
                        o = \
                            instruction.opcode << 10 \
                            | (registers[token["operands"][0]] << 6)
                        data.append(o & 0xff)
                        data.append((o & 0xff00) >> 8)

                        offset += 2
                    case InstructionTypes.TYPE5:
                        o = \
                            instruction.opcode << 2
                        data.append(o & 0xff)
                        data.append((o & 0xff00) >> 8)
                        
                        offset += 1
            elif token["type"] == TokenTypes.LABEL:
                self.labels.update({token["name"]:offset})
            elif token["type"] == TokenTypes.DIRECTIVE:
                s = re.split(r",? ", token["data"])
                match s[0]:
                    case ".byte":
                        for b in s[1:]:
                            data.append(self.get_imm_value(b))
                            offset += 1
                    case ".string":
                        s = token["data"][len(s[0])+2:len(token["data"])-1]
                        for c in s:
                            data.append(ord(c))
                            offset += 1
                    case ".stringz":
                        s = token["data"][len(s[0])+2:len(token["data"])-1]
                        for c in s:
                            data.append(ord(c))
                            offset += 1
                        data.append(0)
                        offset += 1
                    case ".org":
                        offset = self.get_imm_value(s[1])
        #print(self.labels)
        self.reloc(data)
        for b in data:
            print(hex(b), end=" ")

        with open("out.bin", "wb") as file:
        	file.write(data)