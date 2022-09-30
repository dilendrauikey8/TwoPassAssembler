int pass_1(string input_filename, string intermediate_filename)
{
    fstream input_file, intermediate_file;
    // ifstream input_file(input_filename); // First open the required input and intermediate files with read and write permissions respectively
    // ofstream intermediate_file(intermediate_filename);
    input_file.open("input.txt", ios::in);
    intermediate_file.open("intermediate.txt", ios::out);
    if (!input_file.is_open())
    {
        cout << "Error opening " + input_filename << endl;
        exit(1);
    }
    if (!intermediate_file.is_open()) // If an error occurs while opening these files, we will report them and then exit the program
    {
        cout << "Error opening " + intermediate_filename << endl;
        exit(1);
    }
    string input_line, CSECT;
    int LOCCTR = 0, initial_addr = 0, prev_addr; // Declaration of LOCCTR and other variables to keep track of recent addresses.
    bool error_bit = false;                      // Will keep record that whether an error occured during this function or not
    while (input_file.peek() != EOF)             // If a line still exists inside the input file which can be parsed
    {
        getline(input_file, input_line);
        prev_addr = LOCCTR;
        vector<string> parsed_line = parse_line(input_line, 1); // Will parse the input line and will create a vector containing the label, opcode and operand
        if (parsed_line[0] != ".")                              // If the line is not a comment
        {
            if (parsed_line[1] == "START" || parsed_line[1] == "CSECT") // If it is a START or CSECT statement
            {
                Curr_EXTREFTAB.clear();
                if (CSECT.size())
                // Will enter the previous section information if it exists.
                {
                    SYMTAB[{CSECT, CSECT}] = {initial_addr, LOCCTR - initial_addr};
                }
                initial_addr = 0; // Reset the address to point to zero for this section
                if (parsed_line.size() >= 3 && parsed_line[2] != "")
                    initial_addr = stoi(parsed_line[2], 0, 16); // Will store the initial address given in the operand in hexadecimal format

                CSECT = parsed_line[0];
                LOCCTR = initial_addr; // Set the LOCCTR to 0 and also set the name of the new CSection
                prev_addr = initial_addr;
                add_register_values(CSECT);

                stringstream sstream;
                sstream << hex << prev_addr;
                intermediate_file << transform_int_to_string(sstream.str(), 4) << "\t" << input_line << endl;
                continue;
            }
            else if (parsed_line[0] != "") // If there is a label present in the line
            {
                if (SYMTAB.find({CSECT, parsed_line[0]}) != SYMTAB.end()) // If the label is already present in the Symbol table
                {                                                         // then showing the duplicate symbol error
                    error_bit = true;
                    cout << "Error: DUPLICATE SYMBOL" << endl;
                    exit(1);
                }
                if (parsed_line[1] == "EQU")
                {
                    int equ_addr = LOCCTR;
                    if (parsed_line[2] == "*")
                    {
                        SYMTAB[{CSECT, parsed_line[0]}] = {LOCCTR, 3}; // If given *, then this means that we have to store the current LOCCTR as the value
                    }
                    else
                    {
                        // Else we will evaluate the expression using simplify_expr function
                        pair<int, vector<string>> result = simplify_expr(CSECT, parsed_line[2], int_to_hex(LOCCTR), Curr_EXTREFTAB);
                        equ_addr = result.first; // It will be the value returned by our function for the given expression.
                        mask_val(equ_addr, 16);  // We will mask the result if it is negative.
                    }
                    intermediate_file << transform_int_to_hex_string(equ_addr, 4) << "\t" << input_line << endl;
                    continue;
                }
                else
                {
                    // We will store the address of this label in the SYMTAB
                    SYMTAB[{CSECT, parsed_line[0]}] = {LOCCTR, 3};
                }
            }

            if (parsed_line.size() >= 3 && parsed_line[2].find('=') != string::npos && LITTAB.find(parsed_line[2]) == LITTAB.end())
            {
                // If no entry of this Literal is present in the LITTAB then add it in the table.
                LITTAB[parsed_line[2]] = {-1, -1};
            }
            if (OPTAB.find(parsed_line[1]) != OPTAB.end() || parsed_line[1] == "WORD")
            {
                int length_to_add = 3; // If it is a WORD then we are setting the length as 3.
                if (OPTAB.find(parsed_line[1]) != OPTAB.end())
                { // Else we will set the length equal to the length of that operand.
                    length_to_add = OPTAB[parsed_line[1]].second;
                }
                LOCCTR += length_to_add; // Now we will increment the value of LOCCTR depending upon the opcode value or if it is a WORD
            }
            else
            {
                if (parsed_line[1] == "RESW")
                {
                    LOCCTR += 3 * stoi(parsed_line[2]); // Will reserve words and parsed_line[2] = Operand which will give us the number of words to reserve
                }
                else
                {
                    if (parsed_line[1] == "RESB")
                    {
                        LOCCTR += stoi(parsed_line[2]); // Similarly will reserve bytes
                    }
                    else
                    {
                        if (parsed_line[1] == "BYTE")
                        {
                            if (parsed_line[2][0] == 'X') // If it is a hexadecimal number
                            {
                                LOCCTR += (parsed_line[2].size() - 3) / 2; // Substracting 3 because the format will be X'<VAL>' so we will remove X,' and ' to get VAL.
                            }
                            else // If it is a character
                            {

                                LOCCTR += parsed_line[2].size() - 3; // A character takes a byte while two hexadecimal digits form a byte
                            }
                        }
                        else
                        {
                            if (parsed_line[1] == "EXTDEF" || parsed_line[1] == "BASE" || parsed_line[1] == "EXTREF")
                            {
                                intermediate_file << "\t\t" << input_line << endl;
                                if (parsed_line[1] == "EXTREF")
                                {
                                    vector<string> operands = split_str(parsed_line[2]);
                                    for (auto it : operands)
                                    {
                                        Curr_EXTREFTAB.push_back(it);
                                    }
                                }
                                continue;
                            }
                            else
                            {
                                if (parsed_line[1] == "END" || parsed_line[1] == "LTORG") // If it is an END/LTORG statement
                                {
                                    intermediate_file << "\t\t" << input_line << endl;
                                    for (auto &it : LITTAB) // When encountering LITTAB set all literals found which were not set till now.
                                    {
                                        if (it.second[1] == -1) // Unset literal
                                        {
                                            intermediate_file << transform_int_to_hex_string(prev_addr, 4) << "\t" << transform_string("*", 1) << "\t\t  ="
                                                              << it.first.substr(1) << endl;
                                            it.second = {LOCCTR, 3};
                                            if (it.first[1] == 'X') // If it is a hexadecimal val
                                            {

                                                LOCCTR += (it.first.size() - 4) / 2;
                                            }
                                            else if (it.first[1] == 'C') // If it is a character
                                            {
                                                LOCCTR += (it.first.size() - 4);
                                            }
                                            else // Else simply add 3
                                                LOCCTR += 3;
                                        }
                                    }
                                    prev_addr = LOCCTR;
                                    continue;
                                }
                                else
                                {
                                    if (parsed_line[1][0] == '+' && OPTAB.find(parsed_line[1].substr(1)) != OPTAB.end())
                                    { // If extended mode is used in the instruction
                                        LOCCTR += OPTAB[parsed_line[1].substr(1)].second + 1;
                                    }
                                    else
                                    { // Else this means that it is an invalid opcode
                                        error_bit = true;
                                        cout << parsed_line[1] << endl;
                                        cout << "Error: INVALID OPERATION CODE" << endl;
                                        exit(1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            stringstream sstream;
            sstream << hex << prev_addr;
            intermediate_file << transform_int_to_string(sstream.str(), 4) << "\t" << input_line << endl;
        }
        else
            intermediate_file << "\t\t" << input_line << endl; // If comment then simply print it in the intermediate file
    }

    if (CSECT.size())
    {
        SYMTAB[{CSECT, CSECT}] = {initial_addr, LOCCTR - initial_addr}; // For the last CSection
    }
    Curr_EXTREFTAB.clear();
    input_file.close(); // Finally close all the opened files
    intermediate_file.close();
    return LOCCTR - initial_addr; // and return the program length
}
