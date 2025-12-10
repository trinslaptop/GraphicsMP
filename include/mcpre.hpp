#ifndef MCPRE_HPP
#define MCPRE_HPP
#include <fstream>
#include <string>
#include <regex>
#include <stack>
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <ctime>

/// MCPre - A Mini C-Like Preprocessor
/// (c) Trin Wasinger 2025
/// 
/// MCPre supports a primitive subset of C preprocessor directives and object macros.
/// It was originally designed to supplement GLSL shaders.
///
/// Supported Directives:
///	+ `#include "..."` - Includes another file relative to the current one (NOTE: this doesn't support the `<...>` form, just `"..."`)
///	+ `#def`/`#define MACRO_NAME` - Defines an empty object macro
///	+ `#def`/`#define MACRO_NAME VALUE` - Defines a macro with a value
/// + `#undef`/`#undefine MACRO_NAME` - Undefines a macro
/// + `#ifdef MACRO_NAME`/`#ifndef MACRO_NAME`/`#else`/`#endif`
/// + `#pragma once` - Prevents future `#includes` of this file from including it again
/// + `#pragma region`/`#pragma endregion`/`#line ...` - Stripped for compatibility but have no effect
///
/// While all are enabled by default, if only a subset of the directives are needed, the `features` parameter can be used to enable only some.
///
/// Builtin Macros:
/// + `__TIME__` - The current time as returned by `time(NULL)`
/// + `__NEWLINE__` - Allows embedding newlines in other macro values
/// + `__MAIN__` - Defined iff in the root file `mcpre::preprocess()` was initially called on and not some other include. Useful for wrapping `main()`
/// 
/// User defined macros must match `[A-Z][A-Z0-9_]` while builtins and implementation defined ones may also start with `_`.
/// 
/// NOTE: MCPre has no knowledge of C, C++, or GLSL syntax. It's just a glorified string replacer.
/// NOTE: MCPre will not raise any errors in the event a file is not found or it encounters unexpected directives; empty strings will be used
/// in place of missing file contents, and anything not transformed is passed to the output. Therefore, if something is wrong, it will show
/// up in the shader logs instead.
/// NOTE: While other macros in macros are fine, don't try and make recursive macros, it won't end well (aka ever)...
namespace mcpre {
    namespace {
        // Adapted from https://stackoverflow.com/a/37516316 by John Martin (2016) and Violet Giraffe (2018) (CC BY-SA 3.0)
        typedef std::function<std::string(const std::smatch&)> MatchReplacer;
        // Replaces all matches of `regex` in `text` with the result of calling `f` on the match, returns `true` if any matches were made
        inline bool regex_freplace(std::string& text, const std::regex& regex, MatchReplacer f) {
            std::sregex_iterator begin(text.cbegin(), text.cend(), regex), end;
            std::string::const_iterator last = text.cbegin();
            std::string result;
            
            bool matched = false;

            std::for_each(begin, end, [&](const std::smatch& match) {
                result.append(last, match[0].first);
                result.append(f(match));
                last = match[0].second;

                matched = true;
            });

            result.append(last, text.cend());

            text = result;
            return matched;
        }
    }

    // Expands object-like macros in a given string (Note: function-like macros are not supported, and object-like macros must be `[A-Z_][A-Z0-9_]`)
    inline std::string expand_macros(std::string text, std::map<std::string, std::string>& macros) {
        std::string ttext;
        while(regex_freplace(ttext = text, std::regex("\\b[A-Z_][A-Z0-9_]*\\b"), [&](auto match) {
            if(macros.find(match[0]) != macros.end()) {
                return std::string(macros[match[0]]);
            } else {
                return std::string(match[0]);
            }
        }) && text != text);
        return ttext;
    }

    /// Preprocesses a string
    /// `features` should be a subset of `{"include", "define", "ifdef", "pragma", "line"}` controling what is enabled (NOTE: some directives are grouped like define and undefine)
    inline std::string preprocess(const std::string& path, std::map<std::string, std::string> macros = std::map<std::string, std::string>(), const std::set<std::string> features = {"include", "define", "ifdef", "pragma", "line"}) {
        try {
            std::vector<std::pair<std::regex, MatchReplacer>> rules;
            std::stack<bool> conditions;
            std::ostringstream out;
    
            std::stack<std::pair<std::filesystem::path, std::ifstream>> files;
            std::set<std::string> excludes;
    
            macros["__TIME__"] = std::to_string(time(NULL));
            macros["__NEWLINE__"] = "\n";
            macros["__MAIN__"] = "";
    
            if(features.find("include") != features.end()) {
                rules.push_back({std::regex("^\\s*#include\\s+\"([^\"]*)\"(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    try {
                        const std::filesystem::path path = std::filesystem::canonical(std::filesystem::path(files.top().first).parent_path() / std::string(match[1]));
                       // if(!excludes.count(path)) {
                            std::ifstream include(path);
                            if(include.is_open()) files.push({path, std::move(include)});
                        //}
                    } catch(...) {}
                    return "";
                }});
            }
    
            if(features.find("define") != features.end()) {
                rules.push_back({std::regex("^\\s*#(?:define|def)\\s+([A-Z_][A-Z0-9_]*)(?:\\s+(.*?))?(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    if(std::string(match[1])[0] != '_') macros[match[1]] = match[2];
                    return "";
                }});
        
                rules.push_back({std::regex("^\\s*#(?:undefine|undef)\\s+([A-Z_][A-Z0-9_]*)(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    if(std::string(match[1])[0] != '_' && macros.count(match[1])) macros.erase(match[1]);
                    return "";
                }});
            }
            
            if(features.find("line") != features.end()) {
                rules.push_back({std::regex("^\\s*#line\\b.*\\s*(?:\\s*?//.*?)?\\s$"), [&](auto match) {
                    return "";
                }});
            }

			if(features.find("pragma") != features.end()) {
                rules.push_back({std::regex("^\\s*#pragma\\s+once(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    //excludes.insert(files.top().first);
                    return "";
                }});
        
                rules.push_back({std::regex("^\\s*#pragma\\s+(?:end)?region(?:\\s+(.*?))?(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    return "";
                }});
            }
    
			if(features.find("ifdef") != features.end()) {
                rules.push_back({std::regex("^\\s*#(?:ifdef)\\s+([A-Z_][A-Z0-9_]*)(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    conditions.push(macros.count(match[1]));
                    return "";
                }});
        
                rules.push_back({std::regex("^\\s*#(?:ifndef)\\s+([A-Z_][A-Z0-9_]*)(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    conditions.push(!macros.count(match[1]));
                    return "";
                }});
                
                rules.push_back({std::regex("^\\s*#else(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    if(conditions.size() > 0) {
                        bool t = conditions.top();
                        conditions.pop();
                        conditions.push(!t);
                    }
                    return "";
                }});
                
                rules.push_back({std::regex("^\\s*#endif(?:\\s*?//.*?)?\\s*$"), [&](auto match) {
                    if(conditions.size() > 0) conditions.pop();
                    return "";
                }});
            }

    
            std::string line;
            
            files.push({std::filesystem::weakly_canonical(std::filesystem::path(path)), std::ifstream(path)});
            if(!files.top().second.is_open()) return "";
    
            while(!files.empty() && std::getline(files.top().second, line)) {
                if(files.size() == 1) {
                    macros["__MAIN__"] = "";
                } else {
                    macros.erase("__MAIN__");
                }
                
                bool matched = false;
                for(const auto& pair : rules) {
                    if((matched = regex_freplace(line, pair.first, pair.second))) break;
                }
    
                if((conditions.size() == 0 || conditions.top()) && (!matched || line != "")) {
                    out << expand_macros(line, macros) << "\n";
                }
    
                if(files.top().second.peek() == EOF) {
                    files.pop();
                }
            }
    
            return out.str();
        } catch(...) {
            return "";
        }
    }
}
#endif