#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <stack>
#include <algorithm>
#include <sstream>
#include <bits/stdc++.h>
using namespace std;

vector<string> registers = {"A", "X", "L", "PC", "SW", "B", "S", "T", "F"}; // Register set for SIC/XE
vector<int> reg_predef_vals = {0, 1, 2, 8, 9, 3, 4, 5, 6};                  // Their default values
map<pair<string, string>, vector<int>> SYMTAB;                              // Symbol Table, LITTAB created using unordered map
unordered_map<string, pair<string, int>> OPTAB;                             // Opcodes Table created using unordered map
                                                                            // Used unordered map as it will fetch us values in O(1) time and internally it implements hashing
map<string, vector<int>> LITTAB;
vector<string> Modify_records, Curr_EXTREFTAB;
unordered_map<string, int> ESTAB;
unordered_map<char, int> operators_order;
vector<string> memory(1 << 18, "XX");
map<int, int> memory_mapper;
string captialize_string(string s) // This function will be used to convert lowercase characters to uppercase.
{
    for (int a = 0; a < s.size(); a++)
    {
        if (s[a] - 'a' >= 0 && s[a] - 'z' <= 25)
        {
            s[a] = (s[a] - 'a') + 'A';
        }
    }
    return s;
}

string transform_int_to_hex_string(int val, int length) // Will first convert val into hexadecimal format
{                                                       // and then into a string of length equal to length parameter
    stringstream sstream;
    sstream << hex << val;
    string hex_str = sstream.str();
    int size = hex_str.size();
    reverse(hex_str.begin(), hex_str.end());
    for (int a = 0; a < length - size; a++)
    {
        hex_str += '0';
    }
    reverse(hex_str.begin(), hex_str.end());
    return captialize_string(hex_str); // The final hex string may contain lowercase characters so therefore it will captialize them
}
string transform_string(string str, int length) // Will transform the string into a string of length equal to length parameter.
{                                               // Will pad spaces in front of the string
    int size = str.size();
    for (int a = 0; a < length - size; a++)
        str += ' ';
    return str;
}
string transform_int_to_string(string str, int length) // Will treat the string as an integer val and will append 0 in it to
{                                                      // transform it into a string of length equal to length parameter
    int size = str.size();
    reverse(str.begin(), str.end());
    for (int a = 0; a < length - size; a++)
    {
        str += '0'; // Appending 0 in front of this string
    }
    reverse(str.begin(), str.end());
    return captialize_string(str);
}
string trim(string str)
{
    int end_index = str.size() - 1;
    while (end_index >= 0 && str[end_index] == ' ')
    {
        end_index--;
    }
    return str.substr(0, end_index + 1);
}
vector<string> parse_line(string line, int type) // This function will be used to parse the input line which be fetch from a file
{
    vector<string> words; // Will contain all the words present in this line
    string curr_word;
    if (type != 3)
    {
        for (int i = 0; i < line.size(); i++)
        {
            if (line[i] == ' ' || line[i] == '\t' || line[i] == '\r' || line[i] == '\n') // A word will not contain spaces, tabs, linefeeds and new lines symbols
            {
                if (curr_word.size())
                {
                    words.push_back(curr_word);
                    curr_word = "";
                }
            }
            else
                curr_word += line[i];
        }
        if (curr_word.size())
        {
            words.push_back(curr_word);
            curr_word = "";
        }              // Type 0 will be used when we will parse the opcodes file
        if (type == 1) // Type 1 will be used when we will parse the input file
        {
            if (words.size() != 3 && words[0] != ".") // If the line is not a comment, then to keep the input line format consistent
            {
                // this will set the label as an empty string
                if (!(words.size() == 2 && words[1] == "CSECT"))
                {
                    reverse(words.begin(), words.end());
                    words.push_back("");
                    reverse(words.begin(), words.end());
                }
            }
        }
        else if (type == 2) // Type 2 will be used when we will parse the intermediate file created after pass 1
        {
            if (words.size() == 3 && words[0] != ".") // Will set the label as an empty string if the line is not a comment
            {
                words.push_back(words[2]);
                words[2] = words[1];
                words[1] = "";
            }
            else if (words.size() == 2 && words[0] != ".") // These ifs are used to keep the input format consistent
            {
                if (words[0] == "END")
                {
                    reverse(words.begin(), words.end());
                    words.push_back("");
                    words.push_back("");
                    reverse(words.begin(), words.end());
                }
                else
                {
                    words.push_back("");
                    words.push_back("");
                    words[2] = words[1];
                    words[1] = "";
                }
            }
        }
    }
    else
    {
        for (auto it : line) // Used for parsing the output file
        {
            if (it == '^')
            {
                words.push_back(curr_word);
                curr_word = "";
            }
            else
            {
                curr_word += it;
            }
        }
        if (curr_word.size())
        {
            words.push_back(curr_word);
        }
    }
    return words;
}

void OPTAB_filler() // Will create the Opcodes table
{
    ifstream opcodes("opcodes_file.txt");
    if (opcodes.is_open())
    {
        string fetched_line;
        while (opcodes.peek() != EOF) // If there still exists a line inside opcodes file which can be parsed
        {
            getline(opcodes, fetched_line);
            vector<string> parsed_opcode = parse_line(fetched_line, 0);
            OPTAB[parsed_opcode[0]] = {parsed_opcode[1], stoi(parsed_opcode[2])}; // Will set the values in OPTAB map
        }
        opcodes.close(); // After setting all the values close the opened opcodes_file.txt
        return;
    }
    cout << "Error opening opcodes_file.txt" << endl; // This means that an error occured while opening the opcodes_file.txt
    exit(1);
}

void set_variables(string &intermediate_filename, string &output_filename, string &listing_filename, string &loader_output_filename, string &loader_output_visualizer_filename, int argc, char *argv[])
{
    intermediate_filename = "intermediate.txt";
    output_filename = "output.txt"; // Default names of intermediate, output and listing files
    listing_filename = "listing.txt";
    loader_output_filename = "memory_visualizer.txt";
    loader_output_visualizer_filename = "memory.txt";
    if (argc >= 3) // If the user provides output file name
    {
        output_filename = argv[2];
    }
    if (argc >= 4) // If along with the output file name, the user also provides the listing file name
    {
        listing_filename = argv[3];
    }
}
void add_register_values(string CSECT) // Setting default register values in Symbol table for the current Csection
{
    for (int i = 0; i < registers.size(); i++)
    {
        SYMTAB[{CSECT, registers[i]}] = {reg_predef_vals[i], 3};
    }
}

bool check_int(string str) // Checks whether the given string is integer or not
{
    if (str.size() == 0)
    {
        return false;
    }
    bool result = true;
    for (int i = 0; i < str.length(); i++)
    {
        if (!(str[i] - '0' >= 0 && str[i] - '0' <= 9))
        {
            result = false;
            break;
        }
    }
    return result;
}

pair<int, vector<string>> evaluate_cond(string CSECT, string v1, string v2, char cond, vector<string> Curr_EXTREFTAB, string LOCCTR, string len)
{ //  Evaluates the condition between the two variables given
    int x = 0, y = 0;
    vector<string> result;
    if (check_int(v2))
    {
        y = stoi(v2);
    }
    else
    {
        if (SYMTAB.find({CSECT, v2}) != SYMTAB.end()) // If it is a symbol
        {
            y = SYMTAB[{CSECT, v2}][0];
        }
        else
        {
            if (LITTAB.find(v2) != LITTAB.end()) // If it is a literal
            {
                y = LITTAB[v2][0];
            }
            else
            {
                if (find(Curr_EXTREFTAB.begin(), Curr_EXTREFTAB.end(), v2) != Curr_EXTREFTAB.end()) // This means we need to update the value at the current LOCCTR with address referenced from another section
                {
                    string temp = "M^";
                    temp += transform_int_to_string(LOCCTR, 6) + "^" + len + "^+";
                    temp += v2;
                    result.push_back(temp);
                }
                else
                {
                    cout << "ERROR: INVALID EXPRESSION" << endl;
                    exit(1);
                }
            }
        }
    }
    if (check_int(v1)) // Same as above
    {
        x = stoi(v1);
    }
    else
    {
        if (SYMTAB.find({CSECT, v1}) != SYMTAB.end())
        {
            x = SYMTAB[{CSECT, v1}][0];
        }
        else
        {
            if (LITTAB.find(v1) != LITTAB.end())
            {
                x = LITTAB[v1][0];
            }
            else
            {
                if (find(Curr_EXTREFTAB.begin(), Curr_EXTREFTAB.end(), v1) != Curr_EXTREFTAB.end())
                {
                    string temp = "M^";
                    temp += transform_int_to_string(LOCCTR, 6) + "^" + len + "^";
                    if (cond == '+' || cond == '-')
                        temp += cond;

                    temp += v1;
                    result.push_back(temp);
                }
                else
                {
                    cout << "ERROR: INVALID EXPRESSION" << endl;
                    exit(1);
                }
            }
        }
    }
    if (cond == '+')
    {
        return {x + y, result};
    }
    if (cond == '-')
    {
        return {y - x, result};
    }
    if (cond == '*')
        return {x * y, result};
    if (x)
        return {y / x, result};
    else
    {
        cout << "ERROR: DIVISION BY ZERO" << endl;
        exit(1);
    }
}
pair<int, vector<string>> simplify_expr(string CSECT, string expr, string LOCCTR, vector<string> Curr_EXTREFTAB, string len = "06")
{
    operators_order['+'] = 1; // Setting the precedance order
    operators_order['-'] = 1;
    operators_order['*'] = 2;
    operators_order['/'] = 2;
    expr = "(0+(" + expr + "))";
    int type = 0, value = 0, size = expr.size(), currval = 0, currsize = 0;
    stack<string> st;      // Stack containing the variables
    stack<char> ops;       // Stack containing the operators
    vector<string> result; // Contains the modification records
    for (int a = 0; a < size; a++)
    {
        if (expr[a] - '0' >= 0 && expr[a] - '0' <= 9) // If it is an integer
        {
            currval *= 10;
            currsize++;
            currval += expr[a] - '0';
        }
        else
        {
            if (expr[a] == '+' || expr[a] == '-' || expr[a] == '*' || expr[a] == '/') // If it is an operator
            {
                if (currsize != 0)
                {
                    st.push(to_string(currval));
                    currsize = 0;
                    currval = 0;
                }
                if (ops.size() && operators_order[ops.top()] > operators_order[expr[a]]) // If precedance of previous operator is greater than current operator
                {
                    string v1 = st.top();
                    st.pop();
                    string v2 = st.top();
                    st.pop();
                    char op = ops.top();
                    ops.pop();
                    st.push(to_string(evaluate_cond(CSECT, v1, v2, op, Curr_EXTREFTAB, LOCCTR, len).first));
                    vector<string> temp = evaluate_cond(CSECT, v1, v2, op, Curr_EXTREFTAB, LOCCTR, len).second;
                    for (auto it : temp)
                    {
                        result.push_back(it);
                    }
                }
                else
                {
                    ops.push(expr[a]);
                }
            }
            else
            {
                if (expr[a] == '(') // If encountered opening bracket
                    ops.push(expr[a]);
                else
                {
                    if (expr[a] == ')') // If encountered closing bracket then perform operations until opening bracket encountered
                    {
                        while (st.size() >= 2 && ops.top() != '(')
                        {
                            string v1 = st.top();
                            st.pop();
                            string v2 = st.top();
                            st.pop();
                            char op = ops.top();
                            ops.pop();
                            st.push(to_string(evaluate_cond(CSECT, v1, v2, op, Curr_EXTREFTAB, LOCCTR, len).first));
                            vector<string> temp = evaluate_cond(CSECT, v1, v2, op, Curr_EXTREFTAB, LOCCTR, len).second;
                            for (auto it : temp)
                            {
                                result.push_back(it);
                            }
                        }
                        if (ops.size())
                            ops.pop();
                    }
                    else
                    {
                        string symbol;
                        while (a <= size && !(expr[a] == '+' || expr[a] == '-' || expr[a] == '*' || expr[a] == '/' || expr[a] == '(' || expr[a] == ')')) // If it is a variable
                        {
                            symbol += expr[a];
                            a++;
                        }
                        a--;
                        st.push(symbol);
                    }
                }
            }
        }
    }

    return {stoi(st.top()), result};
}

void mask_val(int &value, int bits) // Used for masking values
{
    if (value < 0)
    {
        int masked_val = 0;
        for (int i = 0; i < bits; i++)
        {
            masked_val |= 1;
            masked_val = masked_val << 1;
        }
        masked_val = masked_val >> 1;
        value = value & masked_val;
    }
}
vector<string> split_str(string op_str) // Will split the string based on ,
{
    vector<string> result;
    string temp = "";
    int n = op_str.size();
    for (int a = 0; a < n; a++)
    {
        if (op_str[a] == ',')
        {
            result.push_back(temp);
            temp = "";
        }
        else
        {
            temp += op_str[a];
        }
    }
    if (temp.size())
        result.push_back(temp);
    return result;
}

int hex_to_int(string hex_str) // Converts hexadecimal string to integer
{
    int dec = 0;
    for (int i = 0; i < hex_str.size(); i++)
    {
        if (hex_str[i] >= 'A' && hex_str[i] <= 'F')
            dec = dec * 16 + hex_str[i] - 'A' + 10;
        else
            dec = dec * 16 + hex_str[i] - '0';
    }
    return dec;
}
string int_to_hex(int val) // Converts integer into hexadecimal string
{
    string result;
    if (val == 0)
        return "0";
    while (val)
    {
        int dig = val % 16;
        if (dig < 10)
        {
            result += '0' + dig;
        }
        else
        {
            dig -= 10;
            result += 'A' + dig;
        }
        val /= 16;
    }
    reverse(result.begin(), result.end());
    return result;
}
