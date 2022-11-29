###############################################################################

README

Assignment: um
Authors: Danesia Patterson and Helina Mesfin
Date: November 22, 2022

###############################################################################

Acknowledgements
----------------
We consulted Marks lectures and the textbook often. Hanson also helped when it 
came to learning how to utilize his implementation of Sequences. We also used 
alot of Piazza questions and answers to guide us. We got help from TAs Mark
and Mary.

What has been correctly implemented
-----------------------------------
We were able to successfully implement all aspects of the assignment:
this includes the working Universal Machine (all instructions), the memory 
segments and its ability to run within specified time limits.


Significant departures from design
-----------------------------------
In our design we initially did not include how we were going to implement
our unmapped segment ids. We only had a seq_t for our mapped ids, which we 
realized was flawed. We changed that while programming by also creating a 
sequence for our unmapped segment ids according to the spec.  

Our Architecture
-----------------------------------

UM Module - 

Our UM Module contains all of the functions that directly interact with
the UM itself, which includes its intialization, its run function, an execute 
command function, and all of the instructions that can be called on the UM. It 
also includes the struct definiton of our um_T, which includes an instance of 
our mem_T struct (explained in memory_seg module below),the UM's 8 registers, a 
program counter, and 3 unsigned variables to hold for register a, b and c. Our
UM module does not know the secrets of how our memory module functions-- it 
just calls the functions as needed but how these functions work is unknown. Our 
UM module also utilizes our I/O module by calling functions from the module 
within our two input and output instructions within the UM, but not showing 
what goes on under the hood. It does interact with the um_driver module, which 
calls the run_um function.

Memory_segment Module - 

Our memory_segment module contains all functions that 
directly interact with behaviors affecting the memory segments. It includes a
struct definiton of mem_T, which holds two sequences: one for our mapped seg
ids and the other for our unmapped. We also have a struct definition of
seg_T, which includes *segments, an array of words within a segment, and 
segment_length, which provides us with the length of the specific segment.
This module includes all the functions needed to manipulate each memory segment,
including creating our struct instances, freeing the memory, mapping, 
unmapping, and the instructions within the UM that alter the memory segments. 
This module does not know how how the UM module works, nor does it know the I/O 
module.

I/O Module -

Our I/o module contains our two functions that work with input and output 
instructions. This module interacts with our UM, as these functions are 
used in the input_command and output_command. This module does not know/
interact with the memory_segment module, nor does it interact with the
driver module. 


Um_driver Module - 

Our um_driver module is our driver module that runs our UM Module. It reads
in a file, creating the sequence of instructions and then calls our run_um
function from our UM module. It does not know the secrets of the UM module,
it only has access to the run function. 


Time it takes to execute 50 million instructions
-----------------------------------
About 4 seconds, we know this because our UM takes about 6 seconds to execute
the 80 million instructions inside midmark.um

Our unit tests
-----------------------------------
cmov.um - tests conditional move in the case where the condition is false
by calling loading 0 into register C and calling conditional move.

cmov_true.um - tests conditional move in the case where the condition is true
by calling conditional move.

load.um - tests segment_load (and implicitly segment_store) by loading the 
ASCII value "6", mapping a segment, storing the loaded value, loading the
value after storage, and outputting the value

store.um - tests segment_store by loading the ASCII value "6", mapping a 
segment, and storing the loaded value

add.um - tests add by calling add on three registers

multiply.um - tests multiply by loading values into registers, multiplying 
them, and outputting the result which was ASCII value "A"

divide.um - tests divide by loading values into registers, diving them, and
outputting the result which was ASCII value "#"

nand.um - tests nand by calling nand on three registers

halt.um - tests halt by calling the halt instruction

map.um - tests map by calling the map instruction on three registers
 
unmap.um - tests unmap (and implicitly map) by mapping a segment and then 
unmapping it

loop_mapping.um - tests map and unmapping by calling map a large amount of 
times, unmapping a segment, and then mapping another segment

output.um - tests output (and implicitly load_value) by loading the ASCII 
value "A" and then outputting it

input.um - tests input by calling the input value

io.um - tests input and output together by calling input and then output on the
value from input

math.um - tests all the math operations together by loading values, doing
operations on them, and then outputting the values from each step

load_zero.um - tests the load program operation when the program to be 
loaded is 0 by calling load_program on the program and a segment within
the program

Time Spent
-----------------------------------

Analyzing the assignment - 2 hours

Preparing the design - 6 hours

Solving the problems after analysis - 15 hours
