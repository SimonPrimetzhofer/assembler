	# PURPOSE:  This program converts a number into a hexadecimal string.
	#
	# VARIABLES: The registers have the following uses:
	#
	# RAX: Initially the number and later on result of division
	# RBX: Divisor (16)
	# RCX: Loop counter
	# RDX: Rest of division
	# SIL: HEX-Value of the divisions rest from the table
	# RSI: 64 bit register of SIL for adding the ascii values
	# RDI: Return code (sum of ascii values)
	#
	#
	# The following memory locations are used:
	#
	# number     - contains the number to convert to hex
	# hex_number - contains the converted number
	# table      - helper table for conversion to hexadecimal
	#

.section .data
number:		.quad 0xf12345A7
hex_number:	.asciz "________" # eight underscores to make sure there is enough space for four bytes
table:		.ascii "0123456789ABCDEF" #16bit

.section .text
.globl _start

_start:
    #move number into %rax as dividend (lower 64 bits; upper 64 bits would go to rdx)
    movq number, %rax

    #initialize loop counter with 7 (so 8 steps are done later)
    movq $7, %rcx

    #set divisor to 16
    movq $16, %rbx

conversion_loop:
    #Divide by 16
    #Rest in Hexadecimal
    #Repeat until result is 0
    #Rest from last to first is result

    #divide number by 16
    #result in RAX - rest in RDX
    divq %rbx

    #Get hexvalue from table and store it into 8-bit register SIL
    mov table(,%rdx,1), %sil
    #Insert hexvalue into result
    mov %sil, hex_number(,%rcx,1)

    #Add ascii value to sum
    addq %rsi, %rdi

    #clear rdx, since the division in future iterations would not work correctly (rdx are upper 64 bits and we don't want this)
    movq $0, %rdx

    #decrement loop counter
    decq %rcx

    #continue loop until the number in rax is 0
    cmpq $0, %rax
    jne conversion_loop

loop_exit:
    movq $60, %rax
    syscall


