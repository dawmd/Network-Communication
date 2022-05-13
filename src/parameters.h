#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include <cstdlib> // std::size_t
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace Network {

class ArgumentParser {
private:
    struct Arguments {
        bool present = false;
        std::size_t expected_val_count;
        std::vector<std::string> values;

        Arguments(std::size_t arguments_count)
        : expected_val_count{arguments_count}
        , values{} {}
    };

    std::unordered_map<std::string, std::size_t> aliases;
    std::vector<Arguments> arguments;

public:
    ArgumentParser() = default;
    ~ArgumentParser() = default;

    ArgumentParser(const ArgumentParser&) = default;
    ArgumentParser &operator=(const ArgumentParser&) = default;

    ArgumentParser(ArgumentParser&&) = default;
    ArgumentParser &operator=(ArgumentParser&&) = default;

    ArgumentParser &add_prefix(std::initializer_list<std::string> prefixes, std::size_t arg_count) {
        const std::size_t index = arguments.size();
        for (auto &prefix : prefixes) {
            if (aliases.contains(prefix))
                throw std::invalid_argument{"Prefix already exists."};
            aliases.emplace(prefix, index);
        }
        arguments.push_back(Arguments{arg_count});
        return *this;
    }

    // Passing the argument by value is intended here.
    // If we got a temporary value, we'd move it.
    // If we got a const reference, we'd have to copy it anyway.
    void add_arguments(std::vector<std::string> args) {
        auto it = args.begin();
        while (it != args.end()) {
            const std::size_t index = aliases.at(*it++); // can throw std::out_of_range
            for (std::size_t i = 0; i < arguments[index].expected_val_count; ++i) {
                if (it == args.end())
                    throw std::out_of_range{"Too few arguments."};
                arguments[index].values.push_back(std::move(*it++));
            }
            arguments[index].present = true;
        }
    }

    bool is_present(const std::string &prefix) const noexcept {
        if (!aliases.contains(prefix))
            return false;
        return arguments[aliases.at(prefix)].present;
    }

    const std::vector<std::string> &get_arguments(const std::string &prefix) const {
        if (!is_present(prefix))
            throw std::out_of_range{"Argument is not present."};
        return arguments[aliases.at(prefix)].values;
    }
};

} // namespace Network

#endif // __PARAMETERS_H__
