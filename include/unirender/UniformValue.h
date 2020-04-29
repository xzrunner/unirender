#pragma once

namespace ur
{

// Float

struct Float1
{
    Float1() {
        x = 0;
    }

    union
    {
        float x;
        float v[1];
    };
};

struct Float2
{
    Float2() {
        x = 0;
        y = 0;
    }

    union
    {
        struct
        {
            float x;
            float y;
        };

        float v[2];
    };
};

struct Float3
{
    Float3() {
        x = 0;
        y = 0;
        z = 0;
    }

    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };

        float v[3];
    };
};

struct Float4
{
    Float4() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }

    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };

        float v[4];
    };
};

// Int

struct Int1
{
    Int1() {
        x = 0;
    }

    union
    {
        int x;
        int v[1];
    };
};

struct Int2
{
    Int2() {
        x = 0;
        y = 0;
    }

    union
    {
        struct
        {
            int x;
            int y;
        };

        int v[2];
    };
};

struct Int3
{
    Int3() {
        x = 0;
        y = 0;
        z = 0;
    }

    union
    {
        struct
        {
            int x;
            int y;
            int z;
        };

        int v[3];
    };
};

struct Int4
{
    Int4() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }

    union
    {
        struct
        {
            int x;
            int y;
            int z;
            int w;
        };

        int v[4];
    };
};

// UInt

struct UInt1
{
    UInt1() {
        x = 0;
    }

    union
    {
        unsigned int x;
        unsigned int v[1];
    };
};

struct UInt2
{
    UInt2() {
        x = 0;
        y = 0;
    }

    union
    {
        struct
        {
            unsigned int x;
            unsigned int y;
        };

        unsigned int v[2];
    };
};

struct UInt3
{
    UInt3() {
        x = 0;
        y = 0;
        z = 0;
    }

    union
    {
        struct
        {
            unsigned int x;
            unsigned int y;
            unsigned int z;
        };

        unsigned int v[3];
    };
};

struct UInt4
{
    UInt4() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }

    union
    {
        struct
        {
            unsigned int x;
            unsigned int y;
            unsigned int z;
            unsigned int w;
        };

        unsigned int v[4];
    };
};

// Matrix

struct Matrix22
{
    Matrix22() {
        m[0][0] = 1; m[1][0] = 0;
        m[0][1] = 0; m[1][1] = 1;
    }

    union {
        float m[2][2];
        float v[4];
    };
};

struct Matrix33
{
    Matrix33() {
        m[0][0] = 1; m[1][0] = 0; m[2][0] = 0;
        m[0][1] = 0; m[1][1] = 1; m[2][1] = 0;
        m[0][2] = 0; m[1][2] = 0; m[2][2] = 1;
    }

    union {
        float m[3][3];
        float v[9];
    };
};

struct Matrix44
{
    Matrix44() {
        m[0][0] = 1; m[1][0] = 0; m[2][0] = 0; m[3][0] = 0;
        m[0][1] = 0; m[1][1] = 1; m[2][1] = 0; m[3][1] = 0;
        m[0][2] = 0; m[1][2] = 0; m[2][2] = 1; m[3][2] = 0;
        m[0][3] = 0; m[1][3] = 0; m[2][3] = 0; m[3][3] = 1;
    }

    union {
        float m[4][4];
        float v[16];
    };
};

struct Matrix23
{
    Matrix23() {
        m[0][0] = 1; m[1][0] = 0;
        m[0][1] = 0; m[1][1] = 1;
        m[0][2] = 0; m[1][2] = 0;
    }

    union {
        float m[2][3];
        float v[6];
    };
};

struct Matrix32
{
    Matrix32() {
        m[0][0] = 1; m[1][0] = 0; m[2][0] = 0;
        m[0][1] = 0; m[1][1] = 1; m[2][1] = 0;
    }

    union {
        float m[3][2];
        float v[6];
    };
};

struct Matrix24
{
    Matrix24() {
        m[0][0] = 1; m[1][0] = 0;
        m[0][1] = 0; m[1][1] = 1;
        m[0][2] = 0; m[1][2] = 0;
        m[0][3] = 0; m[1][3] = 0;
    }

    union {
        float m[2][4];
        float v[8];
    };
};

struct Matrix42
{
    Matrix42() {
        m[0][0] = 1; m[1][0] = 0; m[2][0] = 0; m[3][0] = 0;
        m[0][1] = 0; m[1][1] = 1; m[2][1] = 0; m[3][1] = 0;
    }

    union {
        float m[4][2];
        float v[8];
    };
};

struct Matrix34
{
    Matrix34() {
        m[0][0] = 1; m[1][0] = 0; m[2][0] = 0;
        m[0][1] = 0; m[1][1] = 1; m[2][1] = 0;
        m[0][2] = 0; m[1][2] = 0; m[2][2] = 1;
        m[0][3] = 0; m[1][3] = 0; m[2][3] = 0;
    }

    union {
        float m[3][4];
        float v[12];
    };
};

struct Matrix43
{
    Matrix43() {
        m[0][0] = 1; m[1][0] = 0; m[2][0] = 0; m[3][0] = 0;
        m[0][1] = 0; m[1][1] = 1; m[2][1] = 0; m[3][1] = 0;
        m[0][2] = 0; m[1][2] = 0; m[2][2] = 1; m[3][2] = 0;
    }

    union {
        float m[4][3];
        float v[12];
    };
};

}