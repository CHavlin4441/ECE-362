######################################################
# Lab 10 template file
######################################################

######################################################
# .data is where variables will reside.
# We define the following below so you know what 
# their values are.
.data
# Nothing in data this time.

// Align to 4bytes boundary as we have a string above
.balign 4
######################################################


######################################################
# All code goes in the .text section
.text
.global asm_sort_int
asm_sort_int:
    /* void asm_sort_int(int32_t* arr, uint32_t n)
     * relies on `qsort` in C standard library to 
     * sort in ascending order
     **/

    /* Enter your code after this comment */
    
    mv a2, a1
    li a3, 4
    la a4, asm_cmp
    tail qsort

    /* Enter your code above this comment */
    ret

.global asm_cmp
asm_cmp:
    // int asm_cmp(const void * a, const void * b)
    // Compare function to int32_t
    // used for qsort

    /* Enter your code after this comment */
    
    lw t0, 0(a0)
    lw t1, 0(a1)

    blt t0, t1, cmp_less
    bgt t0, t1, cmp_great
    
    li a0, 0

    ret

    cmp_less:
    li a0, -1
    ret

    cmp_great:
    li a0, 1
    ret

    /* Enter your code above this comment */
    ret

.global asm_strconcat
asm_strconcat:
    /* char* asm_strconcat(char * str1, char * str2)
     * First assign memory space with `malloc`, 
     * concatenating `str1` and `str2`, 
     * and return the resulted string.
     **/

    /* Enter your code after this comment */
    
    addi sp, sp, -32

    sw ra, 28(sp)
    sw s0, 24(sp)
    sw s1, 20(sp)
    sw s2, 16(sp)
    sw s3, 12(sp)
    sw s4, 8(sp)

    mv s0, a0
    mv s1, a1

    /*lens 1*/
    mv a0, s0
    jal ra, strlen
    mv s2, a0

    /*lens 2*/
    mv a0, s1
    jal ra, strlen
    mv s3, a0

    /*size*/

    add a0, s2, s3
    addi a0, a0, 1
    jal ra, malloc
    mv s4, a0

    /*memcopy 1*/

    mv a0, s4
    mv a1, s0
    mv a2, s2
    jal ra, memcpy

    /*memcopy 2*/

    add a0, s4, s2
    mv a1, s1
    mv a2, s3
    jal ra, memcpy

    /*buf*/

    add t0, s4, s2
    add t0, t0, s3
    sb zero, 0(t0)

    mv a0, s4            

    lw s4, 8(sp)
    lw s3, 12(sp)
    lw s2, 16(sp)
    lw s1, 20(sp)
    lw s0, 24(sp)
    lw ra, 28(sp)
    addi sp, sp, 32

    /* Enter your code above this comment */
    ret

.global asm_fib
asm_fib:
    /* uint32_t asm_fib(uint32_t n)
     * accept an index term `n` and return 
     * the Fn fibonacci term (F0 = 0, F1 = 1)
     **/

    /* Enter your code after this comment */
    
    addi sp, sp, -16
    sw ra, 12(sp)
    sw s0, 8(sp)

    li t0, 2
    blt a0, t0, fib_base

    mv s0, a0

    /* n-1 */
    addi a0, s0, -1
    jal ra, asm_fib
    sw a0, 4(sp)

    /* n-2 */
    addi a0, s0, -2
    jal ra, asm_fib

    /* sum of n-1 + n-2 */
    lw t0, 4(sp)
    add a0, a0, t0

    fib_base:

    fib_done: 
    lw s0, 8(sp)
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

    /* Enter your code above this comment */
    ret

.global asm_bsearch
asm_bsearch:
    /* int32_t asm_bsearch(int *arr, int key, 
                    int32_t start,
                    int32_t end)
     * Return the index of the element in the array
     * if the element does not exist, return -1 instead
     * arr: integer array to be searched
     * key: the element we want to search on
     * start: start index of subarray, inclusive
     * end: end index of subarray, inclusiveloop through an entire string and only convert
     **/

    /* Enter your code after this comment */
    
    addi sp, sp, -32

    sw ra, 28(sp)
    sw s0, 24(sp)
    sw s1, 20(sp)
    sw s2, 16(sp)
    sw s3, 12(sp)
    sw s4, 8(sp) 
    
    mv s0, a0
    mv s1, a1
    mv s2, a2
    mv s3, a3

    bgt s2, s3, bsearch_not

    add t0, s2, s3
    srli t0, t0, 1        
    mv s4, t0

    slli t1, s4, 2        
    add t1, s0, t1
    lw t2, 0(t1)          
    blt t2, s1, bsearch_right   
    bgt t2, s1, bsearch_left   

    mv a0, s4
    j bsearch_done

    bsearch_right:
    mv a0, s0
    mv a1, s1
    addi a2, s4, 1
    mv a3, s3
    jal ra, asm_bsearch
    j bsearch_done

    bsearch_left:
    mv a0, s0
    mv a1, s1
    mv a2, s2
    addi a3, s4, -1
    jal ra, asm_bsearch
    j bsearch_done


    bsearch_not:
    li a0, -1
    
    bsearch_done:
    lw s3, 12(sp)
    lw s2, 16(sp)
    lw s1, 20(sp)
    lw s0, 24(sp)
    lw ra, 28(sp)
    addi sp, sp, 32

    ret
    /* Enter your code above this comment */
    ret

######################################################
