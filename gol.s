	# PURPOSE:  This program plays the game of live.
	#

	# VARIABLES: The registers have the following uses:
	#
	#
	# The following memory locations are used:
	#
	# grid        - the playing field: 80x50 characters. A smaller part may be actually used (see params)
	# grid_next   - the playing field: 80x50 characters. Temporary for calculating the next generation; then copied into grid
	# width       - actual (current - from parameter) width of the field. Must be between 5 and 80.
	# height      - actual (current - from parameter) height of the field. Must be between 5 and 25.
	# generations - number of generations to calculate
	# rng         - current value of the random number generator. Initialized with seed value if present,
	#               otherwise 0. For simplicity 8 bytes, but only 31 bits used: rng=(1103515245*rng+12345)%2^31
	#

.equ STDOUT,1               # File handle for stdout
.equ SYS_WRITE,1            # System call number for "write to file"

# For testing with static patterns: Disable random initialization by setting to 0
.equ INIT_RND,1
# For debugging: Print neighbour count instead of cells; call only with generations set to 1
.equ PRINT_NEIGH,0

# Parameters for the random number generator: Xi=(Xi-1*A+C)/2 % 2^31
.equ RNG_A,1103515245
.equ RNG_C,12345

# EXIT Codes
.equ SUCCESS,0
.equ ERR_PARAM_NUM, -1
.equ ERR_ILLEGAL_VAL, -2

.section .bss
#... Define current and next grid here
# BUFFER_SIZE = 2000 -> max width = 80 * max height = 25
.equ BUFFER_SIZE, 2000
.lcomm CURR_GRID, BUFFER_SIZE
.lcomm NEXT_GRID, BUFFER_SIZE

.section .data
#... Define global data here
rng:		.quad 0         # Internal state of random generator
linebreak:	.asciz "\r\n"   # Constant string to print an empty line; initialized therefore cannot be in BSS
width:      .quad 0
height:     .quad 0
generations:.quad 0
optParam:   .quad 0

#TEST
prefix: .ascii "TEST \0"
.set prefix_len,.-prefix

# Testing - comment out grid in section bss, uncomment a single of these, and make sure to call with width and height 5!
/*
# Test 1 - Produces an empyt result
grid: .ascii    "     ",
      .ascii    "     ",
      .ascii    "     ",
      .ascii    "     ",
      .ascii    "     "

# Test 2 - Produces a single cell in each corner
grid: .ascii    "*****",
      .ascii    "*****",
      .ascii    "*****",
      .ascii    "*****",
      .ascii    "*****"

# Test 3 - Produces a centered vertical bar / next pattern
grid: .ascii    "     ",
      .ascii    "     ",
      .ascii    " *** ",
      .ascii    "     ",
      .ascii    "     "

# Test 4 - Produces a centered horizontal bar / previous pattern
grid: .ascii    "     ",
      .ascii    "  *  ",
      .ascii    "  *  ",
      .ascii    "  *  ",
      .ascii    "     "

# Test 5 - Oscillator: The two middle cells move from vertical to horizontal and back
grid: .ascii    "**   ",
      .ascii    "* *  ",
      .ascii    "     ",
      .ascii    "  * *",
      .ascii    "   **"

# Attention: This one needs a 40 by 40 grid! After 53 generations there are only 8 cells left,
# after 54 generations the field is empty
# Test 6 - Disappearance
grid: .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                  ***                   ",
      .ascii    "                  * *                   ",
      .ascii    "                  * *                   ",
      .ascii    "                                        ",
      .ascii    "                  * *                   ",
      .ascii    "                  * *                   ",
      .ascii    "                  ***                   ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        ",
      .ascii    "                                        "
*/

.section .text
    .equ ST_ARGC, 0    # Number of arguments
    .equ ST_ARGV_0, 8  # Program name
    .equ ST_ARGV_1, 16 # Width
    .equ ST_ARGV_2, 24 # Height
    .equ ST_ARGV_3, 32 # Number of generations
    .equ ST_ARGV_4, 40 # OPTIONAL: Seed of random generator

.globl _start

_start:
    # Create stack frame
	movq %rsp,%rbp

	# Check number of parameters
    cmpq $4, ST_ARGC(%rbp)
    je checkParameters
    jl endErrorArgNum

    cmpq $5, ST_ARGC(%rbp)
    je hasOptParam
    jg endErrorArgNum

hasOptParam:
    movq $1, optParam

checkParameters:
#-----------------------PARAMETER 1-----------------------------------------
    #save caller-save registers that are used in the function RAX, RDI, RDX
    pushq %rax
    pushq %rdi
    pushq %rdx

    #Get width
	movq ST_ARGV_1(%rbp), %rdi

	#convert to num and store value
	call convertToNum
	cmpq $-1, %rax
	je endIllegalValue
	movq %rax, width

	#restore caller-save registers
	popq %rdx
	popq %rdi
	popq %rax

	#save caller-save registers
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rax

	#check bounds
	movq width, %rdi
	movq $5, %rsi
	movq $80, %rdx
	call checkBounds

    #check, if the value is out of bounds
    #if true, end program with -2
	cmpq $0, %rax
	je endIllegalValue

    #restore caller-save registers
    popq %rax
    popq %rdx
    popq %rsi
    popq %rdi
#-----------------------PARAMETER 2-----------------------------------------
	pushq %rax
	pushq %rdi
	pushq %rdx

	#Get height
	movq ST_ARGV_2(%rbp), %rdi

	#convert
	call convertToNum
	cmpq $-1, %rax
	je endIllegalValue
	movq %rax, height

	#restore caller-save registers
	popq %rdx
	popq %rdi
	popq %rax

	#save caller-save registers
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rax

	#check bounds
	movq height, %rdi
	movq $5, %rsi
	movq $25, %rdx
	call checkBounds

	#check, if the value is out of bounds
    #if true, end program with -2
	cmpq $0, %rax
	je endIllegalValue

    #restore caller-save registers
    popq %rax
    popq %rdx
    popq %rsi
    popq %rdi
#-----------------------PARAMETER 3-----------------------------------------

	pushq %rax
	pushq %rdi
	pushq %rdx

	#Get height
	movq ST_ARGV_3(%rbp), %rdi

	#convert
	call convertToNum
	cmpq $-1, %rax
	je endIllegalValue
	movq %rax, generations

	#restore caller-save registers
	popq %rdx
	popq %rdi
	popq %rax

	#save caller-save registers
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rax

	#check bounds
	movq generations, %rdi
	movq $1, %rsi
	movq $100, %rdx
	call checkBounds

	#check, if the value is out of bounds
    #if true, end program with -2
	cmpq $0, %rax
	je endIllegalValue

    #restore caller-save registers
    popq %rax
    popq %rdx
    popq %rsi
    popq %rdi

#-----------------------OPTIONAL PARAMETER 4-----------------------------------------

	#check, if optional parameter is set

	cmpq $1, optParam
	jne fillGrid

    #if parameter 4 is set, convert it
    #save caller-save registers
	pushq %rax
	pushq %rdi
	pushq %rdx

	#Get height
	movq ST_ARGV_4(%rbp), %rdi

	#convert
	call convertToNum

	#if conversion succeeds: rng = num value
	#if conversion fails: %rax contains -1
	movq %rax, rng

	#restore caller-save registers
	popq %rdx
	popq %rdi
	popq %rax

	jmp fillGrid

# purpose: check a number, if it is in given bounds (inclusive)
# input: RDI number to check
#        RSI lower bound
#        RDX upper bound
# output: RAX with 0 out of bounds or 1 inside of bounds

.type checkBounds,@function
checkBounds:
    #in this case, no prologue is needed

    #default is inside of bounds
    #only check, if out of bounds
    movq $1, %rax

    #check lower bound
    cmpq %rsi, %rdi
    jl fault

    #check upper bound
    cmpq %rdx, %rdi
    jg fault

    #everything went fine
    ret

    fault:
        movq $0, %rax
        ret


# purpose: convert the input parameters from string to number
# input: RDI with address to string value
# output: RAX with converted number
# used registers:   RDX for string length
#                   R12B for character value
#                   RBX factor for multiplication
#                   R13 for loop counter
#                   R14 contains power of the highest digit
#                   R15 contains power of current digit

.type convertToNum,@function
convertToNum:
    #prologue
    pushq %rbp
    movq %rsp, %rbp

    #save callee save registers
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    #calculate length of string
    len_loop:
        cmpb $0, (%rdi,%rdx,1)
        je prepare_loop
        incq %rdx               #length is stored in rdx
        jmp len_loop

    prepare_loop:

        #store length in r8 and decrement by 1
        #to get the power
        movq %rdx, %r14
        decq %r14

        #set loop counter
        movq $0, %r13

    convert_loop:
        #check end condition
        cmpq %r13, %rdx
        je end_loop

        #move ascii value of character into %r12b
        mov (%r13,%rdi), %r12b

        #save caller-save registers
        pushq %rdi
        pushq %rsi
        pushq %rdx
        pushq %rax

        #check, if character is a digit
        movq %r12, %rdi #ascii value of character
        movq $48, %rsi  #ascii value of '0'
        movq $57, %rdx  #ascii value of '9'

        call checkBounds
        #check for error
        cmpq $0, %rax
        je end_loop_outOfBounds

        #restore caller-save registers
        popq %rax
        popq %rdx
        popq %rsi
        popq %rdi

        #cmpq $48, %r12
        #jl endIllegalValue

        #check, if character is '9'
        #cmpq $57, %r12
        #jg endIllegalValue

        #convert character to its actual value
        sub $48, %r12b

        #calculate %r12b * 10^n ... n = %rdx
        power:
            #temp factor for calculation
            movq %r14, %r15

            #check if power is 0 -> factor is 1
            movq $1, %rbx
            cmpq $0, %r15
            je compute

            power_loop:
                imulq $10, %rbx
                decq %r15

                cmpq $0, %r15
                jg power_loop
                decq %r14

        compute:
            #multiply character value with factor
            imulq %rbx, %r12

            #move return from rsivalue into rax
            addq %r12, %rax

            #increment loop counter
            incq %r13

            jmp convert_loop

        end_loop_outOfBounds:
            #set error return value
            movq $-1, %rax


        end_loop:
        #epilogue

        #restore callee-save registers
        popq %r15
        popq %r14
        popq %r13
        popq %r12
        popq %rbx

        movq %rbp, %rsp
        popq %rbp

        ret

#fill grid with random values
fillGrid:
    #initialize loop counters
    movq $0, %rcx #height counter

    #set divisor for checking odd/even
    movq $2, %rbx

    #acts like nested loop
    fill_height_loop:

        #check end condition
        cmpq %rcx, height
        je end_fill_loop

        #initialize nested loop counter
        movq $0, %rdx #width counter
        fill_width_loop:

            #retrieve random value from rand
            #save loop counters
            pushq %rcx
            pushq %rdx
            pushq %rax

            #get random number - return value in %rax
            call rand
            #divide by 2 and check for remainder
            divq %rbx

            #remainder is in %rdx
            #if the number is odd (rest = 1), the cell is alive
            cmpq $1, %rdx

            #restore loop counters and rax
            popq %rax
            popq %rdx
            popq %rcx

            #store registers for setcell
            pushq %rax
            pushq %rdi
            pushq %rsi
            pushq %rdx
            pushq %rcx

            #set input parameters for setCell that do not depend on alive/dead
            movq %rcx, %rdi
            movq %rdx, %rsi
            movq $CURR_GRID, %rdx

            #jump after restoring
            jne dead_cell

            #cell should be alive
            mov $42, %cl
            jmp setValue

            dead_cell:
            mov $32, %cl

            setValue:
                call setCell

            #restore registers
            popq %rcx
            popq %rdx
            popq %rsi
            popq %rdi
            popq %rax

            #go to next column
            incq %rdx
            cmpq %rdx, width
            jne fill_width_loop

        #go to next line
        incq %rcx
        jmp fill_height_loop

    end_fill_loop:
        #continue with playing the game

play:
    #initialize loop counter: 0..generations
    movq $0, %rcx

    play_loop:
        #save registers for printgrid
        pushq %rdi
        pushq %rsi
        pushq %rdx

        call printGrid

        #restore registers for printgrid
        popq %rdx
        popq %rsi
        popq %rdi

        #check end condition
        cmpq %rcx, generations
        je endSuccess


# purpose: print whole grid to stdout
# input: none
# output: none
# used registers:   RDI for filedescriptor of stdout
#                   RSI for content to print
#                   RDX for length of string
#                   R15 for loop counter
#                   R14 for index of row
#                   R13 for temporary storing height

.type printGrid,@function
printGrid:
    #prologue
    pushq %r15
    pushq %r14
    pushq %r13

    #init loop count to print every row
    movq $0, %r15
    movq height, %r13

    print_loop:
        cmpq %r15, height
        je end_print_loop

        pushq %r15
        imulq %r13, %r15
        movq %r15, %r14
        popq %r15

        movq $STDOUT, %rdi #write to stdout
        #movq CURR_GRID(%r14), %rsi     #start address of content
        #movq width, %rdx   #length of bytes to write
        movq $prefix, %rsi
        movq $prefix_len, %rdx
        movq $1, %rax
        syscall

        jmp print_loop

    end_print_loop:
        #epilogue
        popq %r13
        popq %r14
        popq %r15

        ret



# This function is so simple, we exceptionally skip prologue and epilogue
# Note: Division by two added, as otherwise the last bit will always exactly alternate,
# i.e. it is not random at all!
# Parameters: None
# Return value: new random number between 0 and 2^31-1
# Internal use: RCX (use for multiplication), RDX (destroyed by multiplication)
rand:
	movq rng,%rax
	movq $RNG_A,%rcx
	mulq %rcx               # Note: RDX destroyed here!
	addq $RNG_C,%rax
	shrq $1,%rax            # Shift right once = division by 2
	andq $0x7fffffff,%rax   # Limit to 31 Bits = modulo 2^31
	movq %rax,rng           # Store current value as new internal state
	ret


# Function to set a cell in a matrix to a specific value. Cells are one byte each.
# Uses the global variable width. No error checks for bounds!
# Parameters: RDI=row, RSI=column, RDX=base address of matrix, CL=value
# Return value: None
# note: the "grid" is actually one-dimensional
setCell:
	pushq %rbp
	movq %rsp,%rbp

    movq width,%rax     # multiplication: RDX:RAX = RAX*factor - RDX are upper 64 bits of result, and therefore, they must be saved beforehand
    pushq %rdx          # Temporarily store it; multiplication will destroy it but we still need it
    mulq %rdi           # rax = row * width
    popq %rdx           # Restore base address
    addq %rax,%rdx      # Calculate real address by adding number of bytes in preceding rows...
    addq %rsi,%rdx      # ...and the preceding columns in this row
    movb %cl,(%rdx)     # insert value at calculated spot

	movq %rbp,%rsp
	popq %rbp
	ret

# Function to get the value of a cell in a matrix. Cells are one byte each.
# Uses the global variable width. No error checks for bounds!
# Parameters: RDI=row, RSI=column, RDX=base address of matrix
# Return value: Content of byte at that position
getCell:
#... Write similar function to above here

# Get the number of alive neighbours a cell has. Note: E.g. the top left corner does not
# have a preceding row, so all three "above" cells cannot be checked (--> memory violation!).
# The same applies to the "left" cells. So this specific cell may only check three neighbors.
# Similar considerations apply to all border cells. For simplicity we check every single possible
# neighbour (8 in total) separately whether it exists, and only then check its state.
# Uses the global variable width. No error checks for bounds!
# Parameters: RDI=row, RSI=column, RDX=base address of matrix
# Return value: Number of neighbours of that cell (0...8)
# Internal use: ...
countNeighbours:
#... Write function here

endErrorArgNum:
    movq $ERR_PARAM_NUM, %rdi
    jmp endProgram

endIllegalValue:
    movq $ERR_ILLEGAL_VAL, %rdi
    jmp endProgram

endSuccess:
    movq $SUCCESS, %rdi

endProgram:
    movq $60, %rax
    syscall
