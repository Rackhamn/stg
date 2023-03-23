#ifndef STG_MAT4_H
#define STG_MAT4_H

#include <math.h>

struct vec4_s {
    union {
        float a[4];
        struct { float x, y, z, w; };
    };
};
typedef struct vec4_s vec4;

void set_rgb_vec4(unsigned char r, unsigned char g, unsigned char b, vec4 * v) {
    v->x = (float)r / 255.0f;
    v->y = (float)g / 255.0f;
    v->z = (float)b / 255.0f;
    v->w = 1.0f;
}

struct vec3_s {
    union {
        float a[3];
        struct { float x, y, z; };
        struct { float u, v, w; };
    };
};
typedef struct vec3_s vec3;

void zero_vec3(vec3 * m) {
    m->x = 0;
    m->y = 0;
    m->z = 0;
}

void set_vec3(float x, float y, float z, vec3 * m) {
    m->x = x;
    m->y = y;
    m->z = z;
}

void add_vec3(vec3 * a, vec3 * b, vec3 * c) {
    c->x = a->x + b->x;
    c->y = a->y + b->y;
    c->z = a->z + b->z;   
}

void sub_vec3(vec3 * a, vec3 * b, vec3 * c) {
    c->x = a->x - b->x;
    c->y = a->y - b->y;
    c->z = a->z - b->z;
}

void divs_vec3(vec3 * a, float s) {
    a->x /= s;
    a->y /= s;
    a->z /= s;
}

void cross_vec3(vec3 * a, vec3 * b, vec3 * c) {
    c->x = (a->y * b->z) - (b->y * a->z);
    c->y = (a->z * b->x) - (b->z * a->x);
    c->z = (a->x * b->y) - (b->x * a->y);
}

float dot_vec3(vec3 * a, vec3 * b) {
    return ((a->x * b->x) + (a->y * b->y) + (a->z * b->z));
}

float mag_vec3(vec3 * a) {
    return sqrt(dot_vec3(a, a));
}

void norm_vec3(vec3 * a) {
    float m = mag_vec3(a);
    if(m == 0.0f) {
        zero_vec3(a);
        return;
    }
    divs_vec3(a, m);
}



#define MAT4_SIZE   (sizeof(float) * 16)
struct mat4_s {
    union {
        // mat3 _mat3;

        float v[16];
        float m[4][4];
        // __m128 r[4];

        // named access into the rows
        struct {
            // ess. vec4            
            struct {
                // __m128 row0;
                float m00, m01, m02, m03;
            };
            struct {
                // __m128 row1;
                float m10, m11, m12, m13;
            };
            struct {
                // __m128 row2;
                float m20, m21, m22, m23;
            };
            struct {
                // __m128 row3;
                float m30, m31, m32, m33;
            };
        };
    };
};
typedef struct mat4_s mat4;

void zero_mat4(mat4 * m) {
    memset(m->v, 0, sizeof(float) * 16); // 64B
}

void identity_mat4(mat4 * m) {
    m->v[0] = 1.0f;
    m->v[1] = 0.0f;
    m->v[2] = 0.0f;
    m->v[3] = 0.0f;
    
    m->v[4] = 0.0f;
    m->v[5] = 1.0f;
    m->v[6] = 0.0f;
    m->v[7] = 0.0f;

    m->v[8] = 0.0f;
    m->v[9] = 0.0f;
    m->v[10] = 1.0f;
    m->v[11] = 0.0f;

    m->v[12] = 0.0f;
    m->v[13] = 0.0f;
    m->v[14] = 0.0f;
    m->v[15] = 1.0f;
}

void copy_mat4(mat4 * target, mat4 * source) {
    memcpy(source->v, target->v, sizeof(float) * 16);
}

void translate_mat4(float x, float y, float z, mat4 * m) {
    m->v[12] = x;
    m->v[13] = y;
    m->v[14] = z;
}

void scale_mat4(float x, float y, float z, mat4 * m) {
    m->v[0] = x;
    m->v[5] = y;
    m->v[10] = z;
}

void perspective_mat4(float fov, float aspect_ratio, float z_near, float z_far, mat4 * m) {
    float f, fn;
    zero_mat4(m);
    
    f = 1.0f / tanf(fov * 0.5f);
    fn = 1.0f / (z_near - z_far);

    m->m[0][0] = f / aspect_ratio;
    m->m[1][1] = f;
    m->m[2][2] = (z_near + z_far) * fn;
    m->m[2][3] = -1.0f;

    m->m[3][2] = 2.0f * (z_near * z_far * fn);
}

void lookat_mat4(vec3 eye, vec3 dir, vec3 up, mat4 * m) {
    vec3 f, u, s, c;

    add_vec3(&eye, &dir, &c);
    sub_vec3(&c, &eye, &f);
    norm_vec3(&f);
    cross_vec3(&f, &up, &s);
    norm_vec3(&s);
    cross_vec3(&s, &f, &u);

    m->m[0][0] =  s.a[0];
	m->m[0][1] =  u.a[0];
	m->m[0][2] = -f.a[0];
	m->m[1][0] =  s.a[1];
	m->m[1][1] =  u.a[1];
	m->m[1][2] = -f.a[1];
	m->m[2][0] =  s.a[2];
	m->m[2][2] = -f.a[2];
	m->m[3][0] = -dot_vec3(&s, &eye);
	m->m[3][1] = -dot_vec3(&u, &eye);
	m->m[3][2] =  dot_vec3(&f, &eye);
	m->m[0][3] =  m->m[1][3] = m->m[2][3] = 0.0f;
	m->m[3][3] =  1.0f;
}

void mul_mat4(mat4 * a, mat4 * b, mat4 * c) {
    float 	
		// load raw data
		a00 = a->m[0][0], a01 = a->m[0][1], a02 = a->m[0][2], a03 = a->m[0][3],
		a10 = a->m[1][0], a11 = a->m[1][1], a12 = a->m[1][2], a13 = a->m[1][3],
		a20 = a->m[2][0], a21 = a->m[2][1], a22 = a->m[2][2], a23 = a->m[2][3],
		a30 = a->m[3][0], a31 = a->m[3][1], a32 = a->m[3][2], a33 = a->m[3][3],

		b00 = b->m[0][0], b01 = b->m[0][1], b02 = b->m[0][2], b03 = b->m[0][3],
		b10 = b->m[1][0], b11 = b->m[1][1], b12 = b->m[1][2], b13 = b->m[1][3],
		b20 = b->m[2][0], b21 = b->m[2][1], b22 = b->m[2][2], b23 = b->m[2][3],
		b30 = b->m[3][0], b31 = b->m[3][1], b32 = b->m[3][2], b33 = b->m[3][3];
	
	// perform the mul
	c->m[0][0] = (a00 * b00) + (a10 * b01) + (a20 * b02) + (a30 * b03);
	c->m[0][1] = (a01 * b00) + (a11 * b01) + (a21 * b02) + (a31 * b03);
	c->m[0][2] = (a02 * b00) + (a12 * b01) + (a22 * b02) + (a32 * b03);
	c->m[0][3] = (a03 * b00) + (a13 * b01) + (a23 * b02) + (a33 * b03);
	c->m[1][0] = (a00 * b10) + (a10 * b11) + (a20 * b12) + (a30 * b13);
	c->m[1][1] = (a01 * b10) + (a11 * b11) + (a21 * b12) + (a31 * b13);
	c->m[1][2] = (a02 * b10) + (a12 * b11) + (a22 * b12) + (a32 * b13);
	c->m[1][3] = (a03 * b10) + (a13 * b11) + (a23 * b12) + (a33 * b13);
	c->m[2][0] = (a00 * b20) + (a10 * b21) + (a20 * b22) + (a30 * b23);
	c->m[2][1] = (a01 * b20) + (a11 * b21) + (a21 * b22) + (a31 * b23);
	c->m[2][2] = (a02 * b20) + (a12 * b21) + (a22 * b22) + (a32 * b23);
	c->m[2][3] = (a03 * b20) + (a13 * b21) + (a23 * b22) + (a33 * b23);
	c->m[3][0] = (a00 * b30) + (a10 * b31) + (a20 * b32) + (a30 * b33);
	c->m[3][1] = (a01 * b30) + (a11 * b31) + (a21 * b32) + (a31 * b33);
	c->m[3][2] = (a02 * b30) + (a12 * b31) + (a22 * b32) + (a32 * b33);
	c->m[3][3] = (a03 * b30) + (a13 * b31) + (a23 * b32) + (a33 * b33);   
}

void transpose_mat4(mat4 * m) {
    mat4 t;
    copy_mat4(&t, m);

    m->m[0][0] = t.m[0][0]; 
	m->m[1][0] = t.m[0][1];
	m->m[0][1] = t.m[1][0];
	m->m[1][1] = t.m[1][1];
	m->m[0][2] = t.m[2][0];
	m->m[1][2] = t.m[2][1];
	m->m[0][3] = t.m[3][0];
	m->m[1][3] = t.m[3][1];
	m->m[2][0] = t.m[0][2];
	m->m[3][0] = t.m[0][3];
	m->m[2][1] = t.m[1][2];
	m->m[3][1] = t.m[1][3];
	m->m[2][2] = t.m[2][2];
	m->m[3][2] = t.m[2][3];
	m->m[2][3] = t.m[3][2];
    m->m[3][3] = t.m[3][3];    
}

void rot_x_mat4(float x, mat4 * m) {
    mat4 v;
    identity_mat4(&v);

    float c = cos(x);
    float s = sin(x);

    v.m[1][1] = c;    
    v.m[1][2] = s;
    v.m[2][1] = -s;
    v.m[2][2] = c;

    mul_mat4(&v, m, m);
}

void rot_y_mat4(float y, mat4 * m) {
    mat4 v;
    identity_mat4(&v);

    float c = cos(y);
    float s = sin(y);

    v.m[0][0] = c;    
    v.m[0][2] = -s;
    v.m[2][0] = s;
    v.m[2][2] = c;

    mul_mat4(&v, m, m);
}

void rot_z_mat4(float z, mat4 * m) {
    mat4 v;
    identity_mat4(&v);

    float c = cos(z);
    float s = sin(z);

    v.m[0][0] = c;    
    v.m[0][1] = s;
    v.m[1][0] = -s;
    v.m[1][1] = c;

    mul_mat4(&v, m, m);
}


void rot_x_cs_mat4(float c, float s, mat4 * m) {
    mat4 v;
    identity_mat4(&v);

    v.m[1][1] = c;    
    v.m[1][2] = s;
    v.m[2][1] = -s;
    v.m[2][2] = c;

    mul_mat4(&v, m, m);
}

void rot_y_cs_mat4(float c, float s, mat4 * m) {
    mat4 v;
    identity_mat4(&v);

    v.m[0][0] = c;    
    v.m[0][2] = -s;
    v.m[2][0] = s;
    v.m[2][2] = c;

    mul_mat4(&v, m, m);
}

void rot_z_cs_mat4(float c, float s, mat4 * m) {
    mat4 v;
    identity_mat4(&v);

    v.m[0][0] = c;    
    v.m[0][1] = s;
    v.m[1][0] = -s;
    v.m[1][1] = c;

    mul_mat4(&v, m, m);
}

void print_mat4(mat4 * m) {
    int i = 0; 
    while(i < 16) {
        printf("%.2f ", m->v[i]);
        i++;
    }
    printf("\n");
}

void print_mat4_rows(mat4 * m) {
    int i = 0; 
    int j = 0;
    while(j < 4) {
        printf("[ ");
        i = 0;
        while(i < 4) {
            printf("%.2f ", m->m[j][i]);
            i++;
        }
        printf("]\n");
        j++;
    } 
    printf("\n");
}

void mat4_test(void) {
    mat4 matrix;

    // assert(sizeof(mat4) == MAT4_SIZE);
    printf("size of mat4: %lu Bytes\n", sizeof(mat4)); 

    memset(&matrix, 0, MAT4_SIZE);

    print_mat4_rows(&matrix);    

    matrix.v[0] = 1;
    print_mat4_rows(&matrix);

    matrix.m[1][1] = 2;
    print_mat4_rows(&matrix);

    matrix.m22 = 3;
    print_mat4_rows(&matrix);

    identity_mat4(&matrix);
    print_mat4_rows(&matrix);
}

#endif /* STG_MAT4_H */

