#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <stdexcept>

std::vector<std::string> keywords = {"print", "set", "get", "exit", "quit", "help", "list", "add"};

std::vector<std::string> tokenize(const std::string& input, char breaker)  {
    std::vector<std::string> tokens = {};
    std::string token = "";
    for (int i = 0; i < input.length(); i++) {
        if (input[i] == breaker) {
            tokens.push_back(token);
            token = "";
        } else {
            token += input[i];
        }
    }
    tokens.push_back(token);
    return tokens;
}

bool isKeyword(const std::string& token) {
    for (int i = 0; i < keywords.size(); i++) {
        if (token == keywords[i]) {
            return true;
        }
    }
    return false;
}

// returns of list of updats for the data
std::vector<std::vector<std::string>> runLine(std::vector<std::string> tokens, std::map<std::string, std::string> data, int tokenOn = 0) {
    std::vector<std::vector<std::string>> updates = {};
    std::vector<std::string> t = {};

    // the first element is always the output, updates[0][1] is the output var
    t.push_back("output");
    t.push_back("null");
    updates.push_back(t); 

    if(tokens[tokenOn] == "help") {
        // prints a list of commands; help
        std::string text = "\n";
        for(int i = 0; i < keywords.size(); i++) {
            text += keywords[i] + "\n";
        }
        std::vector<std::string> update = {};
        update.push_back("outputText");
        update.push_back(text);
        updates.push_back(update);
    } else if(tokens[tokenOn] == "list") {
        // returns a list of all items in memory; list
        std::string text = "";
        for (const auto& pair : data) {
            text += pair.first + " -> " + pair.second + " | ";
        }
        updates[0][1] = text;
    } else if (tokens[tokenOn] == "print") {
        // prints a value or the value returned from a function; print [value]
        std::vector<std::string> update = {};
        update.push_back("outputText");
        if(isKeyword(tokens[tokenOn+1])) {
            update.push_back(runLine(tokens, data, tokenOn+1)[0][1]);
        } else {
            update.push_back(tokens[tokenOn+1]); 
        }
        updates.push_back(update);
    } else if(tokens[tokenOn] == "exit" || tokens[tokenOn] == "quit") {
        // stops the program; exit; quit;
        std::vector<std::string> update = {};
        update.push_back("running");
        update.push_back("false");
        updates.push_back(update);
    } else if(tokens[tokenOn] == "set") {
        // sets the value of a variable; set [varName] [value]
        std::vector<std::string> update1 = {};
        update1.push_back(tokens[tokenOn+1]);
        update1.push_back(tokens[tokenOn+2]);
        updates.push_back(update1);
    } else if(tokens[tokenOn] == "get") {
        // gets the value of a variable; get [varName]
        updates[0][1] = data[tokens[tokenOn+1]];
    } else if (tokens[tokenOn] == "add") {
        // adds two variables together; add [var1] [var2]
        int num1 = std::stoi(data[tokens[tokenOn+1]]);
        int num2 = std::stoi(data[tokens[tokenOn+2]]);
        updates[0][1] = std::to_string(num1 + num2);
    } else {
        // if no commands are found, it treats it as a variable; [varName] [value]
        
        std::vector<std::string> update1 = {};
        update1.push_back(tokens[tokenOn]);
        update1.push_back(tokens[tokenOn+1]);
        updates.push_back(update1);
    }
    return updates;
}

int main() {
    std::string userInput;
    std::map<std::string, std::string> data;
    data["running"] = "true";
    data["outputText"] = "null";
    
    std::cout << "Welcome to the Simple Interpreter!" << std::endl;
    std::cout << "Type 'help' for a list of commands." << std::endl;
    std::cout << "Type 'exit' or 'quit' to stop the program." << std::endl;
    
    while (data["running"] == "true") {

        std::cout << "Enter a command: ";
        std::getline(std::cin, userInput);

        std::vector<std::string> tokens = tokenize(userInput, ' ');
        std::vector<std::vector<std::string>> newData = runLine(tokens, data);

        for(int i = 0; i < newData.size(); i++) {
            std::string key = newData[i][0];
            std::string value = newData[i][1];
            data[key] = value;
        }

        std::cout << "output: " << data["outputText"] << std::endl;
        data["outputText"] = "null";
    }
    
    return 0;
}

