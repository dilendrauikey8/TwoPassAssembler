void loader_pass_2(string prog_addr, string input_filename, string output_filename, string loader_output_visualizer_filename)
{
    ifstream input_file(input_filename); // First open the required input file with read permission
    ofstream output_file("memory.txt");
    ofstream loader_output_visualizer_file("memory_visualizer.txt");
    if (!input_file.is_open())
    {
        cout << "Error opening " + input_filename << endl;
        exit(1);
    }
    string input_line;
    int EXEC_addr = stoi(prog_addr, NULL, 16), CS_addr = EXEC_addr, CS_len;
    string memory_val;

    while (input_file.peek() != EOF)
    {
        getline(input_file, input_line);
        vector<string> parsed_line = parse_line(input_line, 3); // Seperate all values in the line based on ^
        CS_len = stoi(parsed_line[3], NULL, 16);
        while (input_file.peek() != EOF)
        {
            getline(input_file, input_line);
            parsed_line = parse_line(input_line, 3);
            if (parsed_line[0] == "E") // Last entry of a particular section
                break;
            if (parsed_line[0] == "T") // Set memory at the specified location with the value given in the text record
            {
                int Start_addr = CS_addr + stoi(parsed_line[1], NULL, 16), size = parsed_line.size(), index = 0;
                for (int a = 3; a < size; a++)
                {
                    int code_size = parsed_line[a].size();
                    memory_mapper[Start_addr + index] = code_size;
                    for (int b = 0; b < code_size; b += 2) // Set
                    {
                        memory[Start_addr + index] = parsed_line[a].substr(b, 2);
                        index++;
                    }
                }
            }
            else
            {
                if (parsed_line[0] == "M") // Modification Record
                {
                    // Update memory location based on the values given in this modification record
                    if (ESTAB.find(trim(parsed_line[3].substr(1))) != ESTAB.end())
                    {
                        int symbol_val = ESTAB[trim(parsed_line[3].substr(1))], Start_addr = CS_addr + stoi(parsed_line[1], NULL, 16), len = stoi(parsed_line[2], NULL, 16);
                        string curr_val_str;
                        int i = 0;
                        char unchanged_val;
                        if (len % 2) // 05 case
                        {
                            unchanged_val = memory[Start_addr][0];
                        }
                        while (i < len) // Getting the value at this memory location
                        {
                            if (i == 0)
                            {
                                if (len % 2)
                                {
                                    curr_val_str = memory[Start_addr][1];
                                }
                                else
                                    curr_val_str += memory[Start_addr];
                            }
                            else
                            {
                                curr_val_str += memory[Start_addr + i / 2];
                            }
                            i += 2;
                        }
                        int curr_val = stoi(curr_val_str, NULL, 16);
                        if (parsed_line[3][0] == '+') // Add if + else subtract
                        {
                            curr_val += symbol_val;
                        }
                        else
                        {
                            curr_val -= symbol_val;
                        }
                        curr_val_str = transform_int_to_hex_string(curr_val, len);
                        i = 0;
                        int j = 0;
                        while (i < len) // Update part
                        {
                            if (i != 0)
                            {
                                memory[Start_addr + i / 2] = curr_val_str.substr(j, 2);
                            }
                            else
                            {
                                if (len % 2)
                                {
                                    memory[Start_addr][1] = curr_val_str[j];
                                    j--;
                                }
                                else
                                    memory[Start_addr + i / 2] = curr_val_str.substr(j, 2);
                            }
                            j += 2;
                            i += 2;
                        }
                    }
                    else
                    {
                        cout << parsed_line[3].substr(1) << endl;
                        cout << "ERROR: UNDEFINED EXTERNAL SYMBOL" << endl;
                        exit(1);
                    }
                }
            }
        }
        if (parsed_line.size() > 1) // If END Statement specified the execution address
        {
            EXEC_addr = stoi(parsed_line[1], NULL, 16);
        }
        CS_addr += CS_len;
    }
    cout << "EXECUTION ADDRESS: " << transform_int_to_string(prog_addr, 4) << endl;
    loader_output_visualizer_file << "Address\t\tValue" << endl;
    int load_size = ((CS_addr + 15) / 16) * 16, start_addr = ((stoi(prog_addr, NULL, 16))/16)*16;
    for (int i = start_addr; i < load_size; i += 16) // Printing memory which has been updated in the block of 16
    {
        memory_val = transform_int_to_hex_string(i, 4);
        for (int a = 0; a < 16; a += 4)
        {
            if (a)
                memory_val += " ";
            else // All object codes and their corresponding addresses in the memory is shown using it
                memory_val += "\t\t";
            for (int b = 0; b < 4; b++)
            {
                memory_val += memory[i + a + b];
            }
        }
        memory_val += '\n';
        loader_output_visualizer_file << memory_val;
    }
    output_file << "Address\t\tValue" << endl;
    for (auto it : memory_mapper)
    {
        memory_val = transform_int_to_hex_string(it.first, 4) + "\t\t";
        for (int a = it.first; a < it.first + (it.second) / 2; a++) // Visual representation printing of the memory
        {
            memory_val += memory[a];
        }
        memory_val += '\n';
        output_file << memory_val;
    }
    input_file.close();
    output_file.close();
    loader_output_visualizer_file.close();
}