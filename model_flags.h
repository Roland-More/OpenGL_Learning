#ifndef MODEL_FLAGS_H
#define MODEL_FLAGS_H

enum ModelFlags {
    ModelLoad_None        = 0,
    ModelLoad_FlipUVs     = 1 << 0, //      0001
    ModelLoad_Tangents    = 1 << 1, //      0010
    ModelLoad_GAMMA_CRCT  = 1 << 2, //      0100
    ModelLoad_PBR         = 1 << 3, //      1000
    ModelLoad_CustomTex   = 1 << 4, // 0001 0000
    // add more as needed
};

#endif
