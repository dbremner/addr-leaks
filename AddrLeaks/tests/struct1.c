#include <stdio.h>

/* Test indirect leaks via a one-level struct.
 */

int main() {
    struct s1 {
        int a;
        int b;
        int *c;
    };

    struct s1 s;

    printf("%d\n", s.a);
    printf("%d\n", s.b);
    printf("%d\n", s.c);

    s.a = 123;
    s.b = 321;
    s.c = &s.b;
    
    printf("%d\n", s.a);
    printf("%d\n", s.b);
    printf("%d\n", s.c);

    s.a = (int)&s.a;
    s.b = 123;
    s.c = &s.b;

    printf("%d\n", s.a);
    printf("%d\n", s.b);
    printf("%d\n", s.c);

    return 0;
}
