#include "utility.h"
#include "assembler/pass1.h"
#include "assembler/pass2.h"
#include "linker_loader/pass1.h"
#include "linker_loader/pass2.h"

int main(int argc, char *argv[])
{
        if (argc < 2) //  User will provide necessary informations required for the assembler like which input file to convert
        {             //  and what should be the name of the outfile file produced.
                cout << "Wrong format" << endl
                     << "USAGE: <INPUT_FILE_NAME> <OUTPUT_FILE_NAME> <LISTING_FILE_NAME>" << endl
                     << "OPTIONAL: <OUTPUT_FILE_NAME>, <LISTING_FILE_NAME>" << endl;
                return 0;
        }

        string intermediate_filename, output_filename, listing_filename, loader_output_filename, loader_output_visualizer_filename, prog_addr; // Will store the names of the files which will be opened.

        OPTAB_filler();                                                                                                                                 // Using OPTAB_filler(), we will first create the OPCodes Table
        set_variables(intermediate_filename, output_filename, listing_filename, loader_output_filename, loader_output_visualizer_filename, argc, argv); // Here I am setting the variables equal to the values given at the start
        // cout << intermediate_filename << " " << output_filename << " " << listing_filename << " " << loader_output_filename << " " << loader_output_visualizer_filename << endl;
        // return 0;
        int program_length = pass_1(argv[1], intermediate_filename);
        // cout<<program_length<<endl;
        // return 0;
        // As we are implementing 2 pass assembler, therefore this function will perform the pass1 and will return the program length
        pass_2(program_length, intermediate_filename, output_filename, listing_filename); // Pass 2 of the assembler

        cout << "Enter the program address in hexadecimal format... ";
        cin >> prog_addr;
        loader_pass_1(output_filename, prog_addr);                                                            // Pass 1 of the linker loader
        loader_pass_2(prog_addr, output_filename, loader_output_filename, loader_output_visualizer_filename); // Pass 2 of the linker loader

        return 0;
}