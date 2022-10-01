
# Two Pass Assembler
# About
### Implemented a two pass assembler and linking loader for SIC/XE machine architecture in C++.
# How to Run
Clone or Download the Zip folder<br/>
Run the following commands in the same directory(downloaded folder or in cloned folder).<br/>
c++ compiler should in installed.

# Steps Involved

1. Run this command to compile the file.
     ``` 
         $ g++ index.cpp
     ```

2. Run the executable file a.out using the following command.
   ``` 
    $ ./a.out <INPUT_FILE> <OUTPUT_FILE> <LISTING_FILE> 
    $ ./a.out input.txt output.txt listing.txt    
   ```
3. Provide the program addres in hexadecimal format 

4. Check the output, listing, memory.txt, memory_visualizer.txt files created in the same directory whose name is either equal to their default values
        output.txt and listing.txt or equal to the names given by you during the execution.

## Note:-
opcodes_file.txt contains the opcodes and their corresponding values. So if you want to add, delete or edit opcodes, you can directly edit this file.
memory.txt file contains the view of our memory where I have shown the values and their corresponding addresses i.e. all instruction's generated object codes and addresses are shown.
memory_visualizer.txt contains the actual view of our memory where data stored at all the addresses is shown.
Thus these files also takes into account the execution address related to our program.
