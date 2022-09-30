void pass_2(int program_length, string intermediate_filename, string output_filename, string listing_filename)
{
    ifstream intermediate_file(intermediate_filename); // First we will open the intermediate, output and listing files
    ofstream output_file(output_filename), listing_file(listing_filename);
    string input_line, prog_name;
    int initial_addr, prog_len = 0, BASE_val;               // Setting the variables required while operating with the addresses, program length and base register value
    bool error_bit = false;                                 // Will be used to detect whether an error occured during this pass
    getline(intermediate_file, input_line);                 // For parsing the first line i.e. START statement
    vector<string> parsed_line = parse_line(input_line, 2); // Here parsed_line is a vector containing the address, label, opcode and operand
    if (parsed_line[2] == "START")
    {
        prog_name = parsed_line[1];
        initial_addr = stoi(parsed_line[3], NULL, 16);

        output_file << "H^" << transform_string(parsed_line[1], 6) << "^" << transform_int_to_string(parsed_line[3], 6) << "^" << transform_int_to_hex_string(SYMTAB[{prog_name, prog_name}][1], 6) << endl;
        listing_file << input_line << endl;
    }
    else
    {
        cout << "ERROR: Wrong input format" << endl;
        exit(1);
    }

    // We are adding "^" in between the elements to mark the end of the previous element.
    // NOTE: These caret symbols will only be visible in the output file and not the listing file

    string curr_text_record, object_code, end_addr, curr_record_addr;
    bool first_sect = true;
    int curr_text_record_len = 0;           // Variables which will keep records of addresses and current text records
    while (intermediate_file.peek() != EOF) // Will check whether there exists a line inside the intermediate file which can be parsed
    {
        object_code = "";
        getline(intermediate_file, input_line);

        vector<string> parsed_line = parse_line(input_line, 2);
        if (parsed_line[0] == "." || parsed_line[0] == "LTORG") // If this statement is a comment or LTORG statement then just list it in the listing file and continue parsing other lines
        {
            listing_file << input_line << endl;
            continue;
        }
        if (parsed_line[3] == "CSECT")
        {
            prog_name = parsed_line[2];                   // Set the prog_name equal to the new section's name
            prog_len = SYMTAB[{prog_name, prog_name}][1]; // Get the length of the same from SYMTAB

            if (curr_text_record_len) // If previously a text record existed then list it in the output file
            {
                output_file << "T^" << transform_int_to_string(curr_record_addr, 6) << "^" << transform_int_to_hex_string(curr_text_record_len / 2, 2) << curr_text_record << endl;
                curr_text_record = "";
                curr_text_record_len = 0; // Then set the current text record as empty
            }

            int size = Modify_records.size();
            for (int i = 0; i < size; i++) // After setting the previous text records, set all the modifiy records statements for the previous section.
            {
                output_file << Modify_records[i] << endl;
            }
            Modify_records.clear(); // Clear the modifiy records of the previous section

            output_file << "E";
            if (first_sect) // If it is the first section of the program set the initial address too.
            {
                output_file << "^" << transform_int_to_string(to_string(initial_addr), 6);
                first_sect = false;
            }
            output_file << '\n';

            initial_addr = 0;
            Curr_EXTREFTAB.clear(); // Clear the current extref table to clear out all previous section's values.

            output_file << "H^" << transform_string(parsed_line[2], 6) << "^" << transform_int_to_hex_string(0, 6) << "^";
            output_file << transform_int_to_hex_string(prog_len, 6) << endl;
        }
        if (parsed_line[0] == "EXTDEF" || parsed_line[0] == "EXTREF" || parsed_line[0] == "BASE")
        {
            if (parsed_line[0] == "BASE")
            {
                if (SYMTAB.find({prog_name, parsed_line[3]}) != SYMTAB.end()) // If it is a symbol
                {
                    BASE_val = SYMTAB[{prog_name, parsed_line[3]}][0];
                }
                else
                {
                    BASE_val = stoi(parsed_line[3], NULL, 16); // Else it will be a value
                }
            }
            else
            {
                if (parsed_line[0] == "EXTREF")
                {
                    vector<string> operands = split_str(parsed_line[2]); // Find all the references and store in the operands vector

                    output_file << "R";
                    for (auto it : operands)
                    {
                        Curr_EXTREFTAB.push_back(it); // Add these references into extref table
                        output_file << "^" << transform_string(it, 6);
                    }
                    output_file << "\n";
                }
                else
                { // It will be a EXTDEF statement

                    output_file << "D";
                    vector<string> operands = split_str(parsed_line[2]); // Find all the references and store in the operands vector
                    for (auto it : operands)
                    {
                        if (SYMTAB.find({prog_name, it}) != SYMTAB.end())
                        {
                            output_file << "^" << transform_string(it, 6) << "^" << transform_int_to_hex_string(SYMTAB[{prog_name, it}][0], 6);
                        }
                        else
                        {
                            cout << "SYMBOL Not defined yet!" << endl;
                            exit(1);
                        }
                    }
                    output_file << endl;
                }
            }
            listing_file << input_line << endl;
            continue;
        }
        else
        {
            if (OPTAB.find(parsed_line[2]) != OPTAB.end() || (parsed_line[2].size() > 1 && OPTAB.find(parsed_line[2].substr(1)) != OPTAB.end())) // Covers both non extended and extended formats
            {
                if (parsed_line[2] == "COMPR" || parsed_line[2] == "CLEAR" || parsed_line[2] == "TIXR") // 2 - byte format
                {
                    int operands_val = 0;
                    string object_code = OPTAB[parsed_line[2]].first, second_reg = "0";
                    vector<string> registers_used = split_str(parsed_line[3]); // Find all the registers used and store in the registers_used vector
                    if (parsed_line[2] == "COMPR")
                    {
                        if (registers_used.size() != 2)
                        {
                            cout << "ERROR: Invalid Opcode" << endl;
                            exit(1);
                        }
                    }
                    else
                    {
                        if (registers_used.size() != 1)
                        {
                            cout << "ERROR: Invalid Opcode" << endl;
                            exit(1);
                        }
                    }

                    object_code += to_string(SYMTAB[{prog_name, registers_used[0]}][0]);
                    if (parsed_line[2] == "COMPR") // Among three of them, COMPR is the only which uses two registers for comparision
                    {
                        second_reg = to_string(SYMTAB[{prog_name, registers_used[1]}][0]);
                    } // Else we will second register as 0
                    object_code += second_reg;

                    listing_file << input_line << "\t" << object_code << endl;
                    if (curr_text_record_len + object_code.size() <= 60) // If current text record can accommodate a new entry
                    {
                        curr_text_record += "^" + object_code;
                        curr_text_record_len += object_code.size();
                    }
                    else
                    {
                        if (curr_text_record_len) // If this record contains some values
                        {
                            output_file << "T^" << transform_int_to_string(curr_record_addr, 6) << "^" << transform_int_to_hex_string(curr_text_record_len / 2, 2) << curr_text_record << endl;
                            if (object_code.size())
                                curr_text_record = "^" + object_code;
                            else
                                curr_text_record = "";
                            curr_text_record_len = object_code.size();
                            curr_record_addr = parsed_line[0]; // Set the text record's data equal to the object code (For the statements whose opcodes are not applicable during this pass like RESB RESW,, the object code will be an empty string)
                        }
                    }
                }
                else
                {
                    if (parsed_line[2].find('+') == string::npos) // 3 - byte format
                    {
                        int object_code_in_dec = hex_to_int(OPTAB[parsed_line[2]].first) * (1 << 16), PC_Counter = hex_to_int(parsed_line[0]) + 3;
                        if (parsed_line[3] != "")
                        {
                            if (parsed_line[3][0] == '#')
                            {
                                object_code_in_dec |= (1 << 16);
                                parsed_line[3] = parsed_line[3].substr(1);
                            }
                            else
                            {
                                if (parsed_line[3][0] == '@')
                                {
                                    object_code_in_dec |= (1 << 17);
                                    parsed_line[3] = parsed_line[3].substr(1);
                                }
                                else
                                {
                                    object_code_in_dec |= (1 << 16) + (1 << 17);
                                }
                            }
                            if (parsed_line[3].find(',') != string::npos)
                            {
                                object_code_in_dec |= (1 << 15);
                                parsed_line[3] = parsed_line[3].substr(0, parsed_line[3].find(','));
                            }
                        }
                        else
                        {
                            object_code_in_dec |= (1 << 16) + (1 << 17);
                        }
                        if (parsed_line[3] != "") // If operand exists
                        {
                            if (check_int(parsed_line[3])) // If it is an integer
                            {
                                object_code_in_dec += stoi(parsed_line[3]);
                            }
                            else
                            {
                                pair<int, vector<string>> simplified_expr = simplify_expr(prog_name, parsed_line[3], transform_int_to_hex_string(hex_to_int(parsed_line[0]), 6), Curr_EXTREFTAB);
                                for (auto it : simplified_expr.second)
                                {
                                    Modify_records.push_back(it); // All the references that need modified in this expression will be added into Modify_records
                                }
                                int Sym_val = simplified_expr.first;    // Value of the evaluated expression

                                if (Sym_val - PC_Counter <= 2047 || Sym_val - PC_Counter >= -2048) // Using PC for addressing
                                {
                                    int PC_Disp = Sym_val - PC_Counter;
                                    if (PC_Disp < 0)
                                    {
                                        PC_Disp = -PC_Disp;
                                        PC_Disp = (((~PC_Disp) & (0xFFF)) + 1) & (0xFFF);
                                    }
                                    object_code_in_dec += PC_Disp;
                                    object_code_in_dec |= (1 << 13);
                                }
                                else
                                {
                                    if (Sym_val - BASE_val >= 0 && Sym_val - BASE_val < 4096) // Base register addressing
                                    {
                                        object_code_in_dec += (Sym_val - BASE_val);
                                        object_code_in_dec |= (1 << 14);
                                    }
                                    else
                                    { // None of them possible then show error
                                        cout << "Can't fit using PC/Base." << endl;
                                        exit(1);
                                    }
                                }
                            }
                        }
                        stringstream sstream;
                        sstream << hex << object_code_in_dec;
                        string object_code = sstream.str();
                        object_code = transform_int_to_string(object_code, 6);
                        listing_file << input_line << "\t" << object_code << endl;

                        if (object_code.size() + curr_text_record_len <= 60) // If current text record can accommodate this object code
                        {
                            curr_text_record += "^" + object_code;
                            curr_text_record_len += object_code.size();
                        }
                        else
                        {
                            if (curr_text_record_len) // If this record contains some values
                            {
                                output_file << "T^" << transform_int_to_string(curr_record_addr, 6) << "^" << transform_int_to_hex_string(curr_text_record_len / 2, 2) << curr_text_record << endl;
                                if (object_code.size())
                                    curr_text_record = "^" + object_code;
                                else
                                    curr_text_record = "";
                                curr_text_record_len = object_code.size();
                                curr_record_addr = parsed_line[0]; // Set the text record's data equal to the object code (For the statements whose opcodes are not applicable during this pass like RESB RESW,, the object code will be an empty string)
                            }
                        }
                    }
                    else if (parsed_line[2].find('+') != string::npos) // 4 - byte format
                    {
                        parsed_line[2] = parsed_line[2].substr(1); // Remove the + sign from the value
                        int object_code_in_dec = hex_to_int(OPTAB[parsed_line[2]].first) * (1 << 24);
                        object_code_in_dec |= (1 << 20); // Also set the extended bit

                        if (parsed_line[3][0] == '#')
                        {
                            object_code_in_dec |= (1 << 24);
                            parsed_line[3] = parsed_line[3].substr(1);
                        }
                        else
                        {
                            if (parsed_line[3][0] == '@')
                            {
                                object_code_in_dec |= 1 << 25;
                                parsed_line[3] = parsed_line[3].substr(1);
                            }
                            else
                            {
                                object_code_in_dec |= (1 << 24) + (1 << 25);
                            }
                        }
                        if (parsed_line[3].find(',') != string::npos)
                        {
                            object_code_in_dec |= (1 << 23);
                            parsed_line[3] = parsed_line[3].substr(0, parsed_line[3].find(','));
                        }
                        pair<int, vector<string>> simplified_expr = simplify_expr(prog_name, parsed_line[3], transform_int_to_hex_string(hex_to_int(parsed_line[0]) + 1, 6), Curr_EXTREFTAB, "05");
                        for (auto it : simplified_expr.second)
                        {
                            Modify_records.push_back(it); // All the references that need modified in this expression will be added into Modify_records
                        }
                        object_code_in_dec += simplified_expr.first;

                        object_code = transform_int_to_string(int_to_hex(object_code_in_dec), 8);
                        listing_file << input_line << "\t" << object_code << endl;
                        if (object_code.size() + curr_text_record_len <= 60)
                        {
                            curr_text_record += "^" + object_code;
                            curr_text_record_len += object_code.size();
                        }
                        else
                        {
                            if (curr_text_record_len) // If this record contains some values
                            {
                                output_file << "T^" << transform_int_to_string(curr_record_addr, 6) << "^" << transform_int_to_hex_string(curr_text_record_len / 2, 2) << curr_text_record << endl;
                                if (object_code.size())
                                    curr_text_record = "^" + object_code;
                                else
                                    curr_text_record = "";
                                curr_text_record_len = object_code.size();
                                curr_record_addr = parsed_line[0]; // Set the text record's data equal to the object code (For the statements whose opcodes are not applicable during this pass like RESB RESW,, the object code will be an empty string)
                            }
                        }
                    }
                }
            }
            else
            {
                if (parsed_line[2] == "END") // If it is an END statement then store the address corresponding to the Symbol which is passed as an operand
                {

                    stringstream sstream;
                    sstream << hex << initial_addr;
                    end_addr = sstream.str();
                    listing_file << input_line << endl;
                    continue;
                }
                else
                {
                    string object_code;
                    if (parsed_line[2] == "WORD")
                    {
                        pair<int, vector<string>> simplified_expr = simplify_expr(prog_name, parsed_line[3], parsed_line[0], Curr_EXTREFTAB);
                        object_code += transform_int_to_string(to_string(simplified_expr.first), 6); // The word statements will be stored as a standalone object code
                        for (auto it : simplified_expr.second)
                        {
                            Modify_records.push_back(it); // All the references that need modified in this expression will be added into Modify_records
                        }
                    }
                    else if (parsed_line[2] == "BYTE" || parsed_line[2] == "*")
                    {
                        if (parsed_line[2] == "*")
                        {
                            parsed_line[3] = parsed_line[3].substr(1);
                        }
                        if (parsed_line[3][0] == 'X') // If it is an hexadecimal number then it will be stored as it is.
                        {
                            parsed_line[3] = parsed_line[3].substr(2, parsed_line[3].size() - 3);
                            object_code += parsed_line[3];
                        }
                        else
                        {
                            // If it is a character then it will be stored in hexadecimal format
                            // As each character is of 1 byte, therefore it is comprised of 2 hexadecimal characters
                            parsed_line[3] = parsed_line[3].substr(2, parsed_line[3].size() - 3);
                            for (int i = 0; i < parsed_line[3].size(); i++)
                            {
                                object_code += transform_int_to_hex_string(parsed_line[3][i], 2);
                            }
                        }
                    }
                    if (parsed_line[2] != "*")
                        listing_file << input_line << '\t' << object_code << endl; // Finally I am formatting the string to be of length 6 by padding extra 0s in front of it
                    else
                    {
                        listing_file << input_line;
                        for (int i = 0; i < 39; i++)
                            listing_file << ' ';
                        listing_file << object_code << endl;
                    }
                    // Here we will do the things required for the output file
                    if (curr_text_record_len + object_code.size() <= 60 && parsed_line[2] != "RESW" && parsed_line[2] != "RESB") // If the current length of text record + size of the object code
                    {                                                                                                            // is <=60 (69 - 10 +1) and also it is not a RESW or RESB statement
                                                                                                                                 // When these statements are encountered, we will create a new text record and will store the current record
                        if (curr_text_record_len == 0)                                                                           // If no record has been added inside it then we will set the start address as the address of the current instruction
                        {
                            curr_record_addr = parsed_line[0];
                        }
                        if (object_code.size()) // If the opcodes is among the acceptable opcodes for this pass, then only it's length is not 0
                            curr_text_record += "^" + object_code;
                        curr_text_record_len += object_code.size(); // Finally increment the length of this text record by the length of the object code
                    }
                    else
                    {
                        if (curr_text_record_len) // If this record contains some values
                        {
                            output_file << "T^" << transform_int_to_string(curr_record_addr, 6) << "^" << transform_int_to_hex_string(curr_text_record_len / 2, 2) << curr_text_record << endl;
                            if (object_code.size())
                                curr_text_record = "^" + object_code;
                            else
                                curr_text_record = "";
                            curr_text_record_len = object_code.size();
                            curr_record_addr = parsed_line[0]; // Set the text record's data equal to the object code (For the statements whose opcodes are not applicable during this pass like RESB RESW,, the object code will be an empty string)
                        }
                    }
                }
            }
        }
    }
    if (curr_text_record_len)
    {
        output_file << "T^" << transform_int_to_string(curr_record_addr, 6) << "^" << transform_int_to_hex_string(curr_text_record_len / 2, 2) << curr_text_record << endl;
    }
    for (auto it : Modify_records)
    {
        output_file << it << endl;
    }
    Modify_records.clear();
    if (first_sect) // If it is the first section of the program
    {
        output_file << "E^" << transform_int_to_string(end_addr, 6); // Finally store the END statement in the respective format in both output and listing files
    }
    else
    {
        output_file << "E\n";
    }

    intermediate_file.close();
    output_file.close(); // Close all the files opened during this function i.e. intermediate, output and listing files
    listing_file.close();
}
