#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <stack>

// Tokenize input string by a delimiter
std::vector<std::string> tokenize(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    for (char ch : input) {
        if (ch == delimiter) {
            if (!token.empty()) tokens.push_back(token);
            token.clear();
        } else {
            token += ch;
        }
    }
    if (!token.empty()) tokens.push_back(token);
    return tokens;
}

// Interpreter class for handling commands and variables
class Interpreter {
    std::map<std::string, std::string> data;
    std::map<std::string, std::function<std::string(const std::vector<std::string>&, const std::vector<std::string>&)>> commands;
    std::map<std::string, std::string> commandDescriptions;

public:
    Interpreter() {
        // Define command descriptions
        commandDescriptions["help"] = "Displays a list of available commands or detailed information about a specific command if provided.";
        commandDescriptions["exit"] = "Exits the interpreter.";
        commandDescriptions["set"] = "Usage: set [varName] [value] - Sets a variable with the given name and value.";
        commandDescriptions["get"] = "Usage: get [varName] - Retrieves the value of the specified variable.";
        commandDescriptions["add"] = "Usage: add [var1] [var2] &... - Adds two or more numeric values.";
        commandDescriptions["list"] = "Lists all stored variables and their values.";
        commandDescriptions["delete"] = "Usage: delete [varName] - Deletes the specified variable.";
        commandDescriptions["multiply"] = "Usage: multiply [var1] [var2] - Multiplies two numeric variables or values.";
        commandDescriptions["clear"] = "Clears all stored variables.";

        commands["help"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            if (!optionalArgs.empty() && optionalArgs[0][0] == '%') {
                std::string command = optionalArgs[0].substr(1);
                if (commandDescriptions.find(command) != commandDescriptions.end()) {
                    return commandDescriptions[command];
                } else {
                    return "Error: Unknown command '" + command + "'";
                }
            }

            std::string result = "Available commands:\n";
            for (const auto& cmd : commands) {
                result += " - " + cmd.first + "\n";
            }
            return result;
        };

        commands["exit"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            data["running"] = "false";
            return "Exiting program...";
        };

        commands["set"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            if (args.size() != 2) throw std::runtime_error("Usage: set [varName] [value]");
            data[args[0]] = args[1];
            return "Set " + args[0] + " to " + args[1];
        };

        commands["get"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) -> std::string {
            if (args.size() != 1) throw std::runtime_error("Usage: get [varName]");
            if (data.find(args[0]) == data.end()) return std::string("Error: variable not found");
            return data[args[0]];
        };

        commands["add"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            if (args.size() < 2) throw std::runtime_error("Usage: add [var1] [var2] &...");
            int total = 0;
            for (const std::string& arg : args) {
                total += std::stoi(getValue(arg));
            }
            for (const std::string& optArg : optionalArgs) {
                total += std::stoi(getValue(optArg));
            }
            return std::to_string(total);
        };

        commands["list"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            std::string result;
            for (const auto& pair : data) {
                result += pair.first + " -> " + pair.second + "\n";
            }
            return result.empty() ? "No variables stored." : result;
        };

        commands["delete"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            if (args.size() != 1) throw std::runtime_error("Usage: delete [varName]");
            if (data.erase(args[0])) {
                return "Deleted variable: " + args[0];
            }
            return std::string("Error: variable not found");
        };

        commands["multiply"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            if (args.size() != 2) throw std::runtime_error("Usage: multiply [var1] [var2]");
            int val1 = std::stoi(getValue(args[0]));
            int val2 = std::stoi(getValue(args[1]));
            return std::to_string(val1 * val2);
        };

        commands["clear"] = [this](const std::vector<std::string>& args, const std::vector<std::string>& optionalArgs) {
            data.clear();
            return "Cleared all variables.";
        };
    }

    // Parse a line of input and execute commands using a stack-based approach
    std::string runLine(const std::string& input) {
        auto tokens = tokenize(input, ' ');
        if (tokens.empty()) return "";

        std::stack<std::pair<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>>> commandStack;

        // Push initial command
        commandStack.push({tokens[0], {{}, {}}});

        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i][0] == '@') {
                commandStack.push({tokens[i].substr(1), {{}, {}}});
            } else if (tokens[i][0] == '&') {
                commandStack.top().second.second.push_back(tokens[i].substr(1));
            } else {
                commandStack.top().second.first.push_back(tokens[i]);
            }

            while (!commandStack.empty() && commandStack.top().second.first.size() == getCommandArgsCount(commandStack.top().first)) {
                auto [command, args] = commandStack.top();
                commandStack.pop();
                std::string result = commands[command](args.first, args.second);
                if (!commandStack.empty()) {
                    commandStack.top().second.first.push_back(result);
                } else {
                    return result;
                }
            }
        }

        // Check if the command requires 0 inputs and execute it immediately
        while (!commandStack.empty() && getCommandArgsCount(commandStack.top().first) == 0) {
            auto [command, args] = commandStack.top();
            commandStack.pop();
            std::string result = commands[command](args.first, args.second);
            if (commandStack.empty()) {
                return result;
            } else {
                commandStack.top().second.first.push_back(result);
            }
        }

        return "Error: Incomplete command";
    }

    // Helper function to get the number of required arguments for a command
    size_t getCommandArgsCount(const std::string& command) {
        static const std::map<std::string, size_t> commandArgsCount = {
            {"help", 0},
            {"exit", 0},
            {"set", 2},
            {"get", 1},
            {"add", 2},
            {"list", 0},
            {"delete", 1},
            {"multiply", 2},
            {"clear", 0}
        };
        auto it = commandArgsCount.find(command);
        if (it != commandArgsCount.end()) {
            return it->second;
        }
        throw std::runtime_error("Unknown command: " + command);
    }

    // Get variable value or evaluate the token as an expression
    std::string getValue(const std::string& token) {
        if (data.find(token) != data.end()) {
            return data[token];
        }
        return token;
    }

    void run() {
        data["running"] = "true";
        std::cout << "Welcome to the Interpreter! Type 'help' for a list of commands.\n";

        while (data["running"] == "true") {
            std::string input;
            std::cout << "> ";
            std::getline(std::cin, input);
            try {
                std::cout << runLine(input) << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }
};

int main() {
    Interpreter interpreter;
    interpreter.run();
    return 0;
}

