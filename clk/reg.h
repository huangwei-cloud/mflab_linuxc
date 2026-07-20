#ifndef _REG_H_
#define _REG_H_

#include <stdint.h>

typedef struct {
    int id;                  // 寄存器编号 ID
    const char* name;        // 逻辑名称，如 "SCOPE_TRANS_SPEED"
    uint32_t offset;         // 寄存器偏移
    const char* desc;        // 参数描述
    const char* options;     // 选项值或类型提示（如 JSON 保存用）
} reg_entry_t;


#define REG(id, name, offset, desc, options) \
    ((reg_entry_t){id, #name, offset, desc, options})

void init_reg();

uint32_t reg_read(uint32_t offset);

// 通过名称读取寄存器的值
int reg_read_by_name(const char* name, uint32_t* value_out);

// 通过名称写入寄存器的值
int reg_write_by_name(const char* name, uint32_t value);

// 字符串值转数值写寄存器
int write_reg_by_name(const char* name, const char* str_val);

#endif

