#include<stdio.h>
#include<stdint.h>

// ==========================================
// 1. 模拟 CPU 状态
// ==========================================
uint8_t R[4] = {0}; // 寄存器
uint8_t PC;

#define PRINT_FUNCINFO 0    // 控制函数内是否打印信息
#define PRINT_REGISTER 1    // 控制是否输出寄存器

// ==========================================
// 2. 指令注册表
// ==========================================
typedef void (*FuncHandler)(uint8_t raw_ins);
typedef struct{
    const char *name;
    FuncHandler handler;
}OpcodeEntry;
#define REGISTRY_SIZE 4
OpcodeEntry InstructionRegistry[REGISTRY_SIZE];


// ==========================================
// 3. 具体指令实现
// ==========================================

#if PRINT_FUNCINFO == 1
    // 文字模式：保留指令内部的 printf
    #define LOG_TEXT(...) printf(__VA_ARGS__)
#else
    // 寄存器表格模式：静默具体指令内部的明文打印
    #define LOG_TEXT(...) ((void)0)
#endif

void isa_add(uint8_t raw){
    uint8_t rs2 = raw & 0x03;
    uint8_t rs1 = (raw >> 2) & 0x03;
    uint8_t rd  = (raw >> 4) & 0x03;
    R[rd] = R[rs1] + R[rs2];
    LOG_TEXT("[%s] R[%d] = R[%d] + R[%d] -> R[%d]=%d\n", __func__, rd, rs1, rs2, rd, R[rd]);
    PC++;
}

void isa_li(uint8_t raw){
    uint8_t rd  = (raw >> 4) & 0x03;
    uint8_t imm = raw & 0x0F;
    R[rd] = imm;
    LOG_TEXT("[%s] R[%d]=%d\n", __func__, rd, imm);
    PC++;
}

void isa_bner0(uint8_t raw){
    uint8_t rs2  = raw & 0x03;
    uint8_t addr = (raw >> 2) & 0x0F;

    if(R[rs2] != R[0]){
        PC = addr;
        LOG_TEXT("Jump to PC=%d\n", PC);
    }
    else{
        PC++;
        LOG_TEXT("No Jump, Next PC\n");
    }
}


// ==========================================
// 4. 指令注册
// ==========================================
void register_instruction(uint8_t opcode, const char *name, FuncHandler handler){
    if(opcode < REGISTRY_SIZE){
        InstructionRegistry[opcode].name = name;
        InstructionRegistry[opcode].handler = handler;
    }
}

void init_cpu(void){
    register_instruction(0b00, "ADD", isa_add);
    register_instruction(0b10, "LI", isa_li);
    register_instruction(0b11, "BNER0", isa_bner0);
}


// ==========================================
// 5. 执行器
// ==========================================
#if PRINT_REGISTER
    void print_cpu_header(void) {
        printf("\n=== Execution Trace ===\n");
        printf("%-4s ", "PC");
        for (int i = 0; i < REGISTRY_SIZE; i++) {
            printf("R[%d] ", i);
        }
        printf("\n-----------------------\n");
    }
    void print_cpu_snapshot(void) {
        // 使用左对齐格式化，保证数据和表头完美对齐
        printf("0x%02X ", PC);
        for (int i = 0; i < REGISTRY_SIZE; i++) {
            printf("%-4d ", R[i]);
        }
        printf("\n");
    }
#endif
void execute(uint8_t raw_ins){
    uint8_t opcode = (raw_ins >> 6) & 0x03;
    OpcodeEntry entry = InstructionRegistry[opcode];
    
    if (entry.handler != NULL) {
        // 直接调用注册的函数
        entry.handler(raw_ins);
    } else {
        printf("[ERROR] Unregistered Opcode: 0b%d%d (Raw: 0x%02X)\n", 
               (opcode >> 1) & 1, opcode & 1, raw_ins);
        PC += 1; // 遇到未知指令跳过，防止死循环
    }
    #if PRINT_REGISTER
        print_cpu_snapshot();
    #endif
}


// ==========================================
// 6. PC 程序
// ==========================================
uint8_t PC_list[] = {
    0b10001011, //  0: li    r0 11  // r0 存放跳转推出
    0b10010001, //  1: li    r1 1   // r1 存放当前数字
    0b10100000, //  2: li    r2 0  // r2 存放计算结果
    0b10110010, //  3: li    r3 2   // r3 用于 r1 递增
    0b00101001, //  4: add   r2 r1 r2
    0b00010111, //  5: add   r1 r1 r3
    0b11010001, //  6: bner0 r1 4
    // 最后死循环的没加，不然 C 语言停不下来了
};

int main(void){
    init_cpu();
    int program_length = sizeof(PC_list) / sizeof(PC_list[0]);

    #if PRINT_REGISTER
        print_cpu_header();
    #endif
    while(PC < program_length)
        execute(PC_list[PC]);

    return 0;
}