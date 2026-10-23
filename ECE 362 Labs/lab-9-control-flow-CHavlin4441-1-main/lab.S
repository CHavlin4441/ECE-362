######################################################
# Lab 9 template file
######################################################

######################################################
# .data is where variables will reside.
# We define the following below so you know what 
# their values are.
.data
.global lowercase_string
lowercase_string:
    // Specify a string ending with '\0'
    .asciz "ece 362 is awesome!"

// Align to 4bytes boundary as we have a string above
.balign 4
######################################################


######################################################
# All code goes in the .text section
.text

.global asm_strlen
asm_strlen:
    /* uint32_t asm_strlen(char *str)
     * Take in a pointer to a string and return
     * its length up to the null terminating
     * character `\0`
    **/   

    /* Enter your code after this comment */
    
    beqz a0, .strlen_null

    li t0, 0
    mv t1, a0

.strlen_loop:
    lb t2, 0(t1)

    beqz t2, .strlen_done

    addi t1, t1, 1
    addi t0, t0, 1
    j .strlen_loop

    .strlen_done: 

        mv a0, t0
        ret
    
    .strlen_null:
        li a0, 0

    /* Enter your code above this comment */
    ret

.global asm_fib
asm_fib:
    /* void asm_fib(int *arr, uint32_t length)
     * Accept a pointer to an array of integer and
     * the length of the array, then generate fibonacci
     * series starting at `0` and store the result
     * back to the array `arr` at corresponding indices.
     **/

    /* Enter your code after this comment */
    

    beqz a0, .fib_done
    beqz a1, .fib_done

    li t0, 0
    li t1, 1
    li t2, 0

.fib_loop:
    bgeu t2, a1, .fib_done

    slli t3, t2, 2
    add t4, a0, t3

    li t5, 0
    beq t2, t5, .fib_store_0
    li t5, 1
    beq t2, t5, .fib_store_1

    add t5, t0, t1
    mv t0, t1
    mv t1, t5

    sw t1, 0(t4)             
    j .fib_next

.fib_store_0:
    sw t0, 0(t4) 
    j .fib_next 

.fib_store_1:
    sw t1, 0(t4)             
    j .fib_next           

.fib_next:
    addi t2, t2, 1             
    j .fib_loop

.fib_done:

    /* Enter your code above this comment */
    ret

.global asm_toUppercase
asm_toUppercase:
    /* void asm_toUppercase(char *str)
     * loop through an entire string and only convert
     * the lowercase letter to uppercase, leaving the
     * rest of the string intact
     **/

    /* Enter your code after this comment */
    
    beqz a0, .upper_done        

.upper_loop:
    lb t0, 0(a0)              
    beqz t0, .upper_done        

    li t1, 'a'
    blt  t0, t1, .upper_next   
    li t1, 'z'
    bgt t0, t1, .upper_next    

    # Convert: c = c - 32
    addi t0, t0, -32
    sb t0, 0(a0)            

.upper_next:
    addi a0, a0, 1            
    j .upper_loop

.upper_done:

    /* Enter your code above this comment */
    ret


######################################################
