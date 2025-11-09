/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: TLSAccessPatcher.hpp
Open source lines: 36/36 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "vector"

namespace CTRPluginFramework {
    class TLSAccessPatcher {
    public:
        struct Snippet {
            u32 pushAll = 0xE92DD00F; // PUSH {R0-R3, R12, LR, PC}
            u32 mrs_r0_cpsr = 0xE10F0000; // MRS  r0, CPSR
            u32 push_r0 = 0xE52D0004; // PUSH {r0}
            u32 blToGetThreadLocalStorage;
            u32 str_r0_sp_1c = 0xE58D001C; // STR R0, [SP, #0x1C]
            u32 pop_r0 = 0xE49D0004; // POP {R0}
            u32 msr_cpsr_r0 = 0xE129F000; // MSR CPSR, r0
            u32 popall = 0xE8BD500F; // POP {R0-R3, R12, LR}
            u32 pop_orig_reg;
            u32 ldr_pc_pc_4 = 0xE51FF004; // LDR PC, [PC, #-4]
            u32 backaddr;
        };

        static void PatchPlugin();
        static void Process(Snippet* snip, u32 address);

    private:
        static std::vector<Snippet> snippets;
    };
}
