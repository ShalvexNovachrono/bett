#ifndef LoggerHeader
#define LoggerHeader

#include <cstdlib>
#include <iostream>
#include <string>

// Color codes for terminals that support ANSI escapes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"

#define MAKE_STRING_RED(_string)    (std::string(COLOR_RED) + (_string) + COLOR_RESET)
#define MAKE_STRING_YELLOW(_string) (std::string(COLOR_YELLOW) + (_string) + COLOR_RESET)
#define MAKE_STRING_GREEN(_string)  (std::string(COLOR_GREEN) + (_string) + COLOR_RESET)

// Toggle logging at runtime by changing this flag. Default: enabled.
inline bool CGUI_LOG_ENABLED = true;

// Basic (B-prefixed) logging - uses std::cout for non-errors
#define BLogDebug(message)    { if (CGUI_LOG_ENABLED) { std::cout << COLOR_GREEN << "[LOGGING] " << message << COLOR_RESET << "\n"; } }
#define BLogDebugR(message)   { if (CGUI_LOG_ENABLED) { std::cout << COLOR_GREEN << "\r[LOGGING] " << message << COLOR_RESET << std::flush; } }
#define BLogWarning(message)  { if (CGUI_LOG_ENABLED) { std::cout << COLOR_YELLOW << "[WARNING] " << message << COLOR_RESET << "\n"; } }
#define BLogError(message)    { if (CGUI_LOG_ENABLED) { std::cout << COLOR_RED << "[ERROR] " << message << COLOR_RESET << "\n"; } }
#define BLogFatal(message)    { if (CGUI_LOG_ENABLED) { std::cout << COLOR_RED << "[FATAL] " << message << COLOR_RESET << "\n"; } exit(EXIT_FAILURE); }

// Stream-style logging (uses std::cerr for warnings/errors)
#define LogDebug(message)     { if (CGUI_LOG_ENABLED) { std::cout << MAKE_STRING_GREEN("[LOGGING] " + std::string(message)) << "\n"; } }
#define LogDebugR(message)    { if (CGUI_LOG_ENABLED) { std::cout << "\r" << MAKE_STRING_GREEN("[LOGGING] " + std::string(message)) << std::flush; } }
#define LogWarning(message)   { if (CGUI_LOG_ENABLED) { std::cerr << MAKE_STRING_YELLOW("[WARNING] " + std::string(message)) << "\n"; } }
#define LogError(message)     { if (CGUI_LOG_ENABLED) { std::cerr << MAKE_STRING_RED("[ERROR] " + std::string(message)) << "\n"; } }
#define LogFatal(message)     { if (CGUI_LOG_ENABLED) { std::cerr << MAKE_STRING_RED("[ERROR] " + std::string(message)) << "\n"; } exit(EXIT_FAILURE); }

#endif
