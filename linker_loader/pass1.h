int loader_pass_1(string input_filename, string prog_addr)
{
    ifstream input_file(input_filename); // First open the required input file with read permission
    if (!input_file.is_open())
    {
        cout << "Error opening " + input_filename << endl;
        exit(1);
    }
    string input_line;
    int Progaddr = stoi(prog_addr, NULL, 16), CSaddr = Progaddr, CS_len;
    while (input_file.peek() != EOF)
    {
        getline(input_file, input_line);
        vector<string> parsed_line = parse_line(input_line, 3); // Seperate all values in the line based on ^
        CS_len = stoi(parsed_line[3], NULL, 16);
        if (ESTAB.find(trim(parsed_line[1])) == ESTAB.end())    // If it is not present in the table then add the entry
        {
            ESTAB[trim(parsed_line[1])] = CSaddr;
        }
        else
        {   // Else show error 
            cout << "ERROR: DUPLICATE EXTERNAL SYMBOL" << endl;
            exit(1);
        }
        while (input_file.peek() != EOF)
        {
            getline(input_file, input_line);
            parsed_line = parse_line(input_line, 3);
            if (parsed_line[0] == "E")  // Last statement for a particular Section
            {
                break;
            }
            if (parsed_line[0] == "D")
            {
                int size = parsed_line.size();  
                for (int i = 1; i < size; i += 2)
                {
                    if (ESTAB.find(trim(parsed_line[i])) == ESTAB.end())    // Add all the references that we need to define for global use
                    {
                        ESTAB[trim(parsed_line[i])] = stoi(parsed_line[i + 1], NULL, 16) + CSaddr;
                    }
                    else
                    {
                        cout << "ERROR: DUPLICATE EXTERNAL SYMBOL" << endl;
                        exit(1);
                    }
                }
            }
        }
        CSaddr += CS_len;   // Increment CSaddr to point to the next section
    }
    input_file.close();
    return CSaddr;
}