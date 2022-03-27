        .globl main

        .section .data
VERSE_TEMPLATE:
        .string "%ld green bottles hanging on the wall,\n%ld green bottles hanging on the wall,\nand if 1 green bottle should accidentally fall,\nthere'd be %ld green bottles hanging on the wall.\n"

        .section .text
main:
    # assigning a stack frame even if we don't know why
    pushq %rbp
    movq %rsp, %rbp
    # now we are ready
    movq $10, %rbx
print_loop:
    movq %rbx, %rsi
    movq %rbx, %rdx
    decq %rbx
    movq %rbx, %rcx
    movq $VERSE_TEMPLATE, %rdi
    call printf
    cmpq $0, %rbx
    jne print_loop
end:
    leave
    ret
